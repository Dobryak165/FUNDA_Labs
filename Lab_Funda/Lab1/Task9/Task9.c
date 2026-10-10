#define __USE_MINGW_ANSI_STDIO 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define FIXED_SIZE   100
#define MIN_DYN_SIZE 10
#define MAX_DYN_SIZE 10000
#define MIN_VAL     -1000
#define MAX_VAL      1000

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_ARGC,
    ERROR_NUMBER_FORMAT,
    ERROR_NUMBER_RANGE,
    ERROR_COUNT,
    ERROR_MEMORY
} ErrorStatus;

ErrorStatus parse_int(const char *str, int *out) {
    if (str == NULL || out == NULL) {
        return ERROR_NULL;
    }
    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (*endptr != '\0' || endptr == str) {
        return ERROR_NUMBER_FORMAT;
    }
    if (val < INT_MIN || val > INT_MAX) {
        return ERROR_NUMBER_RANGE;
    }
    *out = (int)val;
    return OK;
}

ErrorStatus generate_fixed_array(int *arr, int size, int a, int b) {
    if (arr == NULL) {
        return ERROR_NULL;
    }
    if (size <= 0) {
        return ERROR_COUNT;
    }
    if (a > b) {
        return ERROR_NUMBER_RANGE;
    }
    int range = b - a + 1;
    for (int i = 0; i < size; i++) {
        arr[i] = a + rand() % range;
    }
    return OK;
}

ErrorStatus find_and_swap_min_max(int *arr, int size, int *min_idx_out, int *max_idx_out) {
    if (arr == NULL || min_idx_out == NULL || max_idx_out == NULL) {
        return ERROR_NULL;
    }
    if (size < 2) {
        return ERROR_COUNT;
    }

    int min_idx = 0;
    int max_idx = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] < arr[min_idx]) {
            min_idx = i;
        }
        if (arr[i] > arr[max_idx]) {
            max_idx = i;
        }
    }

    int tmp = arr[min_idx];
    arr[min_idx] = arr[max_idx];
    arr[max_idx] = tmp;

    *min_idx_out = min_idx;
    *max_idx_out = max_idx;
    return OK;
}

ErrorStatus generate_dynamic_array(int **arr, int *size, int min_size, int max_size, int min_val, int max_val) {
    if (arr == NULL || size == NULL) {
        return ERROR_NULL;
    }
    if (min_size <= 0 || max_size < min_size) {
        return ERROR_COUNT;
    }
    if (min_val > max_val) {
        return ERROR_NUMBER_RANGE;
    }

    int n = min_size + rand() % (max_size - min_size + 1);
    int *a = (int *)malloc(n * sizeof(int));
    if (a == NULL) {
        return ERROR_MEMORY;
    }

    int range = max_val - min_val + 1;
    for (int i = 0; i < n; i++) {
        a[i] = min_val + rand() % range;
    }

    *arr = a;
    *size = n;
    return OK;
}

int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

ErrorStatus find_nearest(int *sorted, int size, int target, int *nearest) {
    if (sorted == NULL || nearest == NULL) {
        return ERROR_NULL;
    }
    if (size <= 0) {
        return ERROR_COUNT;
    }

    if (target <= sorted[0]) {
        *nearest = sorted[0];
        return OK;
    }
    if (target >= sorted[size - 1]) {
        *nearest = sorted[size - 1];
        return OK;
    }

    int left = 0;
    int right = size - 1;

    while (right - left > 1) {
        int mid = (left + right) / 2;
        if (sorted[mid] == target) {
            *nearest = target;
            return OK;
        }
        if (sorted[mid] < target) {
            left = mid;
        } else {
            right = mid;
        }
    }

    int d_left = target - sorted[left];
    int d_right = sorted[right] - target;

    if (d_left <= d_right) {
        *nearest = sorted[left];
    } else {
        *nearest = sorted[right];
    }
    return OK;
}

ErrorStatus form_array_c(int *A, int sizeA, int *B, int sizeB, int **C) {
    if (A == NULL || B == NULL || C == NULL) {
        return ERROR_NULL;
    }
    if (sizeA <= 0 || sizeB <= 0) {
        return ERROR_COUNT;
    }

    int *B_sorted = (int *)malloc(sizeB * sizeof(int));
    if (B_sorted == NULL) {
        return ERROR_MEMORY;
    }
    memcpy(B_sorted, B, sizeB * sizeof(int));
    qsort(B_sorted, sizeB, sizeof(int), compare_ints);

    int *c = (int *)malloc(sizeA * sizeof(int));
    if (c == NULL) {
        free(B_sorted);
        return ERROR_MEMORY;
    }

    for (int i = 0; i < sizeA; i++) {
        int nearest;
        ErrorStatus st = find_nearest(B_sorted, sizeB, A[i], &nearest);
        if (st != OK) {
            free(B_sorted);
            free(c);
            return st;
        }
        c[i] = A[i] + nearest;
    }

    free(B_sorted);
    *C = c;
    return OK;
}

void print_errors(ErrorStatus error) {
    switch (error) {
        case ERROR_NULL:          
            printf("Нулевой указатель\n"); 
            break;
        case ERROR_ARGC:          
            printf("Неверное количество аргументов\n");
            break;
        case ERROR_NUMBER_FORMAT: 
            printf("Введено не число\n"); 
            break;
        case ERROR_NUMBER_RANGE:  
            printf("Число вне диапазона\n"); 
            break;
        case ERROR_COUNT:         
            printf("Неверное количество\n"); 
            break;
        case ERROR_MEMORY:        
            printf("Нет памяти\n"); 
        break;
        default:                  
            printf("Ну тут капец, что-то другое\n");
    }
}

void print_array_prefix(int *arr, int size, int limit) {
    int n = (size < limit) ? size : limit;
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL) ^ (unsigned)clock());

    if (argc != 3) {
        print_errors(ERROR_ARGC);
        return 1;
    }

    int a, b;
    ErrorStatus status = parse_int(argv[1], &a);
    if (status != OK) {
        print_errors(status);
        return 1;
    }
    status = parse_int(argv[2], &b);
    if (status != OK) {
        print_errors(status);
        return 1;
    }
    if (a > b) {
        print_errors(ERROR_NUMBER_RANGE);
        return 1;
    }

    int fixed[FIXED_SIZE];
    status = generate_fixed_array(fixed, FIXED_SIZE, a, b);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    printf("Исходный массив:\n");
    print_array_prefix(fixed, FIXED_SIZE, 20);

    int min_before = 0;
    int max_before = 0;
    int min_idx = 0;
    int max_idx = 0;

    for (int i = 0; i < FIXED_SIZE; i++) {
        if (fixed[i] < fixed[min_before]) {
            min_before = i;
        }
        if (fixed[i] > fixed[max_before]) {
            max_before = i;
        }
    }
    min_idx = min_before;
    max_idx = max_before;

    printf("До замены: min = %d (индекс %d), max = %d (индекс %d)\n",
           fixed[min_idx], min_idx, fixed[max_idx], max_idx);

    status = find_and_swap_min_max(fixed, FIXED_SIZE, &min_idx, &max_idx);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    printf("После замены: arr[%d] = %d, arr[%d] = %d\n",
           min_idx, fixed[min_idx], max_idx, fixed[max_idx]);

    printf("Область вокруг индекса %d после swap:\n", min_idx);
    print_around(fixed, FIXED_SIZE, min_idx, 3);

    printf("Область вокруг индекса %d после swap:\n", max_idx);
    print_around(fixed, FIXED_SIZE, max_idx, 3);

    int *A = NULL;
    int *B = NULL;
    int *C = NULL;
    int sizeA = 0;
    int sizeB = 0;

    status = generate_dynamic_array(&A, &sizeA, MIN_DYN_SIZE, MAX_DYN_SIZE, MIN_VAL, MAX_VAL);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    status = generate_dynamic_array(&B, &sizeB, MIN_DYN_SIZE, MAX_DYN_SIZE, MIN_VAL, MAX_VAL);
    if (status != OK) {
        free(A);
        print_errors(status);
        return 1;
    }

    printf("Размер A = %d, размер B = %d\n", sizeA, sizeB);

    qsort(B, sizeB, sizeof(int), compare_ints);

    status = form_array_c(A, sizeA, B, sizeB, &C);
    if (status != OK) {
        free(A);
        free(B);
        print_errors(status);
        return 1;
    }

    printf("Первые 10 элементов A: ");
    print_array_prefix(A, sizeA < 10 ? sizeA : 10, 10);

    printf("Первые 10 элементов B (отсортирован): ");
    print_array_prefix(B, sizeB < 10 ? sizeB : 10, 10);

    printf("Первые 10 элементов C = A[i] + ближайший из B: ");
    print_array_prefix(C, sizeA < 10 ? sizeA : 10, 10);

    free(A);
    free(B);
    free(C);

    return 0;
}