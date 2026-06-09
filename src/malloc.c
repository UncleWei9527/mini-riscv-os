#include "def.h"
extern char __bss_end[];
extern char __bss_start[];
#define HEAP_SIZE (1024 * 1024 * 2)

struct Block {

  int size;
  int is_free;
  struct Block *next;
};
static struct Block *free_list = 0;
// static int arr[1024*1024];
void malloc_init() {
  unsigned int start = (unsigned int)__bss_end;

  start = (start + 3) & ~3;
  free_list = (struct Block *)start;
  free_list->size = HEAP_SIZE - sizeof(struct Block);
  free_list->is_free = 1;
  free_list->next = NULL;
}
void *sys_malloc(int size) {
  if (size < 0)
    return (void*)0;
  size = (size + 3) & ~3;
  struct Block *curr = free_list;
  while (curr) {
    if (curr->is_free && curr->size >= size) {
      if (curr->size > size + sizeof(struct Block) + 4) {

        struct Block *new_block =
            (struct Block *)((char *)curr + sizeof(struct Block) + size);
        new_block->size = curr->size - size - sizeof(struct Block);
        new_block->is_free = 1;
        new_block->next = curr->next;
        curr->size = size;
        curr->is_free = 0;
        curr->next = new_block;
      }
      curr->is_free = 0;
      return (void *)((char *)curr + sizeof(struct Block));
    }
    curr = curr->next;
  }
  return NULL;
}
void sys_free(void *ptr) {
  if (ptr == NULL)
    return;
  struct Block *curr = (struct Block *)((char *)ptr - sizeof(struct Block));
  curr->is_free = 1;
}
