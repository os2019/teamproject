#include "threads/thread.h"
#include "devices/block.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <stdio.h>

void swap_init();
bool swap_out(struct thread* t,  void* upage, uint8_t* kpage);