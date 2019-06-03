#include "userprog/process.h"
#include <inttypes.h>
#include <threads/palloc.h>
#include <string.h>
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include "frame.h"

struct frame_table_entry {
    void* upage;
    struct thread *t;
};

static struct lock ft_lock;
static struct {
    struct frame_table_entry * entry;
    size_t length;
} frame_table;

void frame_init(size_t frame_cnt){
    lock_init(&ft_lock);
    //frame_table.length = frame_cnt;
    frame_table.entry = palloc_get_page(false);
}

void add_frame(struct thread* t, void* upage, void* kpage){
    add_frame_multiple(t,upage,kpage,1);
}

void add_frame_multiple(struct thread* t, void* upage, void* kpage, size_t page_cnt){
    lock_acquire(&ft_lock);
    for(size_t i = 0; i < page_cnt; i++){
        printf("%s %p %p %d\n", t->name, upage, kpage, get_user_index(kpage) + i);
        frame_table.entry[get_user_index(kpage) + i].upage = upage;
        frame_table.entry[get_user_index(kpage) + i].t = t;
    }
    lock_release(&ft_lock);
}
void remove_frame(void * kpage){
    remove_frame_multiple(kpage,1);
}

void remove_frame_multiple(void * kpage, size_t page_cnt){
    lock_acquire(&ft_lock);
    for(size_t i = 0; i < page_cnt; i++){
        frame_table.entry[get_user_index(kpage)+i].upage = NULL;
        frame_table.entry[get_user_index(kpage)+i].t = NULL;
    }
    lock_release(&ft_lock);
}
