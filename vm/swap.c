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

bool swap_out(struct thread* t,  void* upage, uint8_t* kpage){
    ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);
    
    //해당 쓰레드의 pageDir에서 제거
    pagedir_clear_page(t->pagedir,upage);
    //물리 프레임과 frame table 제거
    palloc_free_page(kpage);

    /*swap table에서 공간 검색*/
    struct block* swap = block_get_role(BLOCK_SWAP);
    size_t idx;
    for(idx = 0; idx < s_table.length; idx++){
        if(s_table.entry == NULL || s_table.entry[idx].t == NULL || s_table.entry[idx].upage == NULL)
            break;
    }
    if(idx == s_table.length){
        /*디스크로 내부내는 코드 구현해야 함*/
        printf("swap space is full\n");
        return false;
    }

    //swap space에 write
    for(int i = 0 ; i < BLOCK_PER_PAGE; i++){
        //printf("sector index: %d, out from physical address %p\n", idx * BLOCK_PER_PAGE + i, kpage + i * BLOCK_SECTOR_SIZE);
        block_write(swap, idx * BLOCK_PER_PAGE + i, kpage + i * (BLOCK_SECTOR_SIZE));
    }
    /*swap table 업데이트*/
    s_table.entry[idx].t = t;
    s_table.entry[idx].upage = upage;
    s_table.entry[idx].writable = pagedir_is_writable(t->pagedir, upage);

    /*
    debugging, 대충 swap table 전부 찍어보는 코드
    for(size_t i = 0; i < s_table.length; i++){
        if(s_table.entry[i].t == NULL || s_table.entry[i].upage == NULL){
            
        }
        else{
            //printf("swap table :: index: %d, thread: %s, user page: %p\n",i, s_table.entry[i].t->name,s_table.entry[i].upage);
            void * p = palloc_get_page(false);
            for(int j = 0; j <BLOCK_PER_PAGE; j++){
                block_read(swap, i * BLOCK_PER_PAGE + j, p + BLOCK_SECTOR_SIZE * j);
                //printf("swap disk :: user page: %p\n", p + BLOCK_SECTOR_SIZE * j);
            }
            palloc_free_page(p);
        }
    }
    */
    return true;
}

bool swap_in(struct thread *t, void* upage, uint8_t kpage){
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
    }
    /*해당 쓰레드의 pageDIR에 추가*/
    pagedir_set_page(t->pagedir, upage, kpage,pagedir_is_writable(t->pagedir, upage));
    /*frame table 업데이트*/
    add_frame(t,upage,kpage);
    /*swap table 에서 제거*/
    s_table.entry[idx].t = NULL;
    s_table.entry[idx].upage = NULL;
    /*swap space에서 제거*/
    /*덮어쓰면 되기 때문에 필요 없다*/
}