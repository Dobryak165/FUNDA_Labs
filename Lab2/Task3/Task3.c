#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef enum {
    OK = 0,
    ERROR_ARGC,
    ERROR_MEMORY,
    ERROR_FILE_OPEN,
    ERROR_INVALID_INPUT
} ErrorStatus;

typedef struct {
    int file_index; 
    int line_num;    
    int col_num;   
} MatchResult;

typedef struct {
    MatchResult *data;
    int count;
    int capacity;
} MatchList;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_ARGC:          
            printf("Неверное количество аргументов\n"); 
            break;
        case ERROR_MEMORY:        
            printf("Ошибка выделения памяти\n"); 
            break;
        case ERROR_FILE_OPEN:     
            printf("Не удалось открыть файл\n"); 
            break;
        case ERROR_INVALID_INPUT: 
            printf("Неверные входные данные\n"); 
            break;
        default:                  
                printf("Неизвестная ошибка\n"); 
                break;
    }
}

int my_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

ErrorStatus init_match_list(MatchList *list) {
    if (list == NULL) return ERROR_INVALID_INPUT;
    list->count = 0;
    list->capacity = 16;
    list->data = (MatchResult *)malloc(list->capacity * sizeof(MatchResult));
    if (list->data == NULL) return ERROR_MEMORY;
    return OK;
}

ErrorStatus add_match(MatchList *list, int file_idx, int line, int col) {
    if (list->count == list->capacity) {
        int new_cap = list->capacity * 2;
        MatchResult *new_data = (MatchResult *)realloc(list->data, new_cap * sizeof(MatchResult));
        if (new_data == NULL) return ERROR_MEMORY;
        list->data = new_data;
        list->capacity = new_cap;
    }
    list->data[list->count].file_index = file_idx;
    list->data[list->count].line_num = line;
    list->data[list->count].col_num = col;
    list->count++;
    return OK;
}

void free_match_list(MatchList *list) {
    if (list != NULL && list->data != NULL) {
        free(list->data);
        list->data = NULL;
    }
}

ErrorStatus read_file_to_string(const char *filename, char **out_str, long *out_len) {
    FILE *fin = fopen(filename, "rb");
    if (fin == NULL) return ERROR_FILE_OPEN;

    fseek(fin, 0, SEEK_END);
    long length = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if (length < 0) {
        fclose(fin);
        return ERROR_FILE_OPEN;
    }

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        fclose(fin);
        return ERROR_MEMORY;
    }

    size_t read = fread(buffer, 1, length, fin);
    buffer[read] = '\0';
    
    fclose(fin);
    *out_str = buffer;
    *out_len = (long)read;
    return OK;
}

ErrorStatus find_in_files_core(const char *substr, int file_count, MatchList *list, const char **filenames_array) {
    if (substr == NULL || list == NULL || file_count <= 0 || filenames_array == NULL) {
        return ERROR_INVALID_INPUT;
    }
    
    int sub_len = my_strlen(substr);
    if (sub_len == 0) return ERROR_INVALID_INPUT;

    for (int i = 0; i < file_count; i++) {
        const char *filename = filenames_array[i];
        char *text = NULL;
        long text_len = 0;

        ErrorStatus status = read_file_to_string(filename, &text, &text_len);
        if (status != OK) {
            return status;
        }

        int current_line = 1;
        int current_col = 1;

        for (long pos = 0; pos <= text_len - sub_len; pos++) {
            int match = 1;
            for (int j = 0; j < sub_len; j++) {
                if (text[pos + j] != substr[j]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                status = add_match(list, i, current_line, current_col);
                if (status != OK) {
                    free(text);
                    return status;
                }
            }

            if (text[pos] == '\n') {
                current_line++;
                current_col = 1;
            } else {
                current_col++;
            }
        }
        free(text);
    }

    return OK;
}

ErrorStatus find_in_files(const char *substr, int file_count, MatchList *list, ...) {
    if (substr == NULL || list == NULL || file_count <= 0) {
        return ERROR_INVALID_INPUT;
    }

    const char **temp_files = (const char **)malloc(file_count * sizeof(char *));
    if (temp_files == NULL) return ERROR_MEMORY;

    va_list args;
    va_start(args, list);
    for (int i = 0; i < file_count; i++) {
        temp_files[i] = va_arg(args, const char *);
    }
    va_end(args);

    ErrorStatus status = find_in_files_core(substr, file_count, list, temp_files);
    
    free(temp_files);
    return status;
}

void print_results(const MatchList *list, const char **filenames_array) {
    for (int i = 0; i < list->count; i++) {
        MatchResult res = list->data[i];
        const char *filename = filenames_array[res.file_index];
        printf("Файл: %s | Строка: %d | Символ: %d\n", filename, res.line_num, res.col_num);
    }
}

int main(int argc, char *argv[]) {
    MatchList demo_list;
    if (init_match_list(&demo_list) == OK) {
        ErrorStatus status_demo = find_in_files("test", 2, &demo_list, "Text1.txt", "Text4.txt");
        
        if (status_demo == OK) {
            const char *demo_names[] = {"Text1.txt", "Text4.txt"};
            printf("Вхождений найдено: %d\n", demo_list.count);
            print_results(&demo_list, demo_names);
        } else {
            print_error(status_demo);
        }
        free_match_list(&demo_list);
    }
    
    if (argc < 3) { 
        return 1;
    }

    const char *substr = argv[1];
    int file_count = argc - 2;
    const char **filenames = (const char **)&argv[2];

    MatchList list;
    ErrorStatus status = init_match_list(&list);
    if (status != OK) {
        print_error(status);
        return 1;
    }

    status = find_in_files_core(substr, file_count, &list, filenames);

    if (status != OK) {
        print_error(status);
        free_match_list(&list);
        return 1;
    }

    printf("Найдено вхождений: %d\n", list.count);
    print_results(&list, filenames);

    free_match_list(&list);
    return 0;
}