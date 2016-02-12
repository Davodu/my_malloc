# my_malloc
Implementation of various allocation policies for malloc

This submission covers the entire scope of the project(non- thread safe and thead safe implementations of malloc and free where necessary )

There are three make files, all created as I progressed through the three main phases of the project. 
1)	“makefile1 ” which builds the executable files libmymalloc.so and mymalloc_test
2)	“makefile2 ” which builds the executable files for equal_size_allocs, large_range_rand_allocs, and small_range_rand_allocs

3)	“Makefile” which builds the executables  thread_test_malloc_free (or thread_test if name is changed )



PART A

Three different policy implementations of C library function malloc . The three policies were executed using the same basic structure. Contained in the my_malloc.h header file are various function implementations. These functions were used at different stages of each required policy. Below is a brief summary of how I broke these down into functions and macros with each function’s description.

a)   void *ff_malloc(size_t size);
     void ff_free(void *ptr);
     //Best Fit malloc/free
b)   void *bf_malloc(size_t size);
     void bf_free(void *ptr);
     //Worst Fit malloc/free
c)   void *wf_malloc(size_t size);
     void wf_free(void *ptr);
Each of the three allocation policy *_malloc functions call on these functions :

1) Aligned pointer- To align the pointer with  our integer size, there is a need to align the size of the data block in all *_malloc implementations. This was achieved by initializing a pointer using the following macro :

#define ALIGN_SIZE( sizeToAlign, PowerOfTwo ) (((sizeToAlign) + (PowerOfTwo) - 1) & ~((PowerOfTwo) - 1))

This macro takes two parameters , the size of the pointer to align and the size of a char* pointer(this makes it a universal aligner for both 32 and 64 bit operating systems )

2) Finding a chunk: This function comes in three different forms for the implementation of all three policies. It finds the free sufficiently wide chunk according to various policy requirements.

t_block find_block(t_block *last,size_t size)
t_block find_best_block(t_block *last,size_t size)
t_block find_worst_block(t_block *last,size_t size)

It returns the last fitting chunk, or NULL if none were found.

3) Extending the heap: Function to extend the heap if no fitting chunk is found (or if the heap is empty)
t_block extend_heap (t_block last, size_t s)
This function calls sbrk() and brk() when necessary.

4) Splitting large blocks – Function to split a block if it is wide enough to accommodate another extra chunk of the same size 
void split_block (t_block b , size_t s )


The *_free functions call on the following: 

1) Fusion ()– function to fuse block with neighboring blocks if they are free. It continues the free list by adjusting the *next and *prev fields after splitting.

t_block fusion(t_block b)

2) Input Pointer validation : Is it really a malloc’ ed pointer?

int valid_addr(void *p)

3) Getting a block : Getting a block from an address
t_block get_block(void *p)


All six functions were implemented and called on one or more of the above predefined functions before execution. Programs are appropriately commented for clarity.


Performance study: Performance study of the malloc() performance with different allocation policies. These functions were used for the calculation of fragmentation
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

A global structure (see below) was used to store the values of the requested size and the allocated memory(chunk_size) respectively.
struct get_val{
size_t alloc_size;
size_t chunk_size;
} _get_val;





The results of this study are tabulated below:

malloc & free patterns	Execution time (s)	Fragmentation 
•	equal_size_allocs	262.6483	0.99999
•	small_range_rand_allocs	153.8426	0.99998
•	large_range_rand_allocs	0.008876	0.99961

Conclusion: From the table above, it is clear that the large range random memory allocation function was the fastest to execute but had the most relative fragmentation. The equal size allocation took the longest to execute but had the least relative fragmentation.




PART B 

Thread-Safe malloc() implementation
For this section, using the best policy memory allocation, the following functions were executed.

     void *ts_malloc(size_t size);
     void ts_free(void *ptr);
1)Shared library Destructor: The constructor is run when a shared library is loaded, typically during program startup. The destructor is run when the shared library is unloaded, typically at program exit.

__attribute__((constructor)) void init(void) { MUTEX_INIT(); }
__attribute__((destructor))  void fini(void) { MUTEX_DESTROY(); }

2)Macros to quit program in case of failure(present in header file)

#define MUTEX_INIT()
#define MUTEX_LOCK()
#define MUTEX_UNLOCK()
#define MUTEX_DESTROY()

The aim is to lock mutexes and unlock at every exit possible. In the called multithreaded function there are multiple return  points and mutexes are unlocked at all return points in the called functions. I introduces two global pointers , declared in the header file.
static t_block head = NULL;
static t_block tail = NULL;

These eliminate the need for the *last pointer which traversed the list in the earlier functions to find the last allocated blocks. In the function   static t_block _find_best_block (size_t size) only the head of the list is passed because the tail is only necessary when the heap is extended. Also if we try to access the head of the free list, we must lock mutex, go throught the registry and check if a ptr is present.

The following function static void fuse_and_release(t_block b)  calls on the function static t_block _fusion(t_block b) to establish fusion with predecessor the later function does not require a lock because these are simple mutexes (as opposed to recursive mutexes) and synchronization can be established from the calling function.
