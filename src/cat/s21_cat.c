#include <stdio.h>
#include "../common/argc_error.h"

int main(int argc, char *argv[]) {
    int stop = 0;
    if (arg_count_error(argc) == 1) stop = 1;
    return 0;
}