#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <stdint.h>


void frame_init(size_t frame_cnt);

void add_frame(struct thread* t, void* upage, void* kpage);
void add_frame_multiple(struct thread* t, void* upage, void* kpage, size_t page_cnt);
void remove_frame(void * kpage);
void remove_frame_multiple(void * kpage, size_t page_cnt);


#endif