/**
 * Operating Systems Assignment Semester 1 2016
 * Part A. pmms using multiple processes.
 * 
 * Author: Luke Healy
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include "matrix.h"

/**
 * The 2 dimensional arrays are initialised as one dimensional
 * and indexed with pointer arithmatic to acheive the 2 dimensions.
 * This is because it is far easier to handle in shared memory.
 * I do not have an array C because as I calculate an element of a row of C,
 * I add it to the sum of the previous elements of that row. When I have
 * a subtotal available, it gets added to total as soon as it can be.
 */
int *A;				//Matrix A
int *B;				//Matrix B
Subtotal *sub;		//Stores the subtotal of a row of C.
int ax, ay, bx, by;	//Matrix Dimensions.
int total = 0;		//Total of all subtotals of C.
Semaphores *s;

/**
 * producer() is the function that each child runs, it calculates the
 * subtotal of a row of the resultant matrix in parallel.
 * I don't store each subtotal individualy, simply sum them. This is because
 * there is no point, once the subtotal is calculated, we can send it straight
 * away to the consumer.
 */
void producer(int offset ){
	int temp_sub = 0;
	for(int i = offset; i < (offset + ax); i++ ){ //For each term in row of C.
		temp_sub += calc(A[i], ((i - offset) * bx) - 1, (i - offset) * bx, 0 );
	}
	sem_wait(&s->sem_empty); 	//Post result when able
		sem_wait(&s->sem_lock); //Mutex lock
			sub->value = temp_sub;
			sub->pid = getpid();
		sem_post(&s->sem_lock); //Mutex unlock
	sem_post(&s->sem_full );
	_exit(0);
}
/**
 * consumer() is the function that only the parent runs  in order to
 * collect the subtotal made available by producer(). It also prints
 * the requied data to the screen.
 */
void consumer(){
	for(int i = 0; i < ay; i++){
		sem_wait(&s->sem_full );
			sem_wait(&s->sem_lock);	//Mutex lock
				total += sub->value;
				printf("PID %d: Subtotal = %d\n", sub->pid, sub->value );
			sem_post(&s->sem_lock );//Mutex unlock
		sem_post(&s->sem_empty );
	}
}
/**
 * proc() forks all the required producer processes.
 * Sets the signal SIGCHLD to SIG_IGN to prevent zombie processes.
 * It tells the OS to immediatly remove the process from the 
 * system when it terminates. Returns EXIT_FAILURE on failed fork,
 * mainly to safe-gaurd against forking to many processes.
 * offset is the length of a row of A.
 */
int proc(){
	int offset = 0;
	pid_t parent = getpid();
	pid_t child = 0;

	signal(SIGCHLD, SIG_IGN); //Handle zombies.

	for(int j = 0; j < ax * ay; j+= ax ){ 	//Do this for each row of C.		
		if(getpid() == parent ){ 			//Only the parent forks.
			offset = j;
			child = fork();
		}
	}

	if(child == -1){		//Fork() failed (Parent).
		perror("Failed to fork");
		return EXIT_FAILURE;
	}
	else if(child != 0 ){	//Parent
		consumer();
	}
	else{					//Child
		producer(offset );
	}
	return EXIT_SUCCESS;
}
/**
 * Recursively calculates the sum of a row of C[].
 * If the index of b[] is between that of 
 * the inital call (start of a row) and the length
 * of a row in the matrix then call calc again, 
 * incrementing idx by 1. 
 * Once the last index is reached, unwind and sum.
 */
int calc(int a_val, int idx, int init, int total ){
	if(idx++ - init < bx - 1 ){ //-1 as 0 based.
		return calc(a_val, idx, init, total) + (a_val * B[idx] );
	}
	else{
		return 0;
	}
}
/**
 * Used to set the matrix dimensions as per the command line arguments.
 */
void get_dimensions(char const *argv[]){
	ay = atoi(argv[3] );
	ax = atoi(argv[4] );
	by = atoi(argv[4] );
	bx = atoi(argv[5] );
}

/**
 * Used to create shared memory blocks, resize them and map them to virtual
 * memory. 
 */
void create_shared_memory(File_descriptors* fds){
	/*Create shared memory blocks to be pointed to by a file desctriptor*/
	fds->fd_S = shm_open("shared_memory_S", O_RDWR|O_CREAT, 0666 );		//Subtotal (C)
	fds->fd_sem = shm_open("shared_memory_sem", O_RDWR|O_CREAT, 0666 );	//Semaphores
	fds->fd_A = shm_open("shared_memory_A", O_RDWR|O_CREAT, 0666 );		//Matrx A
	fds->fd_B = shm_open("shared_memory_B", O_RDWR|O_CREAT, 0666 );		//Matrix B

	/*Resize shared block.*/
	ftruncate(fds->fd_S, (off_t)sizeof(Subtotal) ); 
	ftruncate(fds->fd_sem, (off_t)sizeof(Semaphores) );
	ftruncate(fds->fd_A, (off_t)(sizeof(int) * ay * ax) );
	ftruncate(fds->fd_B, (off_t)(sizeof(int) * by * bx) );

	/*Map shared regions to the pointer variables.*/
	sub = (Subtotal*)mmap(NULL, sizeof(Subtotal), 
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fds->fd_S, 0 );
	s = (Semaphores*)mmap(NULL, sizeof(Semaphores), 
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fds->fd_sem, 0 );
	A = (int*)mmap(NULL, (sizeof(int) * ay * ax), 
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fds->fd_A, 0 );
	B = (int*)mmap(NULL, (sizeof(int) * by * bx), 
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, fds->fd_B, 0 );
}

/**
 * Initialise the semaphores in the global struct.
 */
void initialise_semaphores(){
	sem_init(&s->sem_full, 1, 0 );
	sem_init(&s->sem_empty, 1, 1 );
	sem_init(&s->sem_lock, 1, 1 );
}

/**
 * Destroy semaphores.
 */
void destroy_semaphores(){
	sem_destroy(&s->sem_full );
	sem_destroy(&s->sem_empty );
	sem_destroy(&s->sem_lock );
}

/**
 * Unmap and unlink shared memory elements.
 */
void destroy_shared_memory(File_descriptors *fds){
	/*Unmap shared memory.*/
	munmap(sub, sizeof(Subtotal) );
	munmap(s, sizeof(Semaphores) );
	munmap(A, sizeof(int) * ay * ax );
	munmap(B, sizeof(int) * by * bx );
	/*Unlink shared region.*/
	shm_unlink("shared_memory_S" );
	shm_unlink("shared_memory_sem" );
	shm_unlink("shared_memory_A" );
	shm_unlink("shared_memory_B" );
}

int main(int argc, char const *argv[] ){
	if(argc < 6 ){
		printf("Not enough arguments\n" );
		return EXIT_FAILURE;
	}

	/*Store the M N and K commandline arguments.*/
	get_dimensions(argv );

	/*Create file descripter struct. Put it on the stack so that there are no 
	issues with copies of it inside child processes lingering after the
	children exit.*/
	File_descriptors fds;
	
	/*Create shared memory.*/
	create_shared_memory(&fds );
	
	/*Create and initialise unnamed semaphors.*/
	initialise_semaphores();

	/*Read matricies from file.*/
	if(read_matrix(argv[1], A , ax * ay) || read_matrix(argv[2], B ,bx * by )){
		return EXIT_FAILURE;
	}

	/*Do the calculations etc.*/
	if(proc() == EXIT_FAILURE ){
		return EXIT_FAILURE;
	}

	printf("Total = %d\n\n", total );

	/*Clean up shared memory.*/
	destroy_semaphores();
	destroy_shared_memory(&fds );

	return EXIT_SUCCESS;
}