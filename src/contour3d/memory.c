#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // SIZE_MAX
#include "./memory.h"
#include "./logger.h"

// singly-linked list to store all allocated memory
typedef struct node_t {
  void * ptr;
  struct node_t * next;
} node_t;
static node_t * memory = NULL;

void * contour3d_memory_alloc (
    const size_t nitems,
    const size_t size
) {
  if (SIZE_MAX / nitems < size) {
    logger_error("request too much memory (%zu x %zu)\n", nitems, size);
    return NULL;
  }
  void * const ptr = malloc(nitems * size);
  if (NULL == ptr) {
    logger_error("failed to allocate memory (%zu)\n", nitems * size);
    return NULL;
  }
  node_t * const node = malloc(1 * sizeof(node_t));
  if (NULL == node) {
    logger_error("failed to allocate memory (%zu)\n", sizeof(node_t));
    return NULL;
  }
  node->ptr = ptr;
  node->next = memory;
  memory = node;
  return ptr;
}

int contour3d_memory_free (
    void * const ptr
) {
  if (NULL == ptr) {
    return 0;
  }
  node_t ** node = &memory;
  while (*node) {
    if (ptr == (*node)->ptr) {
      node_t * const node_next = (*node)->next;
      free(*node);
      free(ptr);
      *node = node_next;
      return 0;
    }
    node = &(*node)->next;
  }
  logger_error("this memory is not allocated");
  return 1;
}

int contour3d_memory_free_all (
    void
) {
  while (memory) {
    node_t * const next = memory->next;
    free(memory->ptr);
    free(memory);
    memory = next;
  }
  return 0;
}

