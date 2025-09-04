#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>

#define MAX_PATTERNS 10
#define MAX_LINE_LENGTH 1024

struct Options {
    bool flag_e;
    bool flag_i;
    bool flag_v;
    bool flag_c;
    bool flag_l;
    bool flag_n;
    int file_start_index;
    char *patterns[MAX_PATTERNS];
    int pattern_count;
};

void parse_arguments(int argc, char *argv[], struct Options *opt);
void init_options(struct Options *options);
void process_file(const char *filename, struct Options *opt, bool multiple_files);
int compile_patterns(regex_t *regexes, struct Options *opt);
int match_line(const char *line, regex_t *regexes, struct Options *opt);
void print_match(const char *filename, const char *line, int line_num, int match_count, struct Options *opt, bool multiple_files);
void free_patterns(regex_t *regexes, int regexes_count);


int main(int argc, char *argv[]) {
    struct Options options;
    struct Options *opt = &options;
    init_options(opt);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [options] pattern [file...]\n", argv[0]);
        exit(1);
    }

    parse_arguments(argc, argv, opt);

    if (opt->pattern_count == 0 && opt->file_start_index < argc) {
        opt->patterns[opt->pattern_count] = argv[opt->file_start_index];
        opt->pattern_count++;
        opt->file_start_index++;
    }

    if (opt->pattern_count == 0) {
        fprintf(stderr, "No pattern specified\n");
        exit(1);
    }

    if (opt->file_start_index >= argc) {
        process_file("-", opt, false);
    } else {
        bool multiple_files = (argc - opt->file_start_index) > 1;
        for (int i = opt->file_start_index; i < argc; i++) {
            process_file(argv[i], opt, multiple_files);
        }
    }

    return 0;
}

void parse_arguments(int argc, char *argv[], struct Options *opt) {
    opt->file_start_index = 1;
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "--") == 0) {
                opt->file_start_index = i + 1;
                break;
            }
            
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'e':
                        opt->flag_e = true;
                        if (i + 1 < argc) {
                            opt->patterns[opt->pattern_count] = argv[i + 1];
                            opt->pattern_count++;
                            i++;
                        } else {
                            fprintf(stderr, "grep: option requires an argument -- 'e'\n");
                            exit(1);
                        }
                        j = strlen(argv[i]) - 1;
                        break;
                    case 'i':
                        opt->flag_i = true;
                        break;
                    case 'v':
                        opt->flag_v = true;
                        break;
                    case 'c':
                        opt->flag_c = true;
                        break;
                    case 'l':
                        opt->flag_l = true;
                        break;
                    case 'n':
                        opt->flag_n = true;
                        break;
                    default:
                        fprintf(stderr, "grep: invalid option -- '%c'\n", argv[i][j]);
                        exit(1);
                }
            }
        } else {
            opt->file_start_index = i;
            break;
        }
    }
}

void init_options(struct Options *options) {
    options->flag_e = false;
    options->flag_i = false;
    options->flag_v = false;
    options->flag_c = false;
    options->flag_l = false;
    options->flag_n = false;
    options->file_start_index = 1;
    options->pattern_count = 0;

    for (int i = 0; i < MAX_PATTERNS; i++) {
        options->patterns[i] = NULL;
    }
}

void process_file(const char *filename, struct Options *opt, bool multiple_files) {
    FILE *file;
    if (strcmp(filename, "-") == 0) {
        file = stdin;
        filename = "(standard input)";
    } else {
        file = fopen(filename, "r");
        if (file == NULL) {
            perror("grep");
            return;
        }
    }

    regex_t regexes[MAX_PATTERNS];
    if (compile_patterns(regexes, opt) != 0) {
        if (file != stdin) fclose(file);
        return;
    }

    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    int match_count = 0;
    bool file_printed = false;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        int is_match = match_line(line, regexes, opt);

        if (is_match) {
            match_count++;

            if (opt->flag_l) {
                if (!file_printed) {
                    printf("%s\n", filename);
                    file_printed = true;
                }
                break;
            }
            
            if (!opt->flag_c) {
                print_match(filename, line, line_num, match_count, opt, multiple_files);
            }
        }
    }

    if (opt->flag_c) {
        if (opt->flag_l) {
            if (match_count > 0) {
                printf("%s\n", filename);
            }
        } else {
            if (multiple_files) {
                printf("%s:%d\n", filename, match_count);
            } else {
                printf("%d\n", match_count);
            }
        }
    }

    free_patterns(regexes, opt->pattern_count);

    if (file != stdin) {
        fclose(file);
    }
}

int compile_patterns(regex_t *regexes, struct Options *opt) {
    int flags = REG_EXTENDED;
    if (opt->flag_i) {
        flags |= REG_ICASE;
    }

    for (int i = 0; i < opt->pattern_count; i++) {
        int reg_c = regcomp(&regexes[i], opt->patterns[i], flags);
        if (reg_c != 0) {
            char error_buf[100];
            regerror(reg_c, &regexes[i], error_buf, sizeof(error_buf));
            fprintf(stderr, "Error: Could not compile regex '%s': %s\n", opt->patterns[i], error_buf);
            for (int j = 0; j < i; j++) {
                regfree(&regexes[j]);
            }
            return -1;
        }
    }
    return 0;
}

int match_line(const char *line, regex_t *regexes, struct Options *opt) {
    int match_found = 0;

    for (int i = 0; i < opt->pattern_count; i++) {
        int res = regexec(&regexes[i], line, 0, NULL, 0);
        if (res == 0) {
            match_found = 1;
            break;
        } else if (res != REG_NOMATCH) {
            char error_buf[100];
            regerror(res, &regexes[i], error_buf, sizeof(error_buf));
            fprintf(stderr, "Regex match error: %s\n", error_buf);
        }
    }

    if (opt->flag_v) {
        match_found = !match_found;
    }

    return match_found;
}

void free_patterns(regex_t *regexes, int regexes_count) {
    for (int i = 0; i < regexes_count; i++) {
        regfree(&regexes[i]);
    }
}

void print_match(const char *filename, const char *line, int line_num, 
                 int match_count, struct Options *opt, bool multiple_files) {
    
    if (multiple_files) {
        printf("%s:", filename);
    }
    
    if (opt->flag_n) {
        printf("%d:", line_num);
    }
    
    printf("%s", line);
    
    // Добавляем перенос строки если его нет
    if (line[strlen(line) - 1] != '\n') {
        printf("\n");
    }
}