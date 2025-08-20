#include <stdio.h>
#include "argc_error.h"

int arg_count_error(int argc) {
    int res = 0;
    if (argc < 3) {
        printf("Error of arg count, should be more than 2");
        res = 1;
    }
    return res;
}