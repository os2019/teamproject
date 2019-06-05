#include "threads/thread.h"
#include "devices/block.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include "vm/frame.h"

void swap_init();
bool swap_out(struct thread* t,  void* upage, uint8_t* kpage);
bool swap_in(struct thread *t, void* upage, uint8_t *kpage);
void show_swap_table();