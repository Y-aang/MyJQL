#include "b_tree.h"
#include "buffer_pool.h"

#include <stdio.h>

typedef struct {
    RID child_key;
    off_t child_ptr;
} NewChildEntry;

typedef int OldChildEntry;

NewChildEntry insert_entry(BufferPool *pool, off_t addr, RID rid, b_tree_row_row_cmp_t cmp, off_t root_node_addr, b_tree_insert_nonleaf_handler_t insert_handler) {
    BNode *node = (BNode*)get_page(pool, addr);
    if (!node->leaf) {
        off_t addr_child;
        int prt_child = 0;
        for (; prt_child != node->n; prt_child++) {
            if ((*cmp)(rid, node->row_ptr[prt_child]) < 0) {
                addr_child = node->child[prt_child];
                break;
            }
        }
        addr_child = node->child[prt_child];
        NewChildEntry child_new;
        release(pool, addr);
        child_new = insert_entry(pool, addr_child, rid, cmp, root_node_addr, insert_handler);
        node = (BNode*)get_page(pool, addr);
        if (child_new.child_ptr == -1) {
            release(pool, addr);
            return child_new;
        }
        else {
            if (node->n < 2 * DEGREE) {
                int tmp = 0;
                for (tmp = 0; tmp < node->n; tmp++) {
                    if ((*cmp)(child_new.child_key, node->row_ptr[tmp]) < 0) {
                        break;
                    }
                }
                for (int i = node->n; i > tmp; i--) {
                    node->row_ptr[i] = node->row_ptr[i - 1];
                }
                for (int i = node->n + 1; i > tmp + 1; i--) {
                    node->child[i] = node->child[i - 1];
                }
                node->row_ptr[tmp] = child_new.child_key;
                node->child[tmp + 1] = child_new.child_ptr;
                node->n++;
                release(pool, addr);
                NewChildEntry result_empty;
                get_rid_block_addr(result_empty.child_key) = -1;
                get_rid_idx(result_empty.child_key) = -1;
                result_empty.child_ptr = -1;
                return result_empty;
            }
            else {
                if (addr == root_node_addr) {
                    off_t addr_notleaf_new;
                    BNode *node_notleaf_new;
                    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                    if (ctrl->free_node_head != -1) {
                        addr_notleaf_new = ctrl->free_node_head;
                        node_notleaf_new = (BNode*)get_page(pool, addr_notleaf_new);
                        ctrl->free_node_head = node_notleaf_new->next;
                    }
                    else {
                        addr_notleaf_new = pool->file.length;
                        node_notleaf_new = (BNode*)get_page(pool, pool->file.length);
                    }
                    off_t addr_root_new;
                    BNode *node_new_root;
                    if (ctrl->free_node_head != -1) {
                        addr_root_new = ctrl->free_node_head;
                        node_new_root = (BNode*)get_page(pool, addr_root_new);
                        ctrl->free_node_head = node_new_root->next;
                    }
                    else {
                        addr_root_new = pool->file.length;
                        node_new_root = (BNode*)get_page(pool, pool->file.length);
                    }
                    release(pool, 0);
                    node_notleaf_new->n = DEGREE;
                    node_notleaf_new->leaf = 0;
                    node_notleaf_new->next = -1;
                    NewChildEntry New_child_entry;
                    int child_pos = 0;
                    for (; child_pos != 2 * DEGREE; child_pos++) {
                        if((*cmp)(child_new.child_key, node->row_ptr[child_pos]) < 0) {
                            break;
                        }
                    }
                    if (child_pos < DEGREE) {
                        New_child_entry.child_key = node->row_ptr[DEGREE - 1];
                        New_child_entry.child_ptr = addr_notleaf_new;
                        for (int i = 0; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i] = node->child[DEGREE + i];
                        }
                        node_notleaf_new->child[DEGREE] = node->child[2 * DEGREE];
                        for (int i = DEGREE - 1; i > child_pos; i--) {
                            node->row_ptr[i] = node->row_ptr[i - 1];
                            node->child[i + 1] = node->child[i];
                        }
                        node->row_ptr[child_pos] = child_new.child_key;
                        node->child[child_pos + 1] = child_new.child_ptr;
                    }
                    else if (child_pos == DEGREE) {
                        New_child_entry.child_key = child_new.child_key;
                        New_child_entry.child_ptr = addr_notleaf_new;
                        for (int i = 0; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 1];
                        }
                        node_notleaf_new->child[0] = child_new.child_ptr;
                    }
                    else {
                        New_child_entry.child_key = node->row_ptr[DEGREE];
                        New_child_entry.child_ptr = addr_notleaf_new;
                        node_notleaf_new->child[0] = node->child[DEGREE + 1];
                        for (int i = 0; i != child_pos - DEGREE - 1; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i + 1];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 2];
                        }
                        node_notleaf_new->row_ptr[child_pos - DEGREE - 1] = child_new.child_key;
                        node_notleaf_new->child[child_pos - DEGREE] = child_new.child_ptr;
                        for (int i = child_pos - DEGREE; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 1];
                        }
                    }
                    node->n = DEGREE;
                    release(pool, addr);
                    release(pool, addr_notleaf_new);
                    node_new_root->leaf = 0;
                    node_new_root->n = 1;
                    node_new_root->child[0] = addr;
                    node_new_root->child[1] = addr_notleaf_new;
                    node_new_root->next = -1;
                    node_new_root->row_ptr[0] = New_child_entry.child_key;
                    release(pool, addr_root_new);
                    ctrl = (BCtrlBlock*)get_page(pool, 0);
                    ctrl->root_node = addr_root_new;
                    release(pool, 0);
                    return New_child_entry;
                }
                else {
                    off_t addr_notleaf_new;
                    BNode *node_notleaf_new;
                    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                    if (ctrl->free_node_head != -1) {
                        addr_notleaf_new = ctrl->free_node_head;
                        node_notleaf_new = (BNode*)get_page(pool, addr_notleaf_new);
                        ctrl->free_node_head = node_notleaf_new->next;
                    }
                    else {
                        addr_notleaf_new = pool->file.length;
                        node_notleaf_new = (BNode*)get_page(pool, pool->file.length);
                    }
                    release(pool, 0);
                    node_notleaf_new->n = DEGREE;
                    node_notleaf_new->leaf = 0;
                    node_notleaf_new->next = -1;
                    NewChildEntry New_child_entry;
                    int child_pos = 0;
                    for (; child_pos != 2 * DEGREE; child_pos++) {
                        if((*cmp)(child_new.child_key, node->row_ptr[child_pos]) < 0) {
                            break;
                        }
                    }
                    if (child_pos < DEGREE) {
                        New_child_entry.child_key = node->row_ptr[DEGREE - 1];
                        New_child_entry.child_ptr = addr_notleaf_new;
                        for (int i = 0; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i] = node->child[DEGREE + i];
                        }
                        node_notleaf_new->child[DEGREE] = node->child[2 * DEGREE];
                        for (int i = DEGREE - 1; i > child_pos; i--) {
                            node->row_ptr[i] = node->row_ptr[i - 1];
                            node->child[i + 1] = node->child[i];
                        }
                        node->row_ptr[child_pos] = child_new.child_key;
                        node->child[child_pos + 1] = child_new.child_ptr;
                    }
                    else if (child_pos == DEGREE) {
                        New_child_entry.child_key = child_new.child_key;
                        New_child_entry.child_ptr = addr_notleaf_new;
                        for (int i = 0; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 1];
                        }
                        node_notleaf_new->child[0] = child_new.child_ptr;
                    }
                    else {
                        New_child_entry.child_key = node->row_ptr[DEGREE];
                        New_child_entry.child_ptr = addr_notleaf_new;
                        node_notleaf_new->child[0] = node->child[DEGREE + 1];
                        for (int i = 0; i != child_pos - DEGREE - 1; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i + 1];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 2];
                        }
                        node_notleaf_new->row_ptr[child_pos - DEGREE - 1] = child_new.child_key;
                        node_notleaf_new->child[child_pos - DEGREE] = child_new.child_ptr;
                        for (int i = child_pos - DEGREE; i != DEGREE; i++) {
                            node_notleaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                            node_notleaf_new->child[i + 1] = node->child[DEGREE + i + 1];
                        }
                    }
                    node->n = DEGREE;
                    release(pool, addr);
                    release(pool, addr_notleaf_new);
                    return New_child_entry;
                }
            }
        }
    }
    else {
        if (node->n < 2 * DEGREE) {
            int counter = 0;
            for (; counter < node->n; counter++) {
                if((*cmp)(rid, node->row_ptr[counter]) < 0) {
                    break;
                }
            }
            for (int i = node->n; i > counter ; i--) {
                node->row_ptr[i] = node->row_ptr[i - 1];
            }
            node->row_ptr[counter] = rid;
            node->n++;
            release(pool, addr);
            NewChildEntry result_empty;
            get_rid_block_addr(result_empty.child_key) = -1;
            get_rid_idx(result_empty.child_key) = -1;
            result_empty.child_ptr = -1;
            return result_empty;
        }
        else {
            if (addr == root_node_addr) {
                off_t addr_leaf_new;
                BNode *node_leaf_new;
                BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                if (ctrl->free_node_head != -1) {
                    addr_leaf_new = ctrl->free_node_head;
                    node_leaf_new = (BNode*)get_page(pool, addr_leaf_new);
                    ctrl->free_node_head = node_leaf_new->next;
                }
                else {
                    addr_leaf_new = pool->file.length;
                    node_leaf_new = (BNode*)get_page(pool, pool->file.length);
                }
                off_t addr_root_new;
                BNode *node_new_root;
                if (ctrl->free_node_head != -1) {
                    addr_root_new = ctrl->free_node_head;
                    node_new_root = (BNode*)get_page(pool, addr_root_new);
                    ctrl->free_node_head = node_new_root->next;
                }
                else {
                    addr_root_new = pool->file.length;
                    node_new_root = (BNode*)get_page(pool, pool->file.length);
                }
                release(pool, 0);
                node_leaf_new->n = DEGREE + 1;
                node_leaf_new->leaf = 1;
                node_leaf_new->next = -1;
                int child_pos = 0;
                for (; child_pos != 2 * DEGREE; child_pos++) {
                    if((*cmp)(rid, node->row_ptr[child_pos]) < 0) {
                        break;
                    }
                }
                if (child_pos < DEGREE) {
                    for (int i = 0; i <= DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[DEGREE - 1 + i];
                    }
                    for (int i = DEGREE - 1; i > child_pos; i--) {
                        node->row_ptr[i] = node->row_ptr[i - 1];
                    }
                    node->row_ptr[child_pos] = rid;
                }
                else {
                    for (int i = 0; i != child_pos - DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                    }
                    node_leaf_new->row_ptr[child_pos - DEGREE] = rid;
                    for (int i = child_pos - DEGREE + 1; i <= DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[i + DEGREE - 1];
                    }
                }
                node->n = DEGREE;
                node_new_root->leaf = 0;
                node_new_root->n = 1;
                node_new_root->child[0] = addr;
                node_new_root->child[1] = addr_leaf_new;
                node_new_root->next = -1;
                node_new_root->row_ptr[0] = (*insert_handler)(node_leaf_new->row_ptr[0]);
                release(pool, addr);
                release(pool, addr_leaf_new);
                NewChildEntry child_new;
                child_new.child_key = node_new_root->row_ptr[0];
                child_new.child_ptr = addr_leaf_new;
                release(pool, addr_root_new);
                ctrl = (BCtrlBlock*)get_page(pool, 0);
                ctrl->root_node = addr_root_new;
                release(pool, 0);
                return child_new;
            }
            else {
                off_t addr_leaf_new;
                BNode *node_leaf_new;
                BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                if (ctrl->free_node_head != -1) {
                    addr_leaf_new = ctrl->free_node_head;
                    node_leaf_new = (BNode*)get_page(pool, addr_leaf_new);
                    ctrl->free_node_head = node_leaf_new->next;
                }
                else {
                    addr_leaf_new = pool->file.length;
                    node_leaf_new = (BNode*)get_page(pool, pool->file.length);
                }
                release(pool, 0);
                node_leaf_new->n = DEGREE + 1;
                node_leaf_new->leaf = 1;
                node_leaf_new->next = -1;
                NewChildEntry child_new;
                int child_pos = 0;
                for (; child_pos != 2 * DEGREE; child_pos++) {
                    if((*cmp)(rid, node->row_ptr[child_pos]) < 0) {
                        break;
                    }
                }
                if (child_pos < DEGREE) {
                    child_new.child_key = (*insert_handler)(node->row_ptr[DEGREE - 1]);
                    child_new.child_ptr = addr_leaf_new;
                    for (int i = 0; i <= DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[DEGREE - 1 + i];
                    }
                    for (int i = DEGREE - 1; i > child_pos; i--) {
                        node->row_ptr[i] = node->row_ptr[i - 1];
                    }
                    node->row_ptr[child_pos] = rid;
                }
                else {
                    for (int i = 0; i != child_pos - DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[DEGREE + i];
                    }
                    node_leaf_new->row_ptr[child_pos - DEGREE] = rid;
                    for (int i = child_pos - DEGREE + 1; i <= DEGREE; i++) {
                        node_leaf_new->row_ptr[i] = node->row_ptr[i + DEGREE - 1];
                    }
                    child_new.child_key = (*insert_handler)(node_leaf_new->row_ptr[0]);
                    child_new.child_ptr = addr_leaf_new;
                }
                node->n = DEGREE;
                release(pool, addr);
                release(pool, addr_leaf_new);
                return child_new;
            }
        }
    }
}

OldChildEntry delete_entry(BufferPool *pool, off_t addr, off_t parent_addr, RID rid, b_tree_row_row_cmp_t cmp, b_tree_insert_nonleaf_handler_t insert_handler, b_tree_delete_nonleaf_handler_t delete_handler) {
    BNode *node = (BNode*)get_page(pool, addr);
    if (!node->leaf) {
        off_t addr_child;
        int prt_child = 0;
        for (; prt_child != node->n; prt_child++) {
            if ((*cmp)(rid, node->row_ptr[prt_child]) < 0) {
                addr_child = node->child[prt_child];
                break;
            }
        }
        addr_child = node->child[prt_child];
        OldChildEntry child_old;
        release(pool, addr);
        child_old = delete_entry(pool, addr_child, addr, rid, cmp, insert_handler, delete_handler);
        node = (BNode*)get_page(pool, addr);
        if (child_old == -1) {
            release(pool, addr);
            return -1;
        }
        else {
            if (parent_addr != -1) {
                if (node->n > DEGREE) {
                    BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                    if (childnode->leaf) {
                        (*delete_handler)(node->row_ptr[child_old]); 
                    }
                    release(pool, node->child[0]);
                    for (int i = child_old; i != node->n - 1; i++) {
                        node->row_ptr[i] = node->row_ptr[i + 1];
                        node->child[i + 1] = node->child[i + 2];
                    }
                    node->n--;
                    release(pool, addr);
                    return -1;
                }
                else {
                    BNode *node_par = (BNode*)get_page(pool, parent_addr);
                    int ttmp = 0;
                    for (; ttmp != node_par->n + 1; ttmp++) {
                        if (node_par->child[ttmp] == addr) {
                            break;
                        }
                    }
                    if (ttmp != node_par->n) {
                        off_t addr_bro = node_par->child[ttmp + 1];
                        BNode *brother = (BNode*)get_page(pool, addr_bro);
                        if (brother->n > DEGREE) {
                            BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                            if (childnode->leaf) {
                                (*delete_handler)(node->row_ptr[child_old]); 
                            }
                            release(pool, node->child[0]);
                            for (int i = child_old; i != node->n - 1; i++) {
                                node->row_ptr[i] = node->row_ptr[i + 1];
                                node->child[i + 1] = node->child[i + 2];
                            }
                            node->n--;
                            node->row_ptr[node->n] = node_par->row_ptr[ttmp];
                            node->child[node->n + 1] = brother->child[0];
                            node->n++;
                            node_par->row_ptr[ttmp] = brother->row_ptr[0];
                            for (int i = 1; i != brother->n; i++) {
                                brother->row_ptr[i - 1] = brother->row_ptr[i];
                                brother->child[i - 1] = brother->child[i];
                            }
                            brother->child[brother->n - 1] = brother->child[brother->n];
                            brother->n--;
                            release(pool, addr);
                            release(pool, parent_addr);
                            release(pool, addr_bro);
                            return -1;
                        }
                        else {
                            BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                            if (childnode->leaf) {
                                (*delete_handler)(node->row_ptr[child_old]); 
                            }
                            release(pool, node->child[0]);
                            for (int i = child_old; i != node->n - 1; i++) {
                                node->row_ptr[i] = node->row_ptr[i + 1];
                                node->child[i + 1] = node->child[i + 2];
                            }
                            node->n--;
                            node->row_ptr[node->n] = node_par->row_ptr[ttmp];
                            node->child[node->n + 1] = brother->child[0];
                            node->n++;
                            release(pool, parent_addr);
                            for (int i = 0; i != brother->n; i++) {
                                node->row_ptr[node->n + i] = brother->row_ptr[i];
                                node->child[node->n + i + 1] = brother->child[i + 1];
                            }
                            node->n += brother->n;
                            release(pool, addr);
                            BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                            brother->n = 0;
                            brother->next = ctrl->free_node_head;
                            ctrl->free_node_head = addr_bro;
                            release(pool, 0);
                            release(pool, addr_bro);
                            return ttmp; 
                        }
                    }
                    else {
                        off_t addr_bro = node_par->child[ttmp - 1];
                        BNode *brother = (BNode*)get_page(pool, addr_bro);
                        if (brother->n > DEGREE) {
                            BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                            if (childnode->leaf) {
                                (*delete_handler)(node->row_ptr[child_old]); 
                            }
                            release(pool, node->child[0]);
                            for (int i = child_old; i != node->n - 1; i++) {
                                node->row_ptr[i] = node->row_ptr[i + 1];
                                node->child[i + 1] = node->child[i + 2];
                            }
                            node->n--;
                            for (int i = node->n; i > 0; i--) {
                                node->row_ptr[i] = node->row_ptr[i - 1];
                                node->child[i + 1] = node->child[i];
                            }
                            node->child[1] = node->child[0];
                            node->n++;
                            node->row_ptr[0] = node_par->row_ptr[ttmp - 1];
                            node->child[0] = brother->child[brother->n];
                            node_par->row_ptr[ttmp - 1] = brother->row_ptr[brother->n - 1];
                            brother->n--;
                            release(pool, addr);
                            release(pool, parent_addr);
                            release(pool, addr_bro);
                            return -1;
                        }
                        else {
                            BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                            if (childnode->leaf) {
                                (*delete_handler)(node->row_ptr[child_old]); 
                            }
                            release(pool, node->child[0]);
                            for (int i = child_old; i != node->n - 1; i++) {
                                node->row_ptr[i] = node->row_ptr[i + 1];
                                node->child[i + 1] = node->child[i + 2];
                            }
                            node->n--;
                            brother->row_ptr[brother->n] = node_par->row_ptr[ttmp - 1];
                            brother->child[brother->n + 1] = node->child[0];
                            release(pool, parent_addr);
                            brother->n++;
                            for (int i = 0; i != node->n; i++) {
                                brother->row_ptr[brother->n + i] = node->row_ptr[i];
                                brother->child[brother->n + i + 1] = node->child[i + 1];
                            }
                            brother->n += node->n;
                            release(pool, addr_bro);
                            BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                            node->n = 0;
                            node->next = ctrl->free_node_head;
                            ctrl->free_node_head = addr;
                            release(pool, 0);
                            release(pool, addr);
                            return ttmp - 1;
                        }
                    }
                }
            }
            else {
                BNode *childnode = (BNode*)get_page(pool, node->child[0]);
                if (childnode->leaf) {
                    (*delete_handler)(node->row_ptr[child_old]); 
                }
                release(pool, node->child[0]);
                for (int i = child_old; i != node->n - 1; i++) {
                    node->row_ptr[i] = node->row_ptr[i + 1];
                    node->child[i + 1] = node->child[i + 2];
                }
                node->n--;
                if (node->n == 0) {
                    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                    node->next = ctrl->free_node_head;
                    ctrl->free_node_head = addr;
                    ctrl->root_node = node->child[0];
                    release(pool, 0);
                }
                release(pool, addr);
                return -1;
            }
        }
    }
    else {
        if (parent_addr != -1) {
            if (node->n > DEGREE) {
                int prt_final = 0;
                for(; prt_final != node->n; prt_final++) {
                    if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                        break;
                    }
                }
                for (int i = prt_final; i != node->n - 1; i++) {
                    node->row_ptr[i] = node->row_ptr[i + 1];
                }
                node->n--;
                release(pool, addr);
                return -1;
            }
            else {
                BNode *node_par = (BNode*)get_page(pool, parent_addr);
                int ttmp = 0;
                for (; ttmp != node_par->n + 1; ttmp++) {
                    if (node_par->child[ttmp] == addr) {
                        break;
                    }
                }
                if (ttmp != node_par->n) {
                    off_t addr_bro = node_par->child[ttmp + 1];
                    BNode *brother = (BNode*)get_page(pool, addr_bro);
                    if (brother->n > DEGREE) {
                        int prt_final = 0;
                        for(; prt_final != node->n; prt_final++) {
                            if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                                break;
                            }
                        }
                        for (int i = prt_final; i != node->n - 1; i++) {
                            node->row_ptr[i] = node->row_ptr[i + 1];
                        }
                        node->n--;
                        node->row_ptr[node->n] = brother->row_ptr[0];
                        for (int i = 1; i != brother->n; i++) {
                            brother->row_ptr[i - 1] = brother->row_ptr[i];
                        }
                        node->n++;
                        brother->n--;
                        (*delete_handler)(node_par->row_ptr[ttmp]);
                        node_par->row_ptr[ttmp] = (*insert_handler)(brother->row_ptr[0]);
                        release(pool, addr);
                        release(pool, parent_addr);
                        release(pool, addr_bro);
                        return -1;
                    }
                    else {
                        release(pool, parent_addr);
                        int prt_final = 0;
                        for(; prt_final != node->n; prt_final++) {
                            if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                                break;
                            }
                        }
                        for (int i = prt_final; i != node->n - 1; i++) {
                            node->row_ptr[i] = node->row_ptr[i + 1];
                        }
                        node->n--;
                        for (int i = 0; i != brother->n; i++) {
                            node->row_ptr[node->n + i] = brother->row_ptr[i];
                        }
                        node->n += brother->n;
                        release(pool, addr);
                        BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                        brother->n = 0;
                        brother->next = ctrl->free_node_head;
                        ctrl->free_node_head = addr_bro;
                        release(pool, 0);
                        release(pool, addr_bro);
                        return ttmp; 
                    }
                }
                else {
                    off_t addr_bro = node_par->child[ttmp - 1];
                    BNode *brother = (BNode*)get_page(pool, addr_bro);
                    if (brother->n > DEGREE) {
                        int prt_final = 0;
                        for(; prt_final != node->n; prt_final++) {
                            if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                                break;
                            }
                        }
                        for (int i = prt_final; i > 0; i--) {
                            node->row_ptr[i] = node->row_ptr[i - 1];
                        }
                        node->n--;
                        node->row_ptr[0] = brother->row_ptr[brother->n - 1];
                        node->n++;
                        brother->n--;
                        (*delete_handler)(node_par->row_ptr[ttmp - 1]);
                        node_par->row_ptr[ttmp - 1] = (*insert_handler)(node->row_ptr[0]);
                        release(pool, addr);
                        release(pool, parent_addr);
                        release(pool, addr_bro);
                        return -1;
                    }
                    else {
                        release(pool, parent_addr);
                        int prt_final = 0;
                        for(; prt_final != node->n; prt_final++) {
                            if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                                break;
                            }
                        }
                        for (int i = prt_final; i != node->n - 1; i++) {
                            node->row_ptr[i] = node->row_ptr[i + 1];
                        }
                        node->n--;
                        for (int i = 0; i != node->n; i++) {
                            brother->row_ptr[brother->n + i] = node->row_ptr[i];
                        }
                        brother->n += node->n;
                        release(pool, addr_bro);
                        BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                        node->n = 0;
                        node->next = ctrl->free_node_head;
                        ctrl->free_node_head = addr;
                        release(pool, 0);
                        release(pool, addr);
                        return ttmp - 1; 
                    }
                }
            }
        }
        else {
            int prt_final = 0;
            for(; prt_final != node->n; prt_final++) {
                if((*cmp)(rid, node->row_ptr[prt_final]) == 0) {
                    break;
                }
            }
            for (int i = prt_final; i != node->n - 1; i++) {
                node->row_ptr[i] = node->row_ptr[i + 1];
            }
            node->n--;
            if (node->n == 0) {
                BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
                node->next = ctrl->free_node_head;
                ctrl->free_node_head = addr;
                ctrl->root_node = -1;
                release(pool, 0);
            }
            release(pool, addr);
            return -1;
        }
    }
}

void b_tree_init(const char *filename, BufferPool *pool) {
    init_buffer_pool(filename, pool);
    if (pool->file.length != 0) {
        return;
    }
    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
    ctrl->free_node_head = -1;
    ctrl->root_node = -1;
    release(pool, 0);
}

void b_tree_close(BufferPool *pool) {
    close_buffer_pool(pool);
}

RID search_entry(BufferPool *pool, off_t addr, void *key, size_t size, b_tree_ptr_row_cmp_t cmp){
    BNode *node = (BNode*)get_page(pool, addr);
    if (node->leaf) {
        /* this node is a leaf node */
        int counter = 0;
        for (; counter < node->n; counter++) {
            if((*cmp)(key, size, node->row_ptr[counter]) == 0) {
                break;
            }
        }
        if(counter < node->n){
            RID result = node->row_ptr[counter];
            release(pool, addr);
            return result;
        }
        else{
            RID failed_rid;
            get_rid_block_addr(failed_rid) = -1;
            get_rid_idx(failed_rid) = 0;
            release(pool, addr);
            return failed_rid;
        }
    }
    else {
        /* a non-leaf node */
        off_t addr_child;
        int prt_child = 0;
        for (; prt_child != node->n; prt_child++) {
            if ((*cmp)(key, size, node->row_ptr[prt_child]) < 0) {
                addr_child = node->child[prt_child];
                break;
            }
        }
        addr_child = node->child[prt_child];
        release(pool, addr);
        return search_entry(pool, addr_child, key, size, cmp);
    }
}

RID b_tree_search(BufferPool *pool, void *key, size_t size, b_tree_ptr_row_cmp_t cmp) {
    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
    off_t root_node_addr = ctrl->root_node;
    release(pool, 0);
    BNode *root;
    if (root_node_addr == -1) {
        /* create root node */
        RID failed_rid;
        get_rid_block_addr(failed_rid) = -1;
        get_rid_idx(failed_rid) = 0;
        return failed_rid;
    }
    else {
        // root = (BNode*)get_page(pool, root_node_addr);
        // release(&pool, root_node_addr);
        return search_entry(pool, root_node_addr, key, size, cmp);
    }
}

RID b_tree_insert(BufferPool *pool, RID rid, b_tree_row_row_cmp_t cmp, b_tree_insert_nonleaf_handler_t insert_handler) {
    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
    off_t root_node_addr = ctrl->root_node;
    BNode *root;
    if (root_node_addr != -1) 
        root = (BNode*)get_page(pool, root_node_addr);
    else {
        if (ctrl->free_node_head == -1) 
            root = (BNode*)get_page(pool, PAGE_SIZE);
        else {
            root = (BNode*)get_page(pool, ctrl->free_node_head);
            ctrl->free_node_head = root->next;
        }
        root->n = 0;
        root->next = -1;
        root->leaf = 1;
        ctrl->root_node = PAGE_SIZE;
    }
    root_node_addr = ctrl->root_node;
    release(pool, 0);
    release(pool, root_node_addr);
    insert_entry(pool, root_node_addr, rid, cmp, root_node_addr, insert_handler);
    return rid;
}

void b_tree_delete(BufferPool *pool, RID rid, b_tree_row_row_cmp_t cmp, b_tree_insert_nonleaf_handler_t insert_handler, b_tree_delete_nonleaf_handler_t delete_handler) {
    BCtrlBlock *ctrl = (BCtrlBlock*)get_page(pool, 0);
    off_t root_node_addr = ctrl->root_node;
    if (root_node_addr == -1)
        return;
    release(pool, 0);
    delete_entry(pool, root_node_addr, -1, rid, cmp, insert_handler, delete_handler);
}