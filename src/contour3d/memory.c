#include <stdio.h>
#include <stdlib.h>
#define CONTOUR3D_INTERNAL
#include "internal.h"

struct node_t_ {
  void * ptr;
  struct node_t_ * next;
};
typedef struct node_t_ node_t;

// singly-linked list to store all allocated memory
static node_t * memory = NULL;

void * contour3d_calloc(const size_t nitems, const size_t size){
  FILE * stream = stderr;
  if(SIZE_MAX / nitems < size){
    fprintf(stream, "fatal error: request too much memory (%zu x %zu)\n", nitems, size);
    return NULL;
  }
  void * ptr = calloc(nitems, size);
  if(NULL == ptr){
    fprintf(stream, "fatal error: failed to allocate memory (%zu)\n", nitems * size);
    fflush(stream);
    return NULL;
  }
  node_t * node = calloc(1, sizeof(node_t));
  if(NULL == node){
    fprintf(stream, "fatal error: failed to allocate memory (%zu)\n", sizeof(node_t));
    fflush(stream);
    return NULL;
  }
  node->ptr = ptr;
  node->next = memory;
  memory = node;
  return ptr;
}

int contour3d_free(void * ptr){
  if(NULL == ptr){
    return 1;
  }
  node_t ** node = &memory;
  while(*node){
    if(ptr == (*node)->ptr){
      node_t * node_next = (*node)->next;
      free(*node);
      free(ptr);
      *node = node_next;
      return 0;
    }
    node = &((*node)->next);
  }
  return 1;
}

int contour3d_free_all(void){
  while(memory){
    node_t * next = memory->next;
    free(memory->ptr);
    free(memory);
    memory = next;
  }
  return 0;
}

