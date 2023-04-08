#include "hash_map.h"

#include <stdio.h>

void hash_table_init(const char *filename, BufferPool *pool, off_t n_directory_blocks) {
    init_buffer_pool(filename, pool);
    /* TODO: add code here */
    // FILE *debug_fp = fopen("hash_map_initiate.log", "a");
    // fprintf(debug_fp, "n_directory_blocks = %lld\n", n_directory_blocks);
    // fclose(debug_fp);

    if(pool->file.length != 0){
        return;
    }


    HashMapDirectoryBlock *init_block;
    off_t init_addr;
    
    for(off_t i = 1; i<=n_directory_blocks; i++){
        init_addr = i * PAGE_SIZE;
        init_block = (HashMapDirectoryBlock*)get_page(pool, init_addr);
        // for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
        //     init_block->directory[j] = 0;
        memset(init_block->directory, -1, PAGE_SIZE);
        release(pool, init_addr);
    }

    off_t ctrl_addr = 0;
    HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, ctrl_addr);
    ctrl->free_block_head = -1;
    ctrl->n_directory_blocks = n_directory_blocks;
    ctrl->max_size = PAGE_SIZE * (n_directory_blocks + 1);
    pool->file.length = ctrl->max_size;
    release(pool, ctrl_addr);
    




    // debug_fp = fopen("hash_map_initiate.log", "a");
    // fprintf(debug_fp, "hast_table_initiate\n");
    // HashMapDirectoryBlock *test_block;
    // off_t test_addr;
    // HashMapControlBlock *test_ctrl;
    // off_t test_ctrl_addr = 0;
    // test_ctrl = (HashMapControlBlock*)get_page(pool, test_ctrl_addr);
    // fprintf(debug_fp, "Control Information:\n");
    // fprintf(debug_fp, "max_size = %hd\n", test_ctrl->max_size);
    // fprintf(debug_fp, "n_directory_blocks = %hd\n", test_ctrl->n_directory_blocks);
    // release(pool, test_ctrl_addr);
    // for(off_t i = 0; i<=HASH_MAP_DIR_SIZE; i++){
    //     test_addr = i * PAGE_SIZE;
    //     test_block = (HashMapDirectoryBlock*)get_page(pool, test_addr);
    //     for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
    //         fprintf(debug_fp, "i=%lld j=%lld addr=%lld\n", i, j+(i-1)*HASH_MAP_DIR_BLOCK_SIZE, test_block->directory[j]);
    //     release(pool, test_addr);
    // }
    // fprintf(debug_fp, "\n");
    // fclose(debug_fp);
    
    return;
}


// void hash_table_init(const char *filename, BufferPool *pool, off_t n_directory_blocks) {//play version
//     init_buffer_pool(filename, pool);
//     /* TODO: add code here */
//     HashMapDirectoryBlock *init_block;
//     off_t init_addr;
//     // for(off_t i = 1; i<=HASH_MAP_DIR_SIZE; i++){//set 0
//     //     init_addr = i * PAGE_SIZE;
//     //     init_block = (HashMapDirectoryBlock*)get_page
//     //             (pool, init_addr);
//     //     for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
//     //         init_block->directory[j] = 0;
//     //     release(pool, init_addr);
//     // }
//     pool->file.length = PAGE_SIZE * 9;
//     init_addr = 1 * PAGE_SIZE;
//     init_block = (HashMapDirectoryBlock*)get_page
//             (pool, init_addr);
//     for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
//         init_block->directory[j] = 5;
//     release(pool, init_addr);
//     // init_addr = 0 * PAGE_SIZE;
//     // init_block = (HashMapDirectoryBlock*)get_page
//     //         (pool, init_addr);
//     // for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
//     //     init_block->directory[j] = 0;
//     // release(pool, init_addr);
//     FILE *debug_fp = fopen("hash_map_initiate''.log", "a");
//     fprintf(debug_fp, "hast_table_initiate\n");
//     HashMapDirectoryBlock *test_block;
//     off_t test_addr;
//     // HashMapControlBlock *test_ctrl;  
//     // test_ctrl = (HashMapControlBlock*)get_page(pool, 0);
//     // fprintf(debug_fp, "Control Information\n");
//     // fprintf(debug_fp, "max_size = %hd\n", test_ctrl->max_size);
//     // release(pool, 0);  
//     for(off_t i = 0; i<=8; i++){
//         test_addr = i * PAGE_SIZE;
//         test_block = (HashMapDirectoryBlock*)get_page(pool, test_addr);
//         for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
//             fprintf(debug_fp, "i=%lld j=%lld addr=%lld\n", i, j+(i-1)*HASH_MAP_DIR_BLOCK_SIZE, test_block->directory[j]);
//         release(pool, test_addr);
//     }
//     fprintf(debug_fp, "okkkkkkkkkk__________\n");
//     fprintf(debug_fp, "\n");
//     fclose(debug_fp);  
//     return;
// }

void hash_table_close(BufferPool *pool) {
    close_buffer_pool(pool);
}



void hash_table_insert(BufferPool *pool, short size, off_t addr) {
    // FILE *debug_fp = fopen("hash_map_insert.log", "a");
    // fprintf(debug_fp, "hast_table_insert\n");
    // fprintf(debug_fp, "size = %hd\n", size);
    // fprintf(debug_fp, "addr = %lld\n", addr);
    // HashMapDirectoryBlock *test_block;
    // off_t test_addr;
    // HashMapControlBlock *test_ctrl;
    // test_ctrl = (HashMapControlBlock*)get_page(pool, 0);
    // fprintf(debug_fp, "Control Information\n");
    // fprintf(debug_fp, "max_size = %hd\n", test_ctrl->max_size);
    // release(pool, 0);
    // for(off_t i = 1; i<=HASH_MAP_DIR_SIZE; i++){
    //     test_addr = i * PAGE_SIZE;
    //     test_block = (HashMapDirectoryBlock*)get_page
    //             (pool, test_addr);
    //     for(off_t j = 0; j<HASH_MAP_DIR_BLOCK_SIZE; j++)
    //         fprintf(debug_fp, "i=%lld j=%lld addr=%lld\n", i, j+(i-1)*HASH_MAP_DIR_BLOCK_SIZE, test_block->directory[j]);
    //     release(pool, test_addr);
    // }
    // fprintf(debug_fp, "\n");
    // fclose(debug_fp);









    off_t dir_addr = (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE;
    HashMapDirectoryBlock *dir_block = (HashMapDirectoryBlock*)get_page(pool,dir_addr);

    off_t cnt_addr = dir_block->directory[size % HASH_MAP_DIR_BLOCK_SIZE];
    if (cnt_addr == -1) {//1,if dir hasn't been inserted,add one HashMapBlock
        HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
        if (ctrl->free_block_head != -1) {//control block has free block
            HashMapBlock *free_block = (HashMapBlock*)get_page(pool, ctrl->free_block_head);
            off_t free_addr = ctrl->free_block_head;

            dir_block->directory[size % HASH_MAP_DIR_BLOCK_SIZE] = ctrl->free_block_head;
            ctrl->free_block_head = free_block->next;

            free_block->next = -1;
            free_block->n_items = 1;
            free_block->table[0] = addr;
            
            
            release(pool, free_addr);
            release(pool, dir_addr);
            release(pool, 0);
            return;
        }
        else {//if not, new one block at the end of file
            off_t free_addr = ctrl->max_size;
            dir_block->directory[size % HASH_MAP_DIR_BLOCK_SIZE] = free_addr;
            
            HashMapBlock *free_block = (HashMapBlock*)get_page(pool, free_addr);
            free_block->n_items = 1;
            free_block->table[0] = addr;
            free_block->next = -1;
            
            ctrl->max_size = ctrl->max_size + PAGE_SIZE;
            release(pool, dir_addr);
            release(pool, free_addr);
            release(pool, 0);
            return;
        }
    }
    else {//2,if dir has been inserted,find the first empty block
        HashMapBlock *cnt_block = (HashMapBlock*)get_page(pool, cnt_addr);
        if (cnt_block->n_items != HASH_MAP_BLOCK_SIZE) {//this page has empty space
            cnt_block->table[cnt_block->n_items] = addr;
            cnt_block->n_items++;

            release(pool, dir_addr);
            release(pool, cnt_addr);
            return;
        }
        else {//this page no empty space
            HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
            off_t free_addr = ctrl->max_size;
            
            HashMapBlock *free_block = (HashMapBlock*)get_page(pool, free_addr);
            free_block->next = cnt_addr;
            free_block->n_items = 1;
            free_block->table[0] = addr;
            
            
            dir_block->directory[size % HASH_MAP_DIR_BLOCK_SIZE] = free_addr;
            ctrl->max_size = ctrl->max_size + PAGE_SIZE;

            release(pool, cnt_addr);
            release(pool, free_addr);
            release(pool, 0);
            release(pool, dir_addr);
            return;
        }
    }
}



off_t hash_table_pop_lower_bound(BufferPool *pool, short size) {
    HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
    off_t n_directory_blocks = ctrl->n_directory_blocks;
    release(pool, 0);

    off_t dir_addr = (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE;
    HashMapDirectoryBlock *dir_block = (HashMapDirectoryBlock*)get_page(pool, dir_addr);
    off_t counter  = size % HASH_MAP_DIR_BLOCK_SIZE;
    off_t addr = -1;

    int flag = 0;
    while (1) {
        if((dir_block->directory)[counter] != -1 ){
            flag = 1;
            break;
        }
        if((dir_addr / PAGE_SIZE - 1) * HASH_MAP_DIR_BLOCK_SIZE + counter 
            >= n_directory_blocks * HASH_MAP_DIR_BLOCK_SIZE ){
            release(pool, dir_addr);
            break;
        }

        
        release(pool, dir_addr);
        if (++counter == HASH_MAP_DIR_BLOCK_SIZE) {
            counter = 0;
            dir_addr = dir_addr + PAGE_SIZE;
        }
        dir_block = (HashMapDirectoryBlock*)get_page(pool, dir_addr);
    }
    if ((dir_addr / PAGE_SIZE - 1) * HASH_MAP_DIR_BLOCK_SIZE + counter != n_directory_blocks * HASH_MAP_DIR_BLOCK_SIZE) {
        off_t map_block_addr = (dir_block->directory)[counter];
        
        HashMapBlock *map_block = (HashMapBlock*)get_page(pool, map_block_addr);
        addr = (map_block->table)[0];
        release(pool, dir_addr);
        release(pool, map_block_addr);
        hash_table_pop(pool, (dir_addr / PAGE_SIZE - 1) * HASH_MAP_DIR_BLOCK_SIZE + counter, addr);
        return addr;
    }
    release(pool, dir_addr);
    return addr;
}

void hash_table_pop(BufferPool *pool, short size, off_t addr) {
    off_t dir_addr, cnt_addr, pre_addr, nxt_addr;
    HashMapDirectoryBlock *dir_block = (HashMapDirectoryBlock*)get_page(pool, (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
    dir_addr = (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE;
    cnt_addr = (dir_block->directory)[size % HASH_MAP_DIR_BLOCK_SIZE];
    pre_addr = cnt_addr;

    release(pool, dir_addr);
    while (1) {
        off_t target = -1;

        HashMapBlock *cnt_block = (HashMapBlock*)get_page(pool, cnt_addr);
        nxt_addr = cnt_block->next;
        
        
        for (off_t i = 0; i != cnt_block->n_items; i++){
            if ((cnt_block->table)[i] == addr)    
                target = i;
        }
        if (target == -1) {//not found, check next page
            pre_addr = cnt_addr;
            release(pool, cnt_addr);
            cnt_addr = nxt_addr;
        }
        else {//found, pop target
            off_t n_items = cnt_block->n_items;
            if(n_items >1){//more than one item, pop directly
                for (off_t i = target + 1; i < n_items; i++) {//pop target
                    (cnt_block->table)[i - 1] = (cnt_block->table)[i];
                }
                cnt_block->n_items = cnt_block->n_items - 1;
                (cnt_block->table)[cnt_block->n_items] = 0;
                release(pool, cnt_addr);
                return;
            }
            else{//only one item, link it to control block, set size as zero
                if (pre_addr == cnt_addr) {
                    HashMapDirectoryBlock *pre_block = (HashMapDirectoryBlock*)get_page(pool, (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
                    (pre_block->directory)[size % HASH_MAP_DIR_BLOCK_SIZE] = cnt_block->next;
                    HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
                    off_t free_block_ptr = ctrl->free_block_head;
                    cnt_block->n_items = 0;
                    (cnt_block->table)[0] = 0;
                    cnt_block->next = free_block_ptr;
                    ctrl->free_block_head = cnt_addr;
                    release(pool, (size / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
                    release(pool, cnt_addr);
                    release(pool, 0);
                    return;
                }
                else {
                    HashMapBlock *pre_block = (HashMapBlock*)get_page(pool, pre_addr);
                    pre_block->next = cnt_block->next;
                    HashMapControlBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
                    off_t free_block_ptr = ctrl->free_block_head;
                    cnt_block->n_items = 0;
                    (cnt_block->table)[0] = 0;
                    cnt_block->next = free_block_ptr;
                    ctrl->free_block_head = cnt_addr;
                    release(pool, pre_addr);
                    release(pool, cnt_addr);
                    release(pool, 0);
                    return;
                }
            }
        }
    }
}




// void print_hash_table(BufferPool *pool) {
//     HashMapContro lBlock *ctrl = (HashMapControlBlock*)get_page(pool, 0);
//     HashMapDirectoryBlock *dir_block;
//     off_t block_addr, next_addr;
//     HashMapBlock *block;
//     int i, j;
//     printf("----------HASH TABLE----------\n");
//     for (i = 0; i < ctrl->max_size; ++i) {
//         dir_block = (HashMapDirectoryBlock*)get_page(pool, (i / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
//         if (dir_block->directory[i % HASH_MAP_DIR_BLOCK_SIZE] != 0) {
//             printf("%d:", i);
//             block_addr = dir_block->directory[i % HASH_MAP_DIR_BLOCK_SIZE];
//             while (block_addr != 0) {
//                 block = (HashMapBlock*)get_page(pool, block_addr);
//                 printf("  [" FORMAT_OFF_T "]", block_addr);
//                 printf("{");
//                 for (j = 0; j < block->n_items; ++j) {
//                     if (j != 0) {
//                         printf(", ");
//                     }
//                     printf(FORMAT_OFF_T, block->table[j]);
//                 }
//                 printf("}");
//                 next_addr = block->next;
//                 release(pool, block_addr);
//                 block_addr = next_addr;
//             }
//             printf("\n");
//         }
//         release(pool, (i / HASH_MAP_DIR_BLOCK_SIZE + 1) * PAGE_SIZE);
//     }
//     release(pool, 0);
//     printf("------------------------------\n");
// }