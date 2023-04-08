#include "block.h"

#include "stdio.h"

void init_block(Block *block) {
    block->n_items = 0;
    block->head_ptr = (short)(block->data - (char*)block);
    block->tail_ptr = (short)sizeof(Block);
}

ItemPtr get_item(Block *block, short idx) {
    if (idx < 0 || idx >= block->n_items) {
        printf("get item error: idx is out of range\n");
        return NULL;
    }
    ItemID item_id = get_item_id(block, idx);
    if (get_item_id_availability(item_id)) {
        printf("get item error: item_id is not used\n");
        return NULL;
    }
    short offset = get_item_id_offset(item_id);
    return (char*)block + offset;
}

short new_item(Block *block, ItemPtr item, short item_size) {
    short n_items = block->n_items;
    for(short i = 0; i < block->n_items; i++){
        ItemID item_id = get_item_id(block, i);
        if (get_item_id_availability(item_id)) {
            get_item_id(block, i) = compose_item_id(0, block->tail_ptr - item_size, item_size);//modify id
            
            // char *cp_dst = block->tail_ptr - item_size, *cp = item;
            // for(; cp_dst < block->tail_ptr; cp++, cp_dst++){
            //     *cp_dst = *cp;
            // }
            char* cp = (char*)block + block->tail_ptr - item_size;
            memcpy(cp, item, item_size);

            block->tail_ptr = block->tail_ptr - item_size;//modify the parameters of block
            return i;
        }
    }
    short idx = block->n_items;
    get_item_id(block, n_items) = compose_item_id(0, block->tail_ptr - item_size, item_size);//modify id

    // char *cp_dst = block->tail_ptr - item_size, *cp = item;
    // for(; cp_dst < block->tail_ptr; cp++, cp_dst++){
    //     *cp_dst = *cp;
    // }

    char* cp = (char*)block + block->tail_ptr - item_size;
    memcpy(cp, item, item_size);

    block->n_items++;//modify the parameters of block
    block->tail_ptr = block->tail_ptr - item_size;
    block->head_ptr = block->head_ptr + sizeof(ItemID);
    return idx;
}

void delete_item(Block *block, short idx) {
    ItemID item_id = get_item_id(block, idx);
    short item_size = get_item_id_size(item_id);
    short item_offset = get_item_id_offset(item_id);

    get_item_id(block, idx) = compose_item_id(1, item_offset, item_size);
    block->tail_ptr = block->tail_ptr + item_size;

    char* cp_start = (char*)block + block->tail_ptr - item_size;
    char* cp_target = (char*)block + block->tail_ptr;
    for (int i = item_offset - block->tail_ptr + item_size - 1; i >= 0; i--) {
        cp_target[i] = cp_start[i];
    }//delete content, the one behind slides forward

    short n_items = block->n_items;
    for (int i = 0; i != n_items; i++) {
        ItemID item_id = get_item_id(block, i);
        short tmp_offset = get_item_id_offset(item_id);
        if (get_item_id_availability(item_id) == 0) {
            if(tmp_offset < item_offset){
                get_item_id(block, i) = compose_item_id(0, tmp_offset + item_size, get_item_id_size(item_id));
            }
        }
    }//modify id, elements before id move backword
    ItemID itemID_last = get_item_id(block, n_items - 1);
    if(get_item_id_availability(itemID_last)) {
        block->head_ptr -= sizeof(ItemID);
        block->n_items = block->n_items -1;
    }//set elements deleted unusable 
}

/* void str_printer(ItemPtr item, short item_size) {
    if (item == NULL) {
        printf("NULL");
        return;
    }
    short i;
    printf("\"");
    for (i = 0; i < item_size; ++i) {
        printf("%c", item[i]);
    }
    printf("\"");
}

void print_block(Block *block, printer_t printer) {
    short i, availability, offset, size;
    ItemID item_id;
    ItemPtr item;
    printf("----------BLOCK----------\n");
    printf("total = %d\n", block->n_items);
    printf("head = %d\n", block->head_ptr);
    printf("tail = %d\n", block->tail_ptr);
    for (i = 0; i < block->n_items; ++i) {
        item_id = get_item_id(block, i);
        availability = get_item_id_availability(item_id);
        offset = get_item_id_offset(item_id);
        size = get_item_id_size(item_id);
        if (!availability) {
            item = get_item(block, i);
        } else {
            item = NULL;
        }
        printf("%10d%5d%10d%10d\t", i, availability, offset, size);
        printer(item, size);
        printf("\n");
    }
    printf("-------------------------\n");
}

void analyze_block(Block *block, block_stat_t *stat) {
    short i;
    stat->empty_item_ids = 0;
    stat->total_item_ids = block->n_items;
    for (i = 0; i < block->n_items; ++i) {
        if (get_item_id_availability(get_item_id(block, i))) {
            ++stat->empty_item_ids;
        }
    }
    stat->available_space = block->tail_ptr - block->head_ptr 
        + stat->empty_item_ids * sizeof(ItemID);
}

void accumulate_stat_info(block_stat_t *stat, const block_stat_t *stat2) {
    stat->empty_item_ids += stat2->empty_item_ids;
    stat->total_item_ids += stat2->total_item_ids;
    stat->available_space += stat2->available_space;
}

void print_stat_info(const block_stat_t *stat) {
    printf("==========STAT==========\n");
    printf("empty_item_ids: " FORMAT_SIZE_T "\n", stat->empty_item_ids);
    printf("total_item_ids: " FORMAT_SIZE_T "\n", stat->total_item_ids);
    printf("available_space: " FORMAT_SIZE_T "\n", stat->available_space);
    printf("========================\n");
} */