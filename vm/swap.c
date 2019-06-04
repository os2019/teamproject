#include "swap.h"

struct swap_table_entry{
    struct thread *t;
    void* upage;
};

typedef struct swap_table {
    struct swap_table_entry *entry;
    size_t length;
} swap_table;

static swap_table s_table;

void swap_init(){
    ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);

    static int BLOCK_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;
    struct block* swap = block_get_role(BLOCK_SWAP);
    s_table.length = block_size(swap) / BLOCK_PER_PAGE;
    printf("swap_init, size: %d\n", s_table.length / PGSIZE + 1);
    s_table.entry = palloc_get_multiple(PAL_ZERO, (s_table.length * sizeof(struct swap_table_entry)) / PGSIZE + 1);
    for(size_t i = 0; i < s_table.length; i++){
        s_table.entry->t = NULL;
        s_table.entry->upage = NULL;
    }
}

bool swap_out(struct thread* t,  void* upage, uint8_t* kpage){
    ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);
    static int BLOCK_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

    //해당 쓰레드의 pageDir에서 제거
    pagedir_clear_page(t->pagedir,upage);
    //물리 프레임과 프레임 테이블에서 제거
    palloc_free_page(kpage);

    /*swap table에서 공간 검색*/
    struct block* swap = block_get_role(BLOCK_SWAP);
    size_t idx;
    for(idx = 0; idx < s_table.length; idx++){
        if(s_table.entry == NULL || s_table.entry->t == NULL || s_table.entry->upage == NULL)
            break;
    }
    if(idx == s_table.length){
        /*디스크로 내부내는 코드 구현해야 함*/
        printf("error\n");
    }

    //swap space에 write
    for(int i = 0 ; i < BLOCK_PER_PAGE; i++){
        printf("sector index: %d, out from physical address %p\n", idx * BLOCK_PER_PAGE + i, kpage + i * BLOCK_SECTOR_SIZE);
        block_write(swap, idx * BLOCK_PER_PAGE + i, kpage + i * (BLOCK_SECTOR_SIZE));
    }
    /*swap table 업데이트*/
    s_table.entry[idx].t = t;
    s_table.entry[idx].upage = upage;

    /*debugging, 대충 swap table 전부 찍어보는 코드*/
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

    return true;
}
