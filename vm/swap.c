#include "swap.h"

struct swap_table_entry{
    struct thread *t;
    void* upage;
    bool writable;
};

typedef struct swap_table {
    struct swap_table_entry *entry;
    size_t length;
} swap_table;

static swap_table s_table;
static int BLOCK_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void swap_init(){
    ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);

    struct block* swap = block_get_role(BLOCK_SWAP);
    s_table.length = block_size(swap) / BLOCK_PER_PAGE;
    //printf("swap_init, size: %d\n", s_table.length / PGSIZE + 1);
    s_table.entry = palloc_get_multiple(PAL_ZERO, (s_table.length * sizeof(struct swap_table_entry)) / PGSIZE + 1);
    for(size_t i = 0; i < s_table.length; i++){
        s_table.entry[i].t = NULL;
        s_table.entry[i].upage = NULL;
        s_table.entry[i].writable = false;
    }
}

/*kpage에 있는, 쓰레드 t가 소유하고 있고 upage로 매핑된 페이지를 스왑 아웃 한다*/
bool swap_out(struct thread* t,  void* upage, uint8_t* kpage){
    ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);

    /*swap table에서 공간 검색*/
    struct block* swap = block_get_role(BLOCK_SWAP);
    size_t idx;
    for(idx = 0; idx < s_table.length; idx++){
        if(s_table.entry == NULL || s_table.entry[idx].t == NULL || s_table.entry[idx].upage == NULL)
            break;
    }
    if(idx == s_table.length){
        printf("swap space is full\n");
        return false;
    }

    //swap space에 write
    for(int i = 0 ; i < BLOCK_PER_PAGE; i++){
        //printf("swapping out... sector index: %d, out from physical address %p\n", idx * BLOCK_PER_PAGE + i, kpage + i * BLOCK_SECTOR_SIZE);
        block_write(swap, idx * BLOCK_PER_PAGE + i, kpage + i * (BLOCK_SECTOR_SIZE));
    }

    //해당 쓰레드의 pageDir에서 제거
    pagedir_clear_page(t->pagedir,upage);
    //물리 프레임과 frame table 제거
    palloc_free_page(kpage);

    /*swap table 업데이트*/
    s_table.entry[idx].t = t;
    s_table.entry[idx].upage = upage;
    s_table.entry[idx].writable = pagedir_is_writable(t->pagedir, upage);

    return true;
}

/*swap space에서 t,upage를 찾아 프레임 kpage에 쓴다*/
bool swap_in(struct thread *t, void* upage, uint8_t *kpage){
    struct block *swap = block_get_role(BLOCK_SWAP);
    /*swap table에서 해당 virtual address가 존재하는지 검색*/
    size_t idx;
    for(idx = 0; idx < s_table.length; idx++){
        if(s_table.entry == NULL || s_table.entry[idx].t == NULL || s_table.entry[idx].upage == NULL){
            /*do nothing*/
        }
        else if(s_table.entry[idx].t == t && s_table.entry[idx].upage == upage){
            break;
        }
    }
    /*원하는 페이지가 swap space에 존재하지 않는 경우*/
    if(idx == s_table.length){
        return false;
    }
    
    /*physical frame에 올림*/
    for(int i = 0; i <BLOCK_PER_PAGE; i++){
        block_read(swap, idx * BLOCK_PER_PAGE + i, kpage + BLOCK_SECTOR_SIZE * i);
        //printf("swapping in.... kernel page: %p \n", kpage+BLOCK_SECTOR_SIZE * i);
    }
    /*해당 쓰레드의 pageDIR에 추가*/
    pagedir_set_page(t->pagedir, s_table.entry[idx].upage, kpage, s_table.entry[idx].writable);
    /*frame table 업데이트*/
    add_frame(t, s_table.entry[idx].upage,kpage);
    /*swap table 에서 제거*/
    s_table.entry[idx].t = NULL;
    s_table.entry[idx].upage = NULL;
    /*swap space에서 제거*/
    /*덮어쓰면 되기 때문에 필요 없다*/

    return true;
}

void show_swap_table(){
    for(int idx = 0; idx < 12; idx++){
        if(s_table.entry == NULL || s_table.entry[idx].t == NULL || s_table.entry[idx].upage == NULL){
            /*do nothing*/
        }
        else{
            printf("swap table index %d : thread %s, upage %p\n",idx, s_table.entry[idx].t->name,s_table.entry[idx].upage);
        }
    }
}

void swap_release(struct thread *t){
    if(t == NULL){
        return;
    }
    for(int i = 0; i < s_table.length; i++){
        if(s_table.entry[i].t == t){
            s_table.entry[i].t = NULL;
            s_table.entry[i].upage = NULL;
            s_table.entry[i].writable = false;
        }
    }
}