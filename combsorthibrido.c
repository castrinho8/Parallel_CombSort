/*
 * Practica Final AEC. FIC. UDC. 2012/2013
 *  Pablo Castro Valiño (pablo.castro1@udc.es)
 *  Marcos Chavarría Teijeiro (marcos.chavarria@udc.es)
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "heap.c"

#define CALC_IT_PER_PROC(N,P,R) N/P + (R < N % P)

#ifndef CHUNK
#define CHUNK 1
#endif

void
print_results(	int * array,
				long numberofitems)
{
	long i;
	for(i = 0; i < 5; i++)
		printf(" array[%li]: %i\n", i, array[i]);
	if(numberofitems > 5)
		for(i = numberofitems - 5;i < numberofitems;i++)
			printf(" array[%li]: %i\n", i, array[i]);
}

int
check_results(	int * array,
				long numberofitems)
{
	long i;
	for(i=1;i<numberofitems;i++)
		if(array[i] < array[i-1])
		{
			printf("Elemento en pos %li (%i) e maior que elemento en pos %li (%i)\n",i-1,array[i-1],i,array[i]);
			return 0;
		}
	return 1;
}


void
generate_data(	int ** data,
				long numberofitems)
{
	long i;
	*data = malloc(sizeof(int) * numberofitems);
	assert(*data);

	srand(time(NULL));

	for(i=0;i<numberofitems;i++)
		(*data)[i] = rand();
}

int omp_thread_count() {
    int n = 0;
    #pragma omp parallel reduction(+:n)
    n += 1;
    return n;
}

void
combsort(int a[], long nElements)
{
	short swapped = 0;
	long i, j, gap;
	int temp;

	gap = nElements;
	while (gap > 1 || swapped)
	{
		gap = gap * 10 / 13;
		if (gap == 9 || gap == 10) gap = 11;
		if (gap < 1) gap = 1;

		swapped = 0;
		for (i = 0, j = gap; j < nElements; i++, j++)
		{
			if (a[i] > a[j])
			{
				temp = a[i];
				a[i] = a[j];
				a[j] = temp;
				swapped = 1;
			}
		}
	}
}


void
omp_sort (int a[], long nElements)
{
	long  it_p_th, i, *mx_sta, *mx_end;
	int num_th, thread, * aux, proc, value;
	heap_t h;

	num_th = omp_thread_count();

	if(num_th > nElements)
	{
		combsort(a,nElements);
		return;
	}

	mx_sta = malloc(sizeof(long) * num_th);
	mx_end = malloc(sizeof(long) * num_th);

	it_p_th = nElements / num_th + ((nElements % num_th) > 0);

	#pragma omp parallel for private(thread,i)
	for(i=0; i < nElements; i += it_p_th)
	{
		thread = omp_get_thread_num();

		mx_sta[thread] = i;
		mx_end[thread] = i + it_p_th;
		if(mx_end[thread] >= nElements)
			mx_end[thread] = nElements;

		combsort(a + mx_sta[thread], mx_end[thread] - mx_sta[thread]);

		#ifdef DEBUG
			printf("This is thread %i sorting (%li,%li) -> %li\n",
					thread, mx_sta[thread],mx_end[thread], mx_end[thread] - mx_sta[thread]);
		#endif

		#ifdef CHECKOMP
			int myrank;
			MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
			printf("OMP check. Proc. %2i - Thread %2i (%li-%li): %s\n",myrank, thread, mx_sta[thread], mx_end[thread],
					check_results(a + mx_sta[thread], mx_end[thread] - mx_sta[thread]) ? "Correct" : "Error");
		#endif
	}


	h = new_heap(num_th);
	i = 0;
	aux = malloc(sizeof(int) * nElements);

	for(proc=0; proc < num_th; proc++)
	{
		if(mx_sta[proc] < mx_end[proc])
		{
			push_heap	(h,
						a[mx_sta[proc]],
						proc);
			(mx_sta[proc])++;
		}
	}

	do
	{
		pop_heap(h, &value, &proc);
		aux[i++] = value;

		if(mx_sta[proc] < mx_end[proc])
		{
			push_heap	(h,
						a[mx_sta[proc]],
						proc);
			(mx_sta[proc])++;
		}
	}while(!empty_heap(h));

	#pragma omp parallel for
	for(i=0; i< nElements; i++)
		a[i] = aux[i];

	free(aux);
	dispose_heap(h);
}


void
send_data (int data[], long nDatos)
{
	int tam;
	long i;

	for(i=0; i<nDatos; i += CHUNK)
	{
		tam = i + CHUNK < nDatos ? CHUNK : nDatos - i;

		#ifdef DEBUG
			int myrank;
			MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
			printf("Proc. %i envia datos [%li, %li) dun total de %li\n",myrank,i,i+tam, nDatos);
		#endif

		MPI_Ssend(	data + i,
					tam,
					MPI_INT,
					0,
					tam, // O flag indica o numero de datos enviados.
					MPI_COMM_WORLD);
	}

	#ifdef DEBUG
		int myrank;
		MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
		printf("Proc. %i envia señal de fin.\n",myrank);
	#endif

	MPI_Ssend(NULL,
			 0,
			 MPI_INT,
			 0,
			 0,
			 MPI_COMM_WORLD);
}

int
insert_root (int datos[], long nDatos, long *indiceroot, heap_t h)
{
		long ini_ind,tam;;

		if (*indiceroot >= nDatos)
			return 0;

		ini_ind = *indiceroot;

		tam = (*indiceroot) + CHUNK;
		if (tam > nDatos)
			tam = nDatos;

		#ifdef DEBUG
			printf("Proc. 0 inserta datos [%li, %li) dun total de %li\n",*indiceroot, tam, nDatos);
		#endif

		for(; *indiceroot < tam; (*indiceroot)++)
			push_heap(h, datos[*indiceroot], 0);

		return (*indiceroot) - ini_ind;
}

int
receive_and_insert (int proc, heap_t h)
{
	int i;
	int value[CHUNK];
	MPI_Status status;

	MPI_Recv(value,
			CHUNK,
		 	MPI_INT,
		 	proc,
		 	MPI_ANY_TAG,
		 	MPI_COMM_WORLD,
		 	&status);

	#ifdef DEBUG
		printf("Se recogen e insertan %i datos de %i\n",status.MPI_TAG, proc);
	#endif

	for (i = 0; i < status.MPI_TAG; i++)
		push_heap(h, value[i], proc);

	return status.MPI_TAG;
}

int
main(int argc, char **argv)
{

	int numprocs, myrank, i, proc, *sendcounts, *displs, recvcount;
	int *subarray, *data, *num_el_heap;
	heap_t h;
	double mytime;
	long numberofitems = -1, indiceroot;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	if(argc != 2 && !myrank)
	{
		printf("Usage: ./combsort <number of items>\n");
		return -1;
	}

	numberofitems = atol(argv[1]);

	if(!myrank){

		if(numberofitems <= 0){
			printf("The number of items must be a positive number.\n");
			return -1;
		}

		//Generate a random array of ints.
		generate_data(&data,numberofitems);

		mytime = MPI_Wtime();
	}


   // Fase 1: Comunicación
   // Scatter del array que se quiere ordenar
   // Def. procedimiento DistribucionArray()
    //CALCULAR PARAMETROS SCATTER
    if( !myrank ){

      sendcounts = malloc(sizeof(int) * numprocs);
      assert(sendcounts);

      displs = malloc(sizeof(int) * numprocs);
      assert(displs);

    	for(i=0; i < numprocs; i++)
    	{
    		sendcounts[i] = CALC_IT_PER_PROC(numberofitems, numprocs, i);
    		displs[i] = i ? displs[i-1] + sendcounts[i-1] : 0;
    	}
    }

    recvcount = CALC_IT_PER_PROC(numberofitems, numprocs, myrank);
    subarray = malloc(sizeof(int) * recvcount);
    assert(subarray);

    MPI_Scatterv( 	data,
        			sendcounts,
                    displs,
                    MPI_INT,
                    subarray,
                    recvcount,
                    MPI_INT,
                    0,
                    MPI_COMM_WORLD);

   // Fase 2: Computo
   // Ordenación secuencial local en cada proceso MPI
	omp_sort(subarray,(long) recvcount);

	#ifdef CHECKCHILD
	printf("MPI Check. Proceso %i : %s\n",myrank, check_results(subarray,
		(long) recvcount) ? "Correct" : "Incorrect");
	#endif

   // Fase 3: Comunicación
   // Ordenación global a partir de los arrays ordenados localmente
   // Def. procedimiento: OrdenamientoGlobal()
	//ORDENAR ENTRE PROC.
	if(myrank != 0) //PROCESOS FILLOS
	{
		send_data(subarray,recvcount);
	}
	else //PROCESO ROOT
	{
		h = new_heap(numprocs * CHUNK);
		num_el_heap = malloc(sizeof(long) * numprocs);
		indiceroot = 0;

		//METEMOS NA ARBORE OS DATOS DO PROC ROOT
		num_el_heap[0] = insert_root(subarray,recvcount,&indiceroot, h);

		//METEMOS NA ARBORE OS DATOS DO RESTO DE PROCs.
		for(proc=1; proc<numprocs; proc++)
			num_el_heap[proc] = receive_and_insert(proc, h);

		// EN CADA ITERACIÓN SACASE O VALOR MAIS ALTO E PIDESELLE
		// 	O PROCESADOR QUE TIÑA ESE VALOR OUTRO DATO.
		for(i = 0; i < numberofitems; i++)
		{
			int v;
			pop_heap(h,&v,&proc); // sacamos el elemento mas grande del arbol.
			data[i] = v;

			if ( ! (--num_el_heap[proc]))
			{
				num_el_heap[proc] = proc ? receive_and_insert(proc,h) : insert_root(subarray,recvcount,&indiceroot,h);
			}
		}

		dispose_heap(h);
	}

	if(!myrank){
	    mytime = MPI_Wtime() - mytime;  /*get time just after work section*/
	    mytime = mytime * MPI_Wtick();

		#ifdef CHECK
			printf("FINAL Check: %s\n", check_results(data,numberofitems) ? "Correct" : "Incorrect");
		#endif

		//Print sorted array.
		print_results(data,numberofitems);
		printf("numchunk = %i\n", CHUNK);
		printf("numberofthreads = %i\n",omp_thread_count());
		printf("numberofprocs = %i\n",numprocs );
		printf ("numberofitems = %ld\n", numberofitems);
		printf ("Tiempo        = %f (segs) \n", mytime);
	}

	MPI_Finalize();
   return 0;
}
