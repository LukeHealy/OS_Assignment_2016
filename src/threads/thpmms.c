/**
 * Operating Systems Assignment Semester 1 2016
 * Part B. pmms using multiple threads.
 * 
 * Author: Luke Healy
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "matrix.h"

#define EMPTY 0
#define FULL !EMPTY

int *A;				//Matrix A
int *B;				//Matrix B
Subtotal sub;		//Effectively matrix C.
int ax, ay, bx, by;	//Matrix Dimensions.
int total = 0;		//Total of all subtotals.
int buffer = EMPTY;

pthread_mutex_t mutex;
pthread_cond_t empty;
pthread_cond_t full;

/**
 * Producer is the function that each child runs, it calculates the
 * subtotal of a row of the resultant matrix in parallel.
 */
void* producer(void *offset_param ){
	long offset = (long)(offset_param );
	int temp = 0;

	for(int i = offset; i < (offset + ax); i++ ){ //For each term in row of C.
		temp += calc(A[i], ((i - offset) * bx)-1, (i - offset) * bx, 0 );
	}
	pthread_mutex_lock(&mutex );
		while(buffer == FULL ){	//Wait until last value has been consumed
			pthread_cond_wait(&empty, &mutex );
		}
			sub.value = temp;
			sub.thid = syscall(__NR_gettid);
			buffer = FULL;
		pthread_cond_signal(&full );
	pthread_mutex_unlock(&mutex );

	pthread_exit(NULL );
}

/**
 * Consumer is the function that only the cons_thread runs  in order to
 * collect the subtotal made available by producer(). 
 * Also prints required information. 
 */
void* consumer(void *nothing){
	for(int i = 0; i < ay; i++ ){
		pthread_mutex_lock(&mutex );
			while(buffer == EMPTY ){ //Wait until value is available.
				pthread_cond_wait(&full, &mutex );
			}
				total += sub.value;
				printf("Thread %d: Subtotal = %d\n", sub.thid, sub.value );
				buffer = EMPTY;
			pthread_cond_signal(&empty );
		pthread_mutex_unlock(&mutex );
	}
	pthread_exit(NULL);
}

/**
 * proc() creates all needed threads for the producers. Also creates a thread
 * for the single consumer() so that all threads can be joined. 
 * Returns EXIT_FAILURE on failed thread join or creation.
 * offset is the length of a row of A.
 */
int proc(){
	pthread_t prod_threads[ay];
	pthread_t cons_thread;
	long offset = 0;
	
	for(int i = 0; i < ay; i++ ){
		if(pthread_create(&prod_threads[i], NULL, &producer, (void*)offset)){
			perror("Error creating thread");
			return EXIT_FAILURE;
		}
		offset+= ax;
	}
	if(pthread_create(&cons_thread, NULL, &consumer, NULL )){
		perror("Error creating thread");
		return EXIT_FAILURE;
	}

	for(int i = 0; i < ay; i++ ){
		if(pthread_join(prod_threads[i], NULL)){
			perror("Error joining producers");
			return EXIT_FAILURE;
		}
	}
	if(pthread_join(cons_thread, NULL)){
		perror("Error joining consumer");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * Recursively calculates the sum of a row of C[]. If the index of b[] is 
 * between that of the inital call (start of a row) and the length
 * of a row in the matrix then call calc again, incrementing idx by 1. 
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

int main(int argc, char const *argv[]){

	if(argc < 6){
		printf("Not enough arguments\n");
		return EXIT_FAILURE;
	}

	get_dimensions(argv );

	A = (int*)malloc(sizeof(int) * ax * ay );
	B = (int*)malloc(sizeof(int) * bx * by );

	/*Initialise sub.*/
	sub.value = 0;
	sub.thid = 0;

	/*Read from file.*/
	if(read_matrix(argv[1], A, ax * ay ) || read_matrix(argv[2], B, bx * by )){
		return EXIT_FAILURE;
	}

	/*Initialise mutex and conditions.*/
	pthread_mutex_init(&mutex, NULL );
	pthread_cond_init(&full, NULL );
	pthread_cond_init(&empty, NULL );

	/*Do the calculations etc.*/
	if(proc() == EXIT_FAILURE ){
		return EXIT_FAILURE;
	}

	printf("Total = %d\n", total );

	/*Destroy mutex and conditions.*/
	pthread_mutex_destroy(&mutex );
	pthread_cond_destroy(&full );
	pthread_cond_destroy(&empty );

	free(A );
	free(B );
	
	return EXIT_SUCCESS;
}