#include "buffer_pool.h"
#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>

void init_buffer_pool(const char *filename, BufferPool *pool) {
    FileInfo file;
    open_file(&file, filename);
    pool->file = file;
    for(int i = 0; i<CACHE_PAGE; i++){
        (pool->addrs)[i] = -1;
    }
    memset(pool->cnt, -1, sizeof(pool->cnt));
    memset(pool->ref, 0, sizeof(pool->ref));
}

void close_buffer_pool(BufferPool *pool) {
    FileInfo file = pool->file;
    for(auto i = 0; i<CACHE_PAGE; i++){
        if(1){
            write_page(&((pool->pages)[i]), &(pool->file), (pool->addrs)[i] );
            (pool->addrs)[i] = 0;
            (pool->cnt)[i] = 0;
            (pool->ref)[i] = 0;
        }
    }
    close_file(&file);
}

Page *get_page(BufferPool *pool, off_t addr) {
    // FILE *debug_fp = fopen("buffer.log", "a");
    // fprintf(debug_fp, "<<<<<<<<<buffer_pool\n");
    // fprintf(debug_fp, "get_page: addr=%lld\n", addr);
    // for(int i=0; i<8; i++){
    //     fprintf(debug_fp, "addrs[%d]=%lld\n", i, (pool->addrs)[i]);
    //     fprintf(debug_fp, "cnt[%d]=%lld\n", i, (pool->cnt)[i]);
    //     fprintf(debug_fp, "ref[%d]=%lld\n", i, (pool->ref)[i]);
    //     fprintf(debug_fp, "\n");
    // }
    // fprintf(debug_fp, "buffer_pool>>>>>>>>>>>>>>>\n");
    // fclose(debug_fp);

    short k = 0;
    short big = -1;
    short big_id = 0;
    for(k=0; k<CACHE_PAGE; k++){//if page hit
        if((pool->addrs)[k] == addr){
            (pool->cnt)[k] = (pool->cnt)[k] + 1;
            (pool->ref)[k] = (pool->ref)[k] + 1;
            return &((pool->pages)[k]);
        }
    }
    
    int target = -1;
    int sum = 0;
    int changeable_page[CACHE_PAGE] = {0};
    for (int i = 0; i != CACHE_PAGE; i++) {
        if ((pool->ref)[i] == 0) {
            changeable_page[sum] = i;
            sum++;
        }
    }
    target = changeable_page[0];
    for (int i = 1; i != sum; i++) {
        if (pool->cnt[target] > pool->cnt[changeable_page[i]]) {
            target = changeable_page[i];
        }
    }
    if (target == -1){
        return NULL;
    }
    if (pool->cnt[target] != -1) {
        write_page(&pool->pages[target], &pool->file, pool->addrs[target]);
    }
    FileIOResult result = read_page(&pool->pages[target], &pool->file, addr);
    if (result == ADDR_OUT_OF_RANGE && addr >= 0) {
        pool->file.length = addr + PAGE_SIZE;
        memset(&(pool->pages[target]), 0, PAGE_SIZE);
        pool->addrs[target] = addr;
        for (int i = 0; i != CACHE_PAGE; i++) {
            if (pool->cnt[i] != -1) pool->cnt[i] = 0;
        }
        pool->cnt[target] = 1;
        pool->ref[target] = 1;
        // fprintf(debug_fp, "get_page finish\n\n");
        // fclose(debug_fp);
        return &pool->pages[target];
    }
    if (result == FILE_IO_SUCCESS) {
        pool->addrs[target] = addr;
        for (int i = 0; i != CACHE_PAGE; i++) {
            if (pool->cnt[i] != -1) 
                pool->cnt[i] = 0;
        }
        pool->cnt[target] = 1;
        pool->ref[target] = 1;
        return &pool->pages[target];
    }
    
}

void release(BufferPool *pool, off_t addr) {
    // FILE *debug_fp = fopen("buffer.log", "a");
    // fprintf(debug_fp, "<<<<<<<<<buffer_pool\n");

    // fprintf(debug_fp, "release: addr=%lld\n", addr);

    // for(int i=0; i<8; i++){
    //     fprintf(debug_fp, "addrs[%d]=%lld\n", i, (pool->addrs)[i]);
    //     fprintf(debug_fp, "cnt[%d]=%lld\n", i, (pool->cnt)[i]);
    //     fprintf(debug_fp, "ref[%d]=%lld\n", i, (pool->ref)[i]);
    //     fprintf(debug_fp, "\n");
    // }
    // fprintf(debug_fp, "buffer_pool>>>>>>>>>>>>>>>\n");
    // fclose(debug_fp);
    
    for(short i=0; i<CACHE_PAGE; i++){
        if((pool->addrs)[i] == addr){
            (pool->ref)[i] = (pool->ref)[i] - 1;

            break;
        }
    }
    return;
}

/* void print_buffer_pool(BufferPool *pool) {
} */

/* void validate_buffer_pool(BufferPool *pool) {
} */
