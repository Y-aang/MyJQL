#include "myjql.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
typedef long long my_off_t;
#else
typedef long my_off_t;
#endif

#define MAXSIZE 100000

int test(int num_op, int out)
{
    int flag = 0;
    int i, op;
    myjql_init();

    for (i = 0; i < num_op; ++i) {
        op = rand() % 3;
        if (op == 0) {  /* insert */ //-----set
            char a[MAXSIZE], b[MAXSIZE];
            for(int k = 0; k < MAXSIZE; k++){
                char tmp1 = 'a' + rand() % 25;
                char tmp2 = 'a' + rand() % 25;
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, MAXSIZE, b, MAXSIZE);
        } else if (op == 1 && get_total() != 0) {  /* erase */ //-----delete
            char a[MAXSIZE], b[MAXSIZE];
            for(int k = 0; k < MAXSIZE; k++){
                char tmp1 = 'a' + rand() % 25;
                char tmp2 = 'a' + rand() % 25;
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_del(a, MAXSIZE);
        } else {  /* find */ //-----get
            char a[MAXSIZE], b[MAXSIZE];
            for(int k = 0; k < MAXSIZE; k++){
                char tmp1 = 'a' + rand() % 25;
                char tmp2 = 'a' + rand() % 25;
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, MAXSIZE, b, MAXSIZE);
        }
    }

    myjql_close();
    return flag;
}

int test_rep(int num_op, int out)
{
    int flag = 0;
    int i, op;
    myjql_init();

    for (i = 0; i < num_op; ++i) {
        // op = 0;
        op = rand() % 3;
        int len = MAXSIZE + rand() % 1000;
        if (op == 0) {  /* insert */ //-----set
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, len, b, len);
        } else if (op == 1 && get_total() != 0) {  /* erase */ //-----delete
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_del(a, len);
        } else {  /* find */ //-----get
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, len, b, len);
        }
    }

    for (i = 0; i < num_op; ++i) {
        // op = 0;
        op = rand() % 3;
        int len = MAXSIZE + rand() % 2;
        if (op == 0) {  /* insert */ //-----set
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, len, b, len);
        } else if (op == 1 && get_total() != 0) {  /* erase */ //-----delete
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_del(a, len);
        } else {  /* find */ //-----get
            char a[MAXSIZE + 1000], b[MAXSIZE + 1000];
            for(int k = 0; k < len; k++){
                char tmp1 = 'a';
                char tmp2 = 'a';
                a[k] = tmp1;
                b[k] = tmp2;
            }
            myjql_get(a, len, b, len);
        }
    }

    myjql_close();
    return flag;
}

int main()
{
    
    srand(0);


    if (test(200, 0)) {
        return 1;
    }
    if (test_rep(20000, 0)) {
        return 1;
    }

    return 0;
}