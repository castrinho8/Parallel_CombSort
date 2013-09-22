/*
 * Practica Final AEC. FIC. UDC. 2012/2013
 *  Pablo Castro Valiño (pablo.castro1@udc.es)
 *  Marcos Chavarría Teijeiro (marcos.chavarria@udc.es)
 */
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void
combsort(int a[], long nElements)
{
	short swapped;
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
			return 0;
	return 1;
}

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


int
main(int argc, char const *argv[])
{

	long numberofitems;
	int *data;
	struct timeval t0, t1, t;

	if(argc != 2)
	{
		printf("Usage: ./combsort <number of items>\n");
		return -1;
	}

	numberofitems = atol(argv[1]);

	if(numberofitems <= 0){
		printf("The number of items must be a positive number.\n");
		return -1;
	}

	//Generate a random array of ints.
	generate_data(&data,numberofitems);

	assert (gettimeofday (&t0, NULL) == 0);

	//Sort this array.
	combsort(data,numberofitems);

	assert (gettimeofday (&t1, NULL) == 0);
	timersub(&t1, &t0, &t);

	//Check result if the array is small.
	#ifndef CHECK
		printf("Checking results ->");
		printf("%s\n",check_results(data,numberofitems) ? "Correct" : "Incorrect");
	#endif

	//Print sorted array.
	print_results(data,numberofitems);

	printf ("numberofitems = %ld\n", numberofitems);
	printf ("Tiempo        = %ld:%ld(seg:mseg)\n", t.tv_sec, t.tv_usec/1000);

	return 0;
}
