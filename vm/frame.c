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
    uint8_t length;
} frame_table;

void frame_init(uint8_t frame_cnt){
    lock_init(&ft_lock);
    frame_table.length = frame_cnt;
    frame_table.entry = palloc_get_page(false);
}

void add_frame(struct thread* t, void* upage, void* kpage){
    //printf("%s %p %d\n", t->name, kpage, get_user_index(kpage));
    lock_acquire(&ft_lock);
    frame_table.entry[get_user_index(kpage)].upage = upage;
    frame_table.entry[get_user_index(kpage)].t = t;
    lock_release(&ft_lock);
}

void remove_frame(void * kpage){
    remove_frame_multiple(kpage,1);
}

void remove_frame_multiple(void * kpage, uint8_t page_cnt){
    lock_acquire(&ft_lock);
    for(int i = 0; i < page_cnt; i++){
        //printf("%s %p\n",frame_table.entry[get_user_index(kpage)+i].t->name,frame_table.entry[get_user_index(kpage)+i]);
        frame_table.entry[get_user_index(kpage)+i].upage = NULL;
        frame_table.entry[get_user_index(kpage)+i].t = NULL;
    }
    lock_release(&ft_lock);
}
