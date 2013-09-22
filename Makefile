#######################################################################
# Práctica paralelización. AEC. FIC. 2012/2013.                       #
#   Makefile do programa.                                             #
#                                                                     #
# 	Pablo Castro Valiño (pablo.castro1@udc.es)                        #
# 	Marcos Chavarría Teijeiro (marcos.chavarria@udc.es)               #
#######################################################################

#Valor do chunk.
CHUNK = 200

#Parametros de compilación
PCC = mpicc
SCC = gcc
PROGPARAL = combsorthibrido.c
PROGSEQ = combsortseq.c
EXEPARAL = combsorthibrido
EXESEQ = combsortseq
FLAGS = -fopenmp -Wall -DCHUNK=$(CHUNK) -o $(EXEPARAL)


#Nome tar.gz
NOMETAR = practicaAEC_pablo.castro1_marcos.chavarria.tar.gz
NOMMEM = memoria_pablo.castro1_marcos.chavarria.pdf

# Parametros execución
MPIPROCESSES=8
OMPTHREADS=3


all:
	@echo "Opcións dispoñibles:"
	@echo " * compile		Compila o programa hibrido MPI+OpenMP."
	@echo " * compileseq		Compila o programa secuencial."
	@echo " * compiledebug		Compila o programa hibrido con parametros de debugging."
	@echo " * runseq		Executa o programa secuencial en local."
	@echo " * runlocal		Executa o programa paralelo en local. Ordena 10000000 elementos con 8 procesos."
	@echo " * lanzar		Lanza o proceso para a súa execución no CESGA."
	@echo " * clean		Elimina os ficheiros sobrantes do directorio."

compile:
	$(PCC) $(FLAGS) -O3 $(PROGPARAL)

compileseq:
	$(SCC) -o $(EXESEQ) -O3 $(PROGSEQ)

compiledebug:
	$(PCC) $(FLAGS)  $(PROGPARAL) -g -DCHECK -DCHECKCHILD -DCHECKOMP -DDEBUG

runseq:
	./$(EXESEQ) 10000000

runlocal:
	@echo "Number of MPI processes:   "$(MPIPROCESSES)
	mpirun -n 8 ./$(EXEPARAL) 10000000

lanzar: traballo.sh
	@echo "Number of MPI processes:   "$(MPIPROCESSES)
	@echo "Number of OpenMP threads:  "$(OMPTHREADS)
	qsub -cwd -l arch=amd,num_proc=$(OMPTHREADS),s_rt=00:30:00,s_vmem=2G,h_fsize=1G -pe mpi_1p $(MPIPROCESSES) -N job-mpi$(MPIPROCESSES)-omp$(OMPTHREADS) traballo.sh

gdb:
	mpirun -np 2 xterm -e gdb $(EXE)

clean:
	rm *~ $(EXEPARAL) $(EXESEQ) $(NOMMEM)

tar:
	cp memoria/memoria.pdf $(NOMMEM)
	tar -cvzf $(NOMETAR) $(PROGPARAL) $(PROGSEQ) traballo.sh heap.c Makefile $(NOMMEM)
	
