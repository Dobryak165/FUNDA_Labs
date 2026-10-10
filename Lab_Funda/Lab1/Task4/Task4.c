#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_PATH 512

typedef enum {
    OK = 0,
    ERROR_ARGC,
    ERROR_FLAG,
    ERROR_FILE_OPEN,
    ERROR_FILE_WRITE
} ErrorStatus;

ErrorStatus validate_args(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        return ERROR_ARGC;
    }

    const char *flag_str = argv[1];
    if (strlen(flag_str) < 2 || (flag_str[0] != '-' && flag_str[0] != '/')) {
        return ERROR_FLAG;
    }

    int has_n = (flag_str[1] == 'n');
    char action;

    if (has_n) {
        if (strlen(flag_str) != 3) return ERROR_FLAG;
        action = flag_str[2];
    } else {
        if (strlen(flag_str) != 2) return ERROR_FLAG;
        action = flag_str[1];
    }

    if (action != 'd' && action != 'i' && action != 's' && action != 'a') {
        return ERROR_FLAG;
    }

    if (has_n && argc != 4) return ERROR_ARGC;
    if (!has_n && argc != 3) return ERROR_ARGC;

    return OK;
}

void print_errors(ErrorStatus error) {
    switch (error) {
        case ERROR_ARGC:       
            printf("Неправильно кол-во аргументов\n"); 
            break;
        case ERROR_FLAG:       
            printf("Недопустимый флаг\n"); 
            break;
        case ERROR_FILE_OPEN:  
            printf("Не удалось открыть файл\n"); 
            break;
        case ERROR_FILE_WRITE: 
            printf("Ошибка записи в файл\n"); 
            break;
        default:               
            printf("Ну тут капец, что-то другое\n");
    }
}

ErrorStatus process_file(const char *in_path, const char *out_path, char action) {
    FILE *filein = fopen(in_path, "r");
    if (filein == NULL) {
        return ERROR_FILE_OPEN;
    }

    FILE *fileout = fopen(out_path, "w");
    if (fileout == NULL) {
        fclose(filein);
        return ERROR_FILE_OPEN;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), filein) != NULL) {
        int count = 0;

        for (int i = 0; line[i] != '\0'; i++) {
            char c = line[i];

            if (action == 'd') {
                if (isdigit((unsigned char)c) == 0) {
                    if (fputc(c, fileout) == EOF) {
                        fclose(filein);
                        fclose(fileout);
                        return ERROR_FILE_WRITE;
                    }
                }
            }
            else if (action == 'i') {
                if (isalpha((unsigned char)c) != 0) {
                    count++;
                }
            }
            else if (action == 's') {
                if (isalnum((unsigned char)c) == 0 && isspace((unsigned char)c) == 0) {
                    count++;
                }
            }
            else if (action == 'a') {
                if (isdigit((unsigned char)c) == 0) {
                    if (fprintf(fileout, "%x", (unsigned char)c) < 0) {
                        fclose(filein);
                        fclose(fileout);
                        return ERROR_FILE_WRITE;
                    }
                } else {
                    if (fputc(c, fileout) == EOF) {
                        fclose(filein);
                        fclose(fileout);
                        return ERROR_FILE_WRITE;
                    }
                }
            }
        }

        if (action == 'i' || action == 's') {
            if (fprintf(fileout, "%d\n", count) < 0) {
                fclose(filein);
                fclose(fileout);
                return ERROR_FILE_WRITE;
            }
        }
    }

    fclose(filein);
    fclose(fileout);
    return OK;
}

void make_out_path(const char *in_path, char *out_path, size_t size) {
    snprintf(out_path, size, "out_%s", in_path);
}

int main(int argc, char *argv[]) {
    ErrorStatus status = validate_args(argc, argv);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    const char *in_path = argv[2];
    char out_path[MAX_PATH];
    int has_n = (argv[1][1] == 'n');
    char action = has_n ? argv[1][2] : argv[1][1];

    if (has_n) {
        snprintf(out_path, sizeof(out_path), "%s", argv[3]);
    } else {
        make_out_path(in_path, out_path, sizeof(out_path));
    }

    status = process_file(in_path, out_path, action);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    return 0;
}