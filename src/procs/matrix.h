#include <semaphore.h>

/**
 * Subtotal struct used to store the subtotal to be calculated 
 * by the prioducer and recorded by the consumer. I 
 */
typedef struct Subtotal{
	int value;
	int pid;
} Subtotal;

/**
 * Semaphores struct used to contain each semapore. Will reside in shared 
 * memory.
 */
typedef struct Semaphores{
	sem_t sem_empty;
	sem_t sem_lock;
	sem_t sem_full;
}Semaphores;

/**
 * File_descriptors struct used to contain each file descriptor used to
 * keep track of the created shared memory blocks. Implemented to enable the 
 * encapsulation of shared memory created/destruction etc in dedicated 
 * functions.
 */
typedef struct File_descriptors{
	int fd_S;
	int fd_A;
	int fd_B;
	int fd_sem;
}File_descriptors;

/*Function Declarations.*/
int read_matrix(const char*, int*, int );
int calc(int, int, int, int );
int proc();
void consumer();
void producer(int);
void get_dimensions(char const ** );
void create_shared_memory(File_descriptors* );
void destroy_shared_elements(File_descriptors* );