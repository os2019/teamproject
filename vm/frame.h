#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <stdint.h>


void frame_init(uint8_t frame_cnt);

void add_frame(struct thread* t, void* upage, void* kpage);

void remove_frame(void * kpage);
void remove_frame_multiple(void * kpage, uint8_t page_cnt);


#endif