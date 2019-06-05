#include "userprog/process.h"
#include <inttypes.h>
#include <threads/palloc.h>
#include <string.h>
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <stdio.h>
#include "frame.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"

static frame_table ftable;

void frame_init(size_t frame_cnt){
    lock_init(&ft_lock);
    //frame_table.length = frame_cnt;
    ftable.length = 367;
    ftable.entry = palloc_get_page(false);
}

void add_frame(struct thread* t, void* upage, void* kpage){
    add_frame_multiple(t,upage,kpage,1);
}

void add_frame_multiple(struct thread* t, void* upage, void* kpage, size_t page_cnt){
    lock_acquire(&ft_lock);
    for(size_t i = 0; i < page_cnt; i++){
        /*debugging*/
        //printf("adding... %s %p %p %d\n", t->name, upage, kpage, get_user_index(kpage) + i);
        ftable.entry[get_user_index(kpage) + i].upage = upage;
        ftable.entry[get_user_index(kpage) + i].t = t;
    }
    lock_release(&ft_lock);
}
void remove_frame(void * kpage){
    remove_frame_multiple(kpage,1);
}

void remove_frame_multiple(void * kpage, size_t page_cnt){
    /*debugging*/
    //printf("removing... frame address: %p\n", kpage);
    lock_acquire(&ft_lock);
    for(size_t i = 0; i < page_cnt; i++){
        ftable.entry[get_user_index(kpage)+i].upage = NULL;
        ftable.entry[get_user_index(kpage)+i].t = NULL;
    }
    lock_release(&ft_lock);
}

struct frame_table* get_ftable(){
    return &ftable;
}


/*clock*/
bool swap_out_page(){
    static uint32_t last_pointed = 0;
    /*단계 1*/
    for(uint32_t i = 0; i < ftable.length; i++){
        if(ftable.entry[last_pointed].t == NULL || ftable.entry[last_pointed].upage ==NULL){
            continue;
        }
        else if(!pagedir_is_accessed(ftable.entry[last_pointed].t->pagedir,ftable.entry[last_pointed].upage)
                && !pagedir_is_dirty(ftable.entry[last_pointed].t->pagedir,ftable.entry[last_pointed].upage)){
            //스왑아웃할때 쓰레드, upage, kpage 넘겨줌
            swap_out(ftable.entry[last_pointed].t, ftable.entry[last_pointed].upage, index_to_user_page(last_pointed));
            last_pointed = (last_pointed + 1) % ftable.length;
            return true;
        }
        last_pointed = (last_pointed + 1) % ftable.length;
    }
    /*단계 2*/
    for(uint32_t i = 0; i < ftable.length; i++){
        if(ftable.entry[last_pointed].t == NULL || ftable.entry[last_pointed].upage ==NULL){
            continue;
        }
        else if(!pagedir_is_accessed(ftable.entry[last_pointed].t->pagedir,ftable.entry[last_pointed].upage)){
            //스왑아웃할때 쓰레드, upage, kpage 넘겨줌
            swap_out(ftable.entry[last_pointed].t, ftable.entry[last_pointed].upage, index_to_user_page(last_pointed));
            last_pointed = (last_pointed + 1) % ftable.length;
            return true;
        }else{
            pagedir_set_accessed(ftable.entry[last_pointed].t->pagedir,ftable.entry[last_pointed].upage,false);
        }
        last_pointed = (last_pointed + 1) % ftable.length;
    }
    //스왑아웃할때 쓰레드, upage, kpage 넘겨줌
    swap_out(ftable.entry[last_pointed].t, ftable.entry[last_pointed].upage, index_to_user_page(last_pointed));
    last_pointed = (last_pointed + 1) % ftable.length;
    return true;
}

void show_frame_table_status(){
    lock_acquire(&ft_lock);
    for(size_t i = 0; i < ftable.length; i++){
       if(ftable.entry[i].t == NULL || ftable.entry[i].upage == NULL){
           continue;
       }
       else{
           printf("thread: %s, frame address: %p, user address: %p\n", ftable.entry[i].t->name, index_to_user_page(i),ftable.entry[i].upage);
       }
    }
    lock_release(&ft_lock);
}