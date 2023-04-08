#include "str.h"

#include "table.h"

void read_string(Table *table, RID rid, StringRecord *record) {
    table_read(table, rid, (ItemPtr)&(record->chunk));
    record->idx = 0;
}

int has_next_char(StringRecord *record) {
    if (get_str_chunk_size(&(record->chunk)) != record->idx) 
        return 1;
    if (get_rid_idx(get_str_chunk_rid(&(record->chunk))) == -1) 
        return 0;
    return 1;
}

char next_char(Table *table, StringRecord *record) {
    if (get_str_chunk_size(&(record->chunk)) != record->idx) 
        return get_str_chunk_data_ptr(&(record->chunk))[record->idx++];
    RID rid = get_str_chunk_rid(&(record->chunk));
    if (get_rid_idx(rid) == -1) 
        return 0;
    table_read(table, rid, (ItemPtr)&(record->chunk));
    record->idx = 0;
    return get_str_chunk_data_ptr(&(record->chunk))[record->idx++];
}

int compare_string_record(Table *table, const StringRecord *a, const StringRecord *b) {
    StringRecord record_1 = *a, record_2 = *b;
    size_t sizeA = get_str_chunk_size(&(record_1.chunk)), sizeB = get_str_chunk_size(&(record_2.chunk));
    if (sizeA == 0) 
        return -1;
    if (sizeB == 0) 
        return 1;
    while (has_next_char(&record_1) && has_next_char(&record_2)) {
        char a, b;
        a = next_char(table, &record_1);
        b = next_char(table, &record_2);
        if (a != b) 
            return a > b ? 1 : -1;
    }
    if (has_next_char(&record_1) == 0 && has_next_char(&record_2) != 0) 
        return -1;
    if (has_next_char(&record_1) != 0 && has_next_char(&record_2) == 0) 
        return 1;
    return 0;
}

RID write_string(Table *table, const char *data, off_t size) {
    short extra_len;
    extra_len = size % (STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short));
    if (extra_len == 0)    
        extra_len = (STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short));
    // FILE *debug_fp = fopen("str.log", "a");

    // fprintf(debug_fp, "extra_len = %d\n", extra_len);

    // fclose(debug_fp);
    StringChunk tail_chunk;
    RID tail_rid;
    get_rid_block_addr(tail_rid) = -1;
    get_rid_idx(tail_rid) = -1;
    get_str_chunk_rid(&tail_chunk) = tail_rid;
    get_str_chunk_size(&tail_chunk) = extra_len;
    for (int i = 0; i != extra_len; i++) 
        get_str_chunk_data_ptr(&tail_chunk)[i] = data[size - extra_len + i];
    RID rid; 
    rid = table_insert(table, (ItemPtr)&tail_chunk, calc_str_chunk_size(extra_len));
    short sum = (size - extra_len) / (STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short));
    // FILE *debug_fp = fopen("str.log", "a");

    // fprintf(debug_fp, "sum = %d\n", sum);

    // fclose(debug_fp);
    for (int k = sum; k > 0; k--) {
        // FILE *debug_fp = fopen("str.log", "a");

        // fprintf(debug_fp, "sum = %d\n", sum);

        // fclose(debug_fp);
        StringChunk chunk;
        get_str_chunk_rid(&chunk) = rid;
        get_str_chunk_size(&chunk) = STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short);
        for (int i = 0; i != STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short); i++) 
            get_str_chunk_data_ptr(&chunk)[i] = data[(k - 1) * (STR_CHUNK_MAX_SIZE - sizeof(RID) - sizeof(short)) + i];
        rid = table_insert(table, (ItemPtr)&chunk, sizeof(chunk));
    }
    // FILE *debug_fp = fopen("str.log", "a");

    // fprintf(debug_fp, "rid = %d\n", rid);

    // fclose(debug_fp);
    return rid;
}

void delete_string(Table *table, RID rid) {
    while (1) {
        if(get_rid_idx(rid) == -1)
            break;
        StringChunk chunk;
        table_read(table, rid, (ItemPtr)&chunk);
        table_delete(table, rid);
        rid = get_str_chunk_rid(&chunk);
    }
}


size_t load_string(Table *table, const StringRecord *record, char *dest, size_t max_size) {
    StringRecord rec = *record;
    size_t size = 0;
    while (1) {
        if(has_next_char(&rec) == 0)
            break;
        if(size >= max_size)
            break;
        dest[size] = next_char(table, &rec);
        size++;
    }
    if (size < max_size)    
        dest[size] = 0;
    return size;
}
