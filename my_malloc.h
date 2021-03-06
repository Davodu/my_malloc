//function declarations using sbrk()system calls
#ifndef MY_MALLOC_H_INCLUDED 
#define MY_MALLOC_H_INCLUDED

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#define FAST 1 //Fast pointer validity check
#define BLOCK_SIZE sizeof(struct s_block)
//#define align4(x) (((((x)-1)>>2)<<2)+4)
#define ALIGN_SIZE( sizeToAlign, PowerOfTwo ) \
(((sizeToAlign) + (PowerOfTwo) - 1) & ~((PowerOfTwo) - 1))

//variable to indicate start of list

size_t size_of_used_block = 0;

static pthread_mutex_t lock;
static int mutex_inited = 0;

//fail return conditions
#define MUTEX_INIT() \
{ \
if (!mutex_inited) { \
if (pthread_mutex_init(&lock, NULL) != 0) { \
printf("\n mutex init failed\n"); \
exit(-1); \
} \
mutex_inited = 1; \
} \
}

#define MUTEX_LOCK() \
{ \
MUTEX_INIT() \
if (pthread_mutex_lock(&lock) != 0) { \
printf("\n mutex lock failed\n"); \
exit(-1); \
} \
}

#define MUTEX_UNLOCK() \
{ \
MUTEX_INIT() \
if (pthread_mutex_unlock(&lock) != 0) { \
printf("\n mutex unlock failed\n"); \
exit(-1); \
} \
}

#define MUTEX_DESTROY() \
{ \
MUTEX_INIT() \
if (pthread_mutex_destroy(&lock) != 0) { \
printf("\n mutex destroy failed\n"); \
exit(-1); \
} \
}


void * base = NULL;
//basic data structure for malloc
struct s_block {
  size_t size ;
  struct s_block * next;
  struct s_block * prev;
  int free;
  void * ptr;// to be sure a ptr was obtained by malloc(use- if b->ptr == b->data, tghen b is a valid block)
  char data[1];
}; 
typedef struct s_block * t_block;
static t_block head = NULL;
static t_block tail = NULL;
//class to hold stored values
struct get_val{
  size_t alloc_size;
  size_t chunk_size;
} _get_val;

//shared library constructor(if an entity needs to be initialized in the library)        
__attribute__((constructor)) void init(void) { MUTEX_INIT(); }
__attribute__((destructor))  void fini(void) { MUTEX_DESTROY(); }


//finding a chunk or return NULL if none is found
t_block find_block(t_block *last,size_t size) {
  t_block b = base;
  while (b && !(b-> free && b-> size >= size)){
    *last = b;
    b = b->next;
  }
  return (b);
}

//finding the best fit chunk
t_block find_best_block (t_block * last, size_t size){
  t_block best = NULL;
  t_block b = base;
  while (b!=NULL){
    if (b->free && (b->size >= size) && (best == NULL || b->size < best->size)){
      best = b;
      if (best->size == size ) {break;}    
    }
    *last = b;
    // size_of_whole_block += b->size;
    //size_of_used_block += size;
    b=b->next;
  } 
  return (b);
}
//worst fit algorithm 
t_block find_worst_block (t_block * last, size_t size){
  t_block b = base;
  size_t worst = b->size - size ;
  t_block worst_block = b; 
  //size_t  size_of_whole_block = 0;
  //size_t  size_of_used_block = 0;
  while (b!=NULL){
    if (b->free && (b->size >= size) && (worst == 65000 || b->size > worst_block->size)){
      worst_block = b;
      if (worst_block->size == 65000 ) {break;}
    }
    *last = b;
    //size_of_whole_block = *last;
    // size_of_whole_block->size += b->size;
    // size_of_used_block += size;
    b=b->next;
  }
 
  return (b);
}

//function to split the block if its more than twice the space we need
void split_block (t_block b , size_t s ){
  t_block new;
  new =(t_block) b->data + s; 
  new->size = b-> size - s - BLOCK_SIZE;
  new -> next = b->next;
  new->prev = b;
  new -> free =1;
  new->ptr = new->data;
  b->size = s;
  b->next = new;   
  //size_of_used_block = s;
  if (new->next)
    new ->next ->prev = new;
}
// adding a new block and extending the heap if no appropriate space was found
t_block extend_heap (t_block last, size_t s){
  int sb;
  t_block b;
  b = sbrk(0);
  sb = (int)sbrk(BLOCK_SIZE + s);
  if (sb<0)//if sbrk fails 
    return NULL;
  b->size = s;
  b->next = NULL;
  b->prev = last;
  b->ptr = b->data;
  if(last)
    last->next = b;
  b->free = 0;
  _get_val.chunk_size += b->size;
  _get_val.alloc_size += sb; 
  return (b);
}
		 
// get the block from an address
t_block get_block(void *p){
  char *tmp ;
  tmp = p;
  return (p = tmp -= BLOCK_SIZE);
}
//is the pointer address valid?
int valid_addr(void *p){
  if(base){
    if (p>base && p< sbrk(0)){
      return (p == (get_block(p))->ptr);
    }
  }
  return (0);
}
//fusioning with predecessor
t_block fusion(t_block b){
  if(b->next && b->next->free){
    b->size += BLOCK_SIZE + b->next ->size;
    b->next = b->next ->next;
    if (b->next)
      b->next ->prev = b;
  }
  return (b);
}
//MUTEX FUNCTION IMPLEMENTATIONS

//finding the best fit chunk
static t_block _find_best_block (size_t size)
{
    t_block best = NULL;
    t_block b;
    MUTEX_LOCK();
    b = head;
    while (b != NULL) {
        //printf("looking for a block %p \n", b);
        if ((b->free) &&
            (b->size >= size) &&
            ((best == NULL) || (b->size < best->size))) {
            
            best = b;
            if (best->size == size ) {
                break;
            }
        }
        b=b->next;
    }
    if (NULL != b) {
        b->free = 0; //Say it is taken!
    }
    MUTEX_UNLOCK();
    return (b);
}

//function to split the block if its more than twice the space we need
static void _split_block (t_block b , size_t s ){
    t_block new;
    new = (t_block)(b->data + s);
    new->size = b-> size - s - BLOCK_SIZE;
    new->prev = b;
    new->free = 1;
    new->ptr = new->data;
    b->size = s;
    
    MUTEX_LOCK();
    new->next = b->next;
    if (new->next) {
        new->next->prev = new;
    } else {
        tail = new;
    }
    b->next = new;
    MUTEX_UNLOCK();
}

// adding a new block and extending the heap if no appropriate space was found
static t_block _extend_heap (size_t s)
{
    t_block b;
    
    MUTEX_LOCK(); //Must lock as other threads may be freeing mem.
    b = sbrk(0);
    if (((void*) -1) == sbrk(BLOCK_SIZE + s)) {//if sbrk fails
        printf("Failed to extend process's memory\n");
        MUTEX_UNLOCK();
        return NULL;
    }
    b->size = s;
    b->next = NULL;
    b->ptr = b->data;
    
    b->prev = tail;
    if (tail) {
        tail->next = b;
    }
    tail = b;
    
    if (!head) {
        head = b;
    }
    b->free = 0;
    MUTEX_UNLOCK();
    return (b);
}


// get the block from an address
static t_block _get_block(void *p)
{
    return (t_block)(((char *)p - BLOCK_SIZE) + sizeof (char *));// Do not forget self!
}

//is the pointer address valid?
static int _valid_addr(void *p)
{
    int ret = 0;
#if FAST // define FAST for fast pointer validity checking
    MUTEX_LOCK();
    if (head) {
        if ((p > (void *)head) && (p < sbrk(0))){
            ret = (p == (get_block(p))->ptr);
        }
    }
    MUTEX_UNLOCK();
#else
    t_block b;
    MUTEX_LOCK();
    b = head;
    while (b!=NULL){
        //printf("%p ", b);
        if ((!b->free) && (b->ptr == p)) {
            ret = 1;
            break;
        } else if (b->ptr == p) {
            printf("The pointer is freed already\n");
        }
        b=b->next;
    }
    //printf("\n");
    MUTEX_UNLOCK();
#endif
    return (ret);
}

//fusioning with predecessor
static t_block _fusion(t_block b)
{
    //MUTEX_LOCK();because the parent locks it
    if (b->next && b->next->free){
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next) {
            b->next->prev = b;
        } else {
            tail = b;
        }
    }
    //MUTEX_UNLOCK();
    return (b);
}

static void fuse_and_release(t_block b)
{
    // We must fuse and try to free-up memory in one go as otherwise
    // given pointer may become invalid due to fusion by another thread.
    MUTEX_LOCK();
    b->free = 1;
    //fusion with previous if possible
    if (b->prev && b->prev->free) {
        //printf("We are consuming current\n");
        b = fusion(b->prev);
    }
    //then try to fusion with next
    if(b->next) {
        fusion(b);
    } else {
        //if we're the last block, we release memory
        if(b->prev) {
            b->prev->next = NULL;
            tail = b->prev;
        } else {
            //no more block
            head = NULL;
            tail = NULL;
        }
        if (-1 == brk(b)) {//put break at chunk position
            printf("brk failed\n");
        } else {
            //printf("We just shrunk\n");
        }
    }
    MUTEX_UNLOCK();
}


void * ff_malloc(size_t size);
void ff_free (void * ptr );


void * bf_malloc(size_t size);
void bf_free (void * ptr );


void * wf_malloc(size_t size);
void wf_free (void * ptr );

void * ts_malloc(size_t size);
void ts_free (void * ptr );

unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

#endif
