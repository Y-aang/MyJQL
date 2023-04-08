#include "myjql.h"

#include "buffer_pool.h"
#include "b_tree.h"
#include "table.h"
#include "str.h"

#include <stdlib.h>

typedef struct {
    RID key;
    RID value;
} Record;

void read_record(Table *table, RID rid, Record *record) {
    table_read(table, rid, (ItemPtr)record);
}

RID write_record(Table *table, const Record *record) {
    return table_insert(table, (ItemPtr)record, sizeof(Record));
}

void delete_record(Table *table, RID rid) {
    table_delete(table, rid);
}

BufferPool bp_idx;
Table tbl_rec;
Table tbl_str;

void myjql_init() {
    b_tree_init("rec.idx", &bp_idx);
    table_init(&tbl_rec, "rec.data", "rec.fsm");
    table_init(&tbl_str, "str.data", "str.fsm");
}

void myjql_close() {
    /* validate_buffer_pool(&bp_idx);
    validate_buffer_pool(&tbl_rec.data_pool);
    validate_buffer_pool(&tbl_rec.fsm_pool);
    validate_buffer_pool(&tbl_str.data_pool);
    validate_buffer_pool(&tbl_str.fsm_pool); */
    b_tree_close(&bp_idx);
    table_close(&tbl_rec);
    table_close(&tbl_str);
}

int row_row_cmp(RID rid1, RID rid2) {
    Record record1, record2;
    read_record(&tbl_rec, rid1, &record1);
    StringRecord str_record1, str_record2;
    read_string(&tbl_str, record1.key, &str_record1);
    read_record(&tbl_rec, rid2, &record2);
    read_string(&tbl_str, record2.key, &str_record2);


    if (get_str_chunk_size(&(str_record1.chunk)) == 0) 
        return -1;
    if (get_str_chunk_size(&(str_record2.chunk)) == 0) 
        return 1;


    while (1) {
        if(has_next_char(&str_record1) == 0 )
            break;
        if(has_next_char(&str_record2) == 0 )
            break;
        char a, b;
        a = next_char(&tbl_str, &str_record1);
        b = next_char(&tbl_str, &str_record2);
        if (a != b) 
            return a > b ? 1 : -1;
    }
    if (!has_next_char(&str_record1) && has_next_char(&str_record2)) 
        return -1;
    if (has_next_char(&str_record1) && !has_next_char(&str_record2)) 
        return 1;
    return 0;
}

int ptr_row_cmp(void *key, size_t key_len, RID rid) {
    Record record;
    read_record(&tbl_rec, rid, &record);
    StringRecord str_record;
    read_string(&tbl_str, record.key, &str_record);
    if (get_str_chunk_size(&(str_record.chunk)) == 0) 
        return 1;
    size_t i = 0;
    while (has_next_char(&str_record) && i != key_len) {
        char a, b;
        a = ((char*)key)[i];
        i++;
        b = next_char(&tbl_str, &str_record);
        if (a != b) 
            return a > b ? 1 : -1;
    }
    if (i == key_len && has_next_char(&str_record)) 
        return -1;
    if (i != key_len && !has_next_char(&str_record)) 
        return 1;
    return 0;
}

typedef struct {
    RID chunk;
    struct block_chunk *next_chunk;
} block_chunk;

RID insert_handler(RID rid) {
    Record record;
    read_record(&tbl_rec, rid, &record);
    RID chunk_rid = record.key;
    block_chunk *front = NULL;
    while (1) {
        if(get_rid_block_addr(chunk_rid) == -1)
            break;
        block_chunk *pre = (block_chunk*)malloc(sizeof(block_chunk));
        pre->chunk = chunk_rid;
        pre->next_chunk = front;
        front = pre;
        StringChunk tmp;
        table_read(&tbl_str, chunk_rid, &tmp);
        chunk_rid = get_str_chunk_rid(&tmp);
        if (get_rid_block_addr(chunk_rid) == -1)
            break;
    }
    block_chunk *ptr_chunk = front;
    StringChunk new_chunk;
    table_read(&tbl_str, front->chunk, &new_chunk);
    short size;
    size = calc_str_chunk_size(get_str_chunk_size(&new_chunk));
    RID nxt_chunk;
    while (1) {
        if(ptr_chunk == NULL)
            break;
        nxt_chunk = table_insert(&tbl_str, &new_chunk, size);
        front = ptr_chunk->next_chunk;
        free(ptr_chunk);
        ptr_chunk = front;
        if (ptr_chunk != NULL) {
            table_read(&tbl_str, front->chunk, &new_chunk);
            get_str_chunk_rid(&new_chunk) = nxt_chunk;
            size = sizeof(new_chunk);
        }
    }
    Record new_record;
    new_record.key = nxt_chunk;
    get_rid_idx(new_record.value) = -1;
    get_rid_block_addr(new_record.value) = -1;
    RID result;
    result = write_record(&tbl_rec, &new_record);
    return result;
}

void delete_handler(RID rid){
    Record record;
    read_record(&tbl_rec, rid, &record);
    delete_string(&tbl_str, record.key);
    delete_string(&tbl_str, record.value);
    delete_record(&tbl_rec, rid);
    return;
}

size_t myjql_get(const char *key, size_t key_len, char *value, size_t max_size) {
    // b_tree_ptr_row_cmp_t cmp;
    // cmp = ptr_row_cmp;
    RID bp_result;
    bp_result = b_tree_search(&bp_idx, key, key_len, &ptr_row_cmp);
    if(get_rid_block_addr(bp_result)==-1 && get_rid_idx(bp_result)==0){//if key is invalid
        return -1;
    }
    Record table_record; 
    read_record(&tbl_rec, bp_result, &table_record);
    StringRecord string_record;
    read_string(&tbl_str, table_record.value, &string_record);
    size_t result;
    result = load_string(&tbl_str, &string_record, value, max_size);
    return result;
}

void myjql_set(const char *key, size_t key_len, const char *value, size_t value_len) {
    // b_tree_ptr_row_cmp_t cmp_pr, cmp_rr;
    // cmp_pr = ptr_row_cmp;
    // cmp_rr = row_row_cmp;
    RID bp_result;
    
    bp_result = b_tree_search(&bp_idx, key, key_len, &ptr_row_cmp);
    if(get_rid_block_addr(bp_result)==-1 && get_rid_idx(bp_result)==0){//key not found, insert the key
        RID key_rid, value_rid;
        key_rid = write_string(&tbl_str, key, key_len);
        value_rid = write_string(&tbl_str, value, value_len);
        Record record;
        record.key = key_rid;
        record.value = value_rid;
        RID record_rid;
        record_rid = write_record(&tbl_rec, &record);
        b_tree_insert(&bp_idx, record_rid, &row_row_cmp, &insert_handler);
        return;
    }
    else{//key already exists, update it
        Record record;
        read_record(&tbl_rec, bp_result, &record);
        delete_string(&tbl_str, record.value);
        RID new_value_rid;
        new_value_rid = write_string(&tbl_str, value, value_len);
        b_tree_delete(&bp_idx, bp_result, &row_row_cmp, &insert_handler, &delete_handler);
        delete_record(&tbl_rec, bp_result);
        record.value = new_value_rid;
        RID new_bp_result;
        new_bp_result = write_record(&tbl_rec, &record);
        b_tree_insert(&bp_idx, new_bp_result, &row_row_cmp, &insert_handler);
        return;
    }
    return;
}

void myjql_del(const char *key, size_t key_len) {
    // b_tree_ptr_row_cmp_t cmp_pr, cmp_rr;
    // cmp_pr = ptr_row_cmp;
    // cmp_rr = row_row_cmp;
    RID bp_result;
    // b_tree_insert_nonleaf_handler_t insert_handler;
    // b_tree_delete_nonleaf_handler_t delete_handler;
    // insert_handler = insert_handler;
    // delete_handler = delete_handler;
    bp_result = b_tree_search(&bp_idx, key, key_len, &ptr_row_cmp);
    if(get_rid_block_addr(bp_result)==-1 && get_rid_idx(bp_result)==0){//if key is invalid
        return;
    }
    b_tree_delete(&bp_idx, bp_result, &row_row_cmp, insert_handler, delete_handler);
    Record table_record; 
    read_record(&tbl_rec, bp_result, &table_record);
    delete_string(&tbl_str, table_record.key);
    delete_string(&tbl_str, table_record.value);
    delete_record(&tbl_rec, bp_result);
    return;
}

/* void myjql_analyze() {
    printf("Record Table:\n");
    analyze_table(&tbl_rec);
    printf("String Table:\n");
    analyze_table(&tbl_str);
} */