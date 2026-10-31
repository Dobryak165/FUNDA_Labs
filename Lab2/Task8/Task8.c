#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_BASE,
    ERROR_COUNT,
    ERROR_FORMAT,
    ERROR_MEMORY
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:   
            printf("Нулевой указатель\n"); 
            break;
        case ERROR_BASE:   
            printf("Основание системы счисления вне диапазона [2..36]\n"); 
            break;
        case ERROR_COUNT:  
            printf("Неверное количество аргументов\n"); 
            break;
        case ERROR_FORMAT: 
            printf("Неверный формат числа\n"); 
            break;
        case ERROR_MEMORY: 
            printf("Не удалось выделить память\n"); 
            break;
        default:           
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

int char_to_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

char value_to_char(int v) {
    if (v >= 0 && v <= 9) return (char)('0' + v);
    return (char)('A' + (v - 10));
}

ErrorStatus validate_number_string(const char *str, int base) {
    if (str == NULL) return ERROR_NULL;
    if (strlen(str) == 0) return ERROR_FORMAT;
    
    for (int i = 0; str[i] != '\0'; i++) {
        int value = char_to_value(str[i]);
        if (value < 0 || value >= base) {
            return ERROR_FORMAT;
        }
    }
    return OK;
}

char* remove_leading_zeros(char *str) {
    if (str == NULL) return NULL;
    
    size_t len = strlen(str);
    size_t start = 0;
    if (len > 0) {
        while (start < len - 1 && str[start] == '0') {
            start++;
        }
    }
    
    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
    return str;
}

ErrorStatus add_in_base(const char *num1, const char *num2, int base, char **result) {
    if (num1 == NULL || num2 == NULL || result == NULL) return ERROR_NULL;

    int len1 = (int)strlen(num1);
    int len2 = (int)strlen(num2);
    int max_len = (len1 > len2) ? len1 : len2;
    
    char *temp = (char *)malloc(max_len + 2);
    if (temp == NULL) return ERROR_MEMORY;
    
    int i = len1 - 1;
    int j = len2 - 1;
    int k = 0;
    int carry = 0;
    
    while (i >= 0 || j >= 0 || carry > 0) {
        int v1 = (i >= 0) ? char_to_value(num1[i]) : 0;
        int v2 = (j >= 0) ? char_to_value(num2[j]) : 0;
        
        int sum = v1 + v2 + carry;
        temp[k++] = value_to_char(sum % base);
        carry = sum / base;
        
        i--;
        j--;
    }
    temp[k] = '\0';
    
    char *res = (char *)malloc(k + 1);
    if (res == NULL) {
        free(temp);
        return ERROR_MEMORY;
    }
    
    for (int idx = 0; idx < k; idx++) {
        res[idx] = temp[k - 1 - idx];
    }
    res[k] = '\0';
    free(temp);
    
    remove_leading_zeros(res);
    *result = res;
    return OK;
}

ErrorStatus vsum_numbers_in_base(int base, int count, char **result, va_list args) {
    if (base < 2 || base > 36) return ERROR_BASE;
    if (count < 0 || result == NULL) return ERROR_COUNT;

    char *current_sum = (char *)malloc(2);
    if (current_sum == NULL) return ERROR_MEMORY;
    strcpy(current_sum, "0");

    for (int i = 0; i < count; i++) {
        const char *next_string = va_arg(args, const char *);
        
        ErrorStatus status = validate_number_string(next_string, base);
        if (status != OK) {
            free(current_sum);
            return status;
        }

        char *new_sum = NULL;
        status = add_in_base(current_sum, next_string, base, &new_sum);
        if (status != OK) {
            free(current_sum);
            return status;
        }

        free(current_sum);
        current_sum = new_sum;
    }

    *result = current_sum;
    return OK;
}

ErrorStatus sum_numbers_in_base(int base, int count, char **result, ...) {
    va_list args;
    va_start(args, result);
    ErrorStatus status = vsum_numbers_in_base(base, count, result, args);
    va_end(args);
    return status;
}

int main(void) {
    char *res_var = NULL;
    ErrorStatus status1 = sum_numbers_in_base(16, 3, &res_var, "A", "B", "3");
    if (status1 == OK) {
        printf("Результат: %s\n", res_var);
        free(res_var);
    } else {
        print_error(status1);
    }

    int base, count;
    printf("Введите основание системы счисления (2..36): ");
    if (scanf("%d", &base) != 1 || base < 2 || base > 36) {
        print_error(ERROR_BASE);
        return 1;
    }

    printf("Введите количество чисел: ");
    if (scanf("%d", &count) != 1 || count < 0) {
        print_error(ERROR_COUNT);
        return 1;
    }

    if (count == 0) {
        printf("Результат: 0\n");
        return 0;
    }

    char *res_interactive = (char *)malloc(2);
    if (res_interactive == NULL) {
        print_error(ERROR_MEMORY);
        return 1;
    }
    strcpy(res_interactive, "0");

    char buffer[2048];

    for (int i = 0; i < count; i++) {
        printf("Введите число %d: ", i + 1);

        if (scanf("%2047s", buffer) != 1) {
            print_error(ERROR_FORMAT);
            free(res_interactive);
            return 1;
        }

        ErrorStatus status = validate_number_string(buffer, base);
        if (status != OK) {
            print_error(status);
            free(res_interactive);
            return 1;
        }

        char *next_sum = NULL;
        status = add_in_base(res_interactive, buffer, base, &next_sum);
        if (status != OK) {
            print_error(status);
            free(res_interactive);
            return 1;
        }

        free(res_interactive);
        res_interactive = next_sum;
    }

    printf("\nРезультат интерактивного сложения: %s\n", res_interactive);
    free(res_interactive);
    return 0;
}
