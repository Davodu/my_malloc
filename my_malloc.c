// function implementations 
#include "my_malloc.h"
#include <sys/time.h>

void * ff_malloc (size_t size){
  t_block b, last ;//, size_of_whole_block;
  size_t s;
  //aligning the requested size using the macro defined in my_malloc.h
  s = ALIGN_SIZE(size, sizeof(char*));
   size_of_used_block += size;
  //if base is initialized, we can proceed to implement malloc
  if(base){
    //first find a block 
    last = base; 
    b = find_block (&last,s);
    //size_of_whole_block=last;
    if (b){
      //size_of_whole_block=b;
      //size_of_whole_block->size += b->size;
      //is it big enough to be split
      if ((b->size - s)>= (BLOCK_SIZE + 4 ))
	split_block (b,s);
      b->free = 0;//mark the chunk as used
    } else {//otherwise we extend the heap(NO FITTING BLOCK )      
      b = extend_heap(last,s);
      if (!b)
	return NULL;
    }
  }
    else {//Heap is empty (first time allocation) 
      b = extend_heap(NULL,s);
      if(!b)
	return(NULL);
      base = b;
    }
  //size_of_whole_block->size += b->size;
  head = b;
  //  get_data_segment_size (&last);
  //get_data_segment_free_space_size(get_data_segment_size(&last);
  return (b-> data); 
}
//free implementation(finding the right chunk to be freed and avoiding space fragmentation)
void ff_free (void * ptr ){
  t_block b;
  if (valid_addr(ptr)){
    //we get the block address
    b = get_block(ptr);
    b->free = 1;
    //fusion with previous if possible 
    if (b->prev && b->prev->free) 
      b = fusion(b->prev);
    //then try to fusion with next
    if(b->next)
      fusion(b);
    else
      {
	//if we're the last block, we release memory 
	if(b->prev)
	  b->prev->next = NULL;
	else 
	  //no more block
	  base = NULL;
	brk(b);//put break at chunk position
      }
  }
}

//No. 2 Best fit algorithm                                                                                                                                                           
void * bf_malloc(size_t size){
  t_block b,last;//,size_of_whole_block;
  size_t s; 
  s = ALIGN_SIZE(size, sizeof(char*));
  //size_of_used_block += size;
  if (base){
    //find the best block
    last = base;
    b = find_best_block (&last, s);
    //size_of_whole_block=last;
    if (b){
      // size_of_whole_block->size += b->size;
      //is it big enough to be split                                                                                                                                                 
      if ((b->size - s)>= (BLOCK_SIZE + 4 ))
        split_block (b,s);
      b->free = 0;//mark the chunk as used                                                                                                                                           
    } else {//otherwise we extend the heap(NO FITTING BLOCK )                                                                                                                        
      b = extend_heap(last,s);
      if (!b)
        return NULL;
    }
  }
  else {//Heap is empty (first time allocation)                                                                                                                                      
    b = extend_heap(NULL,s);
    if(!b)
      return(NULL);
    base = b;
  }
  //size_of_whole_block->size += b->size;
  head = b;
  return (b-> data);
}

void bf_free (void * ptr ){
  t_block b;
  if (valid_addr(ptr)){
    //we get the block address		      
    b = get_block(ptr);
    b->free = 1;
    //fusion with previous if possible	       
    if (b->prev && b->prev->free)
      b = fusion(b->prev);
    //then try to fusion with next	       
    if(b->next)
      fusion(b);
    else
      //if we're the last block, we release memory                                      
      if(b->prev)
	b->prev->next = NULL;
      else
	//no more block                                                                                                                                                            
	base = NULL;
    brk(b);//put break at chunk position                                                                                                                                         
  }
}
void * wf_malloc (size_t size){
  t_block b, last;//, size_of_whole_block;
  size_t s;
  //aligning the requested size using the macro defined in my_malloc.h                                                                        
  s = ALIGN_SIZE(size, sizeof(char*));
  //if base is initialized, we can proceed to implement malloc                                                                                
  //size_of_used_block += size;
  if(base){
    //first find a block                                                                                                                      
    last = base;
    b = find_worst_block (&last,s);
    //size_of_whole_block=last;
    if (b){
      // size_of_whole_block->size += b->size;
      //is it big enough to be split                                                                                                          
      if((b->size - s)>= (BLOCK_SIZE + 4 ))
        split_block (b,s);
      b->free = 0;//mark the chunk as used                                                                                                    
    } else {//otherwise we extend the heap(NO FITTING BLOCK )                                                                                 
      b = extend_heap(last,s);
      if (!b)
        return NULL;
    }
  }
  else {//Heap is empty (first time allocation)                                                                                             
    b = extend_heap(NULL,s);
    if(!b)
      return(NULL);
    base = b;
  }
  //size_of_whole_block->size += b->size;
  head = b;
  return (b->data);
}

void wf_free (void * ptr ){
  t_block b;
  if (valid_addr(ptr)){
    //we get the block address                                                                                                                
    b = get_block(ptr);
    b->free = 1;
    //fusion with previous if possible                                                                                                        
    if (b->prev && b->prev->free)
      b = fusion(b->prev);
    //then try to fusion with next                                                                                                            
    if(b->next)
      fusion(b);
    else
      {
        //if we're the last block, we release memory                                                                                          
        if(b->prev)
          b->prev->next = NULL;
        else
          //no more block                                                                                                                     
          base = NULL;
        brk(b);//put break at chunk position                                                                                                  
      }
  }
}

//No. 2 Best fit algorithm
void * ts_malloc(size_t size)
{
    t_block b = NULL;
    size_t s;
    //s = align4(size);
    s = ALIGN_SIZE(size, sizeof(char*));
    //find the best block
    b = _find_best_block (s);
    if (b){
        //is it big enough to be split
        if ((b->size - s) >= (BLOCK_SIZE + sizeof(char *) )) {
            _split_block (b,s);
        }
    }
    
    if (!b) {
        b = _extend_heap(s);
        if (!b) {
            return(NULL);
        }
    }
    return b->data;
}

void ts_free (void * ptr )
{
    t_block b;
    if (_valid_addr(ptr)) {
        //we get the block address
        b = _get_block(ptr);
        fuse_and_release(b);
    }// else {
     //printf("Invalid pointer on free %p\n", ptr);
    // }
}


unsigned long get_data_segment_size(){
  return (unsigned long)_get_val.alloc_size ;
}

 unsigned long get_data_segment_free_space_size(){
   return (unsigned long)(_get_val.alloc_size - _get_val.chunk_size);
}
