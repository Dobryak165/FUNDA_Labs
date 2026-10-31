#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_INVALID_INPUT,
    ERROR_MEMORY,
    ERROR_FORMAT
} ErrorStatus;

typedef struct {
    char *name;
    unsigned long long size;
    unsigned long long importance;
} FileInfo;

typedef struct {
    FileInfo *files;
    int count;
    unsigned long long total_size;
    unsigned long long total_importance;
} BackupResult;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_INVALID_INPUT: 
            printf("Неверные входные данные\n"); 
            break;
        case ERROR_MEMORY:        
            printf("Ошибка выделения памяти\n"); 
            break;
        case ERROR_FORMAT:        
            printf("Неверный формат данных\n"); 
            break;
        default:                  
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

ErrorStatus parse_ull(const char *str, unsigned long long *result) {
    if (str == NULL || result == NULL || str[0] == '\0') return ERROR_INVALID_INPUT;
    unsigned long long value = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return ERROR_INVALID_INPUT;
        unsigned int digit = str[i] - '0';
        if (value > (ULLONG_MAX - digit) / 10) return ERROR_INVALID_INPUT;
        value = value * 10 + digit;
    }
    *result = value;
    return OK;
}

void free_backup_result(BackupResult *res) {
    if (res != NULL && res->files != NULL) {
        free(res->files);
        res->files = NULL;
    }
}

ErrorStatus select_files_for_backup(FileInfo *files, int n, unsigned long long capacity, BackupResult *result) {
    if (files == NULL || result == NULL) return ERROR_INVALID_INPUT;
    if (n < 0) return ERROR_INVALID_INPUT;

    if (capacity > 100000000 || (unsigned long long)(n + 1) * (capacity + 1) > 100000000) {
        return ERROR_MEMORY; 
    }

    unsigned long long **dp = (unsigned long long **)malloc((n + 1) * sizeof(unsigned long long *));
    if (dp == NULL) return ERROR_MEMORY;

    for (int i = 0; i <= n; i++) {
        dp[i] = (unsigned long long *)calloc(capacity + 1, sizeof(unsigned long long));
        if (dp[i] == NULL) {
            for (int k = 0; k < i; k++) free(dp[k]);
            free(dp);
            return ERROR_MEMORY;
        }
    }

    for (int i = 1; i <= n; i++) {
        for (unsigned long long w = 0; w <= capacity; w++) {
            if (files[i - 1].size <= w) {
                unsigned long long take = dp[i - 1][w - files[i - 1].size] + files[i - 1].importance;
                unsigned long long not_take = dp[i - 1][w];
                dp[i][w] = (take > not_take) ? take : not_take;
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    result->files = (FileInfo *)malloc(n * sizeof(FileInfo));
    if (result->files == NULL) {
        for (int i = 0; i <= n; i++) free(dp[i]);
        free(dp);
        return ERROR_MEMORY;
    }
    
    result->count = 0;
    result->total_size = 0;
    result->total_importance = dp[n][capacity];

    unsigned long long current_capacity = capacity;
    for (int i = n; i > 0; i--) {
        if (dp[i][current_capacity] != dp[i - 1][current_capacity]) {
            result->files[result->count] = files[i - 1];
            result->total_size += files[i - 1].size;
            result->count++;
            current_capacity -= files[i - 1].size;
        }
    }

    for (int i = 0; i < result->count / 2; i++) {
        FileInfo temp = result->files[i];
        result->files[i] = result->files[result->count - 1 - i];
        result->files[result->count - 1 - i] = temp;
    }

    for (int i = 0; i <= n; i++) free(dp[i]);
    free(dp);

    return OK;
}

ErrorStatus parse_file_line(const char *line, FileInfo *file) {
    char name_buf[256];
    char size_buf[64];
    char imp_buf[64];

    int parsed = sscanf(line, "%255s %63s %63s", name_buf, size_buf, imp_buf);
    if (parsed != 3) return ERROR_FORMAT;

    if (parse_ull(size_buf, &file->size) != OK || file->size == 0) return ERROR_FORMAT;
    if (parse_ull(imp_buf, &file->importance) != OK) return ERROR_FORMAT;

    file->name = (char *)malloc(strlen(name_buf) + 1);
    if (file->name == NULL) return ERROR_MEMORY;
    strcpy(file->name, name_buf);

    return OK;
}

int main(void) {
    char line[1024];
    
    if (fgets(line, sizeof(line), stdin) == NULL) {
        print_error(ERROR_FORMAT);
        return 1;
    }

    char n_str[64], cap_str[64];
    if (sscanf(line, "%63s %63s", n_str, cap_str) != 2) {
        print_error(ERROR_FORMAT);
        return 1;
    }

    unsigned long long n_ull;
    if (parse_ull(n_str, &n_ull) != OK || n_ull > 10000) {
        print_error(ERROR_INVALID_INPUT);
        return 1;
    }
    int n = (int)n_ull;

    unsigned long long capacity;
    if (parse_ull(cap_str, &capacity) != OK) {
        print_error(ERROR_INVALID_INPUT);
        return 1;
    }

    FileInfo *files = (FileInfo *)malloc(n * sizeof(FileInfo));
    if (files == NULL) {
        print_error(ERROR_MEMORY);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            for(int j=0; j<i; j++) free(files[j].name);
            free(files);
            print_error(ERROR_FORMAT);
            return 1;
        }

        ErrorStatus status = parse_file_line(line, &files[i]);
        if (status != OK) {
            for(int j=0; j<i; j++) free(files[j].name);
            free(files);
            print_error(status);
            return 1;
        }

        for (int j = 0; j < i; j++) {
            if (strcmp(files[i].name, files[j].name) == 0) {
                for(int k=0; k<=i; k++) free(files[k].name);
                free(files);
                print_error(ERROR_INVALID_INPUT);
                return 1;
            }
        }
    }

    BackupResult result;
    result.files = NULL;
    
    ErrorStatus status = select_files_for_backup(files, n, capacity, &result);
    if (status != OK) {
        for(int j=0; j<n; j++) free(files[j].name);
        free(files);
        print_error(status);
        return 1;
    }

    printf("TOTAL_SIZE %llu\n", result.total_size);
    printf("TOTAL_IMPORTANCE %llu\n", result.total_importance);
    printf("FILES %d\n", result.count);
    for (int i = 0; i < result.count; i++) {
        printf("%s\n", result.files[i].name);
    }

    free_backup_result(&result);
    for(int j=0; j<n; j++) free(files[j].name);
    free(files);

    return 0;
}