#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

bool is_empty_line(const char *line);
void print_line(char *line, bool show_end, bool number, int line_number);
void process_file(const char *filename, bool number_lines, bool show_end, bool number_non_empty);
void process_stdin(bool number_lines, bool show_end, bool number_non_empty);
void parse_arguments(int argc, char *argv[], bool *number_lines, bool *show_end, bool *number_non_empty, int *file_start_index);

int main(int argc, char *argv[]) {
    bool number_lines = false;
    bool show_end = false;
    bool number_non_empty = false;
    int file_start_index = 0;

    parse_arguments(argc, argv, &number_lines, &show_end, &number_non_empty, &file_start_index);

    if (file_start_index >= argc) {
        process_stdin(number_lines, show_end, number_non_empty);
    } else {
        for (int i = file_start_index; i < argc; i++) {
            if (strcmp(argv[i], "-") == 0) {
                process_stdin(number_lines, show_end, number_non_empty);
            } else {
                process_file(argv[i], number_lines, show_end, number_non_empty);
            }
        }
    }

    return 0;
}

bool is_empty_line(const char *line) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (!isspace((unsigned char)line[i])) {
            return false;
        }
    }
    return true;
}

void print_line(char *line, bool show_end, bool number, int line_number) {
    if (number) {
        printf("%6d\t", line_number);
    }
    
    if (show_end) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        printf("%s$\n", line);
    } else {
        printf("%s", line);
    }
}


void process_file(const char *filename, bool number_lines, bool show_end, bool number_non_empty) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "cat: %s: ", filename);
        perror("");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) == sizeof(line) - 1 && line[sizeof(line) - 2] != '\n') {
            fprintf(stderr, "Warning: Line truncated in file %s\n", filename);
        }

        if (number_non_empty) {
            if (is_empty_line(line)) {
                print_line(line, show_end, false, 0);
            } else {
                line_number++;
                print_line(line, show_end, true, line_number);
            }
        } else if (number_lines) {
            line_number++;
            print_line(line, show_end, true, line_number);
        } else {
            print_line(line, show_end, false, 0);
        }
    }
    
    if (ferror(file)) {
        fprintf(stderr, "Error reading file: %s\n", filename);
    }
    
    fclose(file);
}


void process_stdin(bool number_lines, bool show_end, bool number_non_empty) {
    char line[MAX_LINE_LENGTH];
    int line_number = 0;

    while (fgets(line, sizeof(line), stdin)) {
        if (number_non_empty) {
            if (is_empty_line(line)) {
                print_line(line, show_end, false, 0);
            } else {
                line_number++;
                print_line(line, show_end, true, line_number);
            }
        } else if (number_lines) {
            line_number++;
            print_line(line, show_end, true, line_number);
        } else {
            print_line(line, show_end, false, 0);
        }
    }
}


void parse_arguments(int argc, char *argv[], bool *number_lines, bool *show_end, bool *number_non_empty, int *file_start_index) {
    *file_start_index = 1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'n':
                        *number_lines = true;
                        *number_non_empty = false;
                        break;
                    case 'E':
                        *show_end = true;
                        break;
                    case 'b':
                        *number_non_empty = true;
                        *number_lines = false;
                        break;
                    default:
                        fprintf(stderr, "cat: invalid option -- '%c'\n", argv[i][j]);
                        print_usage(argv[0]);
                        exit(1);
                }
            }
            (*file_start_index)++;
        } else {
            break;
        }
    }
}