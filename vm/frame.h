#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <stdint.h>


struct frame_table_entry {
    void* upage;
    struct thread *t;
};

static struct lock ft_lock;
typedef struct frame_table {
    struct frame_table_entry * entry;
    size_t length;
} frame_table;

void frame_init(size_t frame_cnt);

void add_frame(struct thread* t, void* upage, void* kpage);
void add_frame_multiple(struct thread* t, void* upage, void* kpage, size_t page_cnt);
void remove_frame(void * kpage);
void remove_frame_multiple(void * kpage, size_t page_cnt);


struct frame_table* get_ftable();
bool swap_out_page();
void show_frame_table_status();
#endif