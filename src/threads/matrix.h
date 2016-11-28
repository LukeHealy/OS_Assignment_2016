/**
 * Subtotal struct used to store the subtotal to be calculated 
 * by the prioducer and recorded by the consumer. I 
 */
typedef struct Subtotal{
	int value;
	int thid;
} Subtotal;

//Function declarations
int read_matrix(const char*, int*, int );
int calc(int, int, int, int );
int proc();
void get_dimensions(char const **);
void* consumer(void*);
void* producer(void*);