#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_INVALID_INPUT,
    ERROR_MEMORY,
    ERROR_ARGC,
    ERROR_FLAG
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_ARGC:
            printf("Неверное количество аргументов\n");
            break;
        case ERROR_FLAG:
            printf("Нневерный формат флага\n");
            break;
        case ERROR_INVALID_INPUT:
            printf("Неверные входные данные\n");
            break;
        case ERROR_MEMORY:
            printf("Не удалось выделить память\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
            break;
    }
}

ErrorStatus validate_args(int argc, char *argv[]) {
    if (argc < 3) {
        return ERROR_ARGC;
    }

    const char *flag = argv[1];
    
    if (flag[0] != '-' && flag[0] != '/') {
        return ERROR_FLAG;
    }
    
    if (flag[1] == '\0' || flag[2] != '\0') {
        return ERROR_FLAG;
    }

    char action = flag[1];
    if (action != 'l' && action != 'r' && action != 'u' && action != 'n' && action != 'c') {
        return ERROR_FLAG;
    }

    if (action == 'c' && argc < 4) {
        return ERROR_ARGC;
    }

    return OK;
}

ErrorStatus parse_unsigned_int(const char *str, unsigned int *result) {
    if (str == NULL || result == NULL || str[0] == '\0') {
        return ERROR_INVALID_INPUT;
    }

    unsigned int value = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return ERROR_INVALID_INPUT;
        }
        
        unsigned int digit = str[i] - '0';
        
        if (value > (UINT_MAX - digit) / 10) {
            return ERROR_INVALID_INPUT;
        }
        
        value = value * 10 + digit;
    }

    *result = value;
    return OK;
}

ErrorStatus string_len(const char *str, unsigned int *length) {
    if (str == NULL || length == NULL) {
        return ERROR_INVALID_INPUT;
    }

    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }

    *length = len;
    return OK;
}

ErrorStatus string_reverse(const char *str, char **result) {
    if (str == NULL || result == NULL) {
        return ERROR_INVALID_INPUT;
    }

    unsigned int len = 0;
    if (string_len(str, &len) != OK) {
        return ERROR_INVALID_INPUT;
    }

    if (len == UINT_MAX) {
        return ERROR_MEMORY;
    }

    char *reversed_string = malloc(len + 1);
    if (reversed_string == NULL) {
        return ERROR_MEMORY;
    }

    for (unsigned int i = 0; i < len; i++) {
        reversed_string[i] = str[len - 1 - i];
    }

    reversed_string[len] = '\0';

    *result = reversed_string;
    return OK;
}

ErrorStatus string_uppOdd(const char *str, char **result) {
    if (str == NULL || result == NULL) {
        return ERROR_INVALID_INPUT;
    }

    unsigned int len = 0;
    if (string_len(str, &len) != OK) {
        return ERROR_INVALID_INPUT;
    }

    if (len == UINT_MAX) {
        return ERROR_MEMORY;
    }

    char *upped_string = malloc(len + 1);
    if (upped_string == NULL) {
        return ERROR_MEMORY;
    }

    for (unsigned int i = 0; i < len; i++) {
        char s = str[i];

        if (i % 2 != 0 && s >= 'a' && s <= 'z') {
            s = s - 32;
        }

        upped_string[i] = s;
    }

    upped_string[len] = '\0';

    *result = upped_string;
    return OK;
}

ErrorStatus string_sort(const char *str, char **result) {
    if (str == NULL || result == NULL) {
        return ERROR_INVALID_INPUT;
    }

    unsigned int len = 0;
    if (string_len(str, &len) != OK) {
        return ERROR_INVALID_INPUT;
    }

    if (len == UINT_MAX) {
        return ERROR_MEMORY;
    }

    char *sorted_string = malloc(len + 1);
    if (sorted_string == NULL) {
        return ERROR_MEMORY;
    }

    unsigned int index = 0;

    for (unsigned int i = 0; i < len; i++) {
        char s = str[i];

        if (s >= '0' && s <= '9') {
            sorted_string[index] = s;
            index++;
        }
    }

    for (unsigned int i = 0; i < len; i++) {
        char s = str[i];

        if ((s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z')) {
            sorted_string[index] = s;
            index++;
        }
    }

    for (unsigned int i = 0; i < len; i++) {
        char s = str[i];
        int is_digit = (s >= '0' && s <= '9');
        int is_letter = ((s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z'));

        if (!is_digit && !is_letter) {
            sorted_string[index] = s;
            index++;
        }
    }

    sorted_string[len] = '\0';
    
    *result = sorted_string;
    return OK;
}

ErrorStatus string_concat(int count, char **strings, unsigned int seed, char **result) {
    if (strings == NULL || result == NULL || count <= 0) {
        return ERROR_INVALID_INPUT;
    }

    unsigned int all_len = 0;

    for (int i = 0; i < count; i++) {
        unsigned int len = 0;
        
        if (string_len(strings[i], &len) != OK) {
            return ERROR_INVALID_INPUT;
        }

        if (all_len > UINT_MAX - len) {
            return ERROR_MEMORY;
        }
        
        all_len += len;
    }

    if (all_len == UINT_MAX) {
        return ERROR_MEMORY;
    }

    int *index = malloc(count * sizeof(int));
    if (index == NULL) {
        return ERROR_MEMORY;
    }

    char *concat_string = malloc(all_len + 1);
    if (concat_string == NULL) {
        free(index);
        return ERROR_MEMORY;
    }

    for (int i = 0; i < count; i++) {
        index[i] = i;
    }

    srand(seed);

    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = index[i];
        index[i] = index[j];
        index[j] = temp;
    }

    unsigned int current_pos = 0;
    for (int i = 0; i < count; i++) {
        int str_idx = index[i];
        unsigned int len = 0;
        string_len(strings[str_idx], &len);

        for (unsigned int j = 0; j < len; j++) {
            concat_string[current_pos] = strings[str_idx][j];
            current_pos++;
        }
    }

    concat_string[all_len] = '\0';

    free(index);
    *result = concat_string;
    return OK;
}

int main(int argc, char *argv[]) {
    ErrorStatus status = validate_args(argc, argv);
    if (status != OK) {
        print_error(status);
        return status;
    }

    char action = argv[1][1];
    char *result = NULL;

    if (action == 'l') {
        unsigned int len = 0;
        status = string_len(argv[2], &len);
        if (status == OK) {
            printf("Длина строки: %u\n", len);
        }
    } 
    else if (action == 'r') {
        status = string_reverse(argv[2], &result);
        if (status == OK) {
            printf("Результат: %s\n", result);
        }
    } 
    else if (action == 'u') {
        status = string_uppOdd(argv[2], &result);
        if (status == OK) {
            printf("Результат: %s\n", result);
        }
    } 
    else if (action == 'n') {
        status = string_sort(argv[2], &result);
        if (status == OK) {
            printf("Результат: %s\n", result);
        }
    } 
    else if (action == 'c') {
        unsigned int seed = 0;
        if (parse_unsigned_int(argv[2], &seed) != OK) {
            print_error(ERROR_INVALID_INPUT);
            return ERROR_INVALID_INPUT;
        }

        int count = argc - 3;
        char **strings = &argv[3];

        status = string_concat(count, strings, seed, &result);
        if (status == OK) {
            printf("Результат: %s\n", result);
        }
    } 

    if (status != OK) {
        print_error(status);
    }

    if (result != NULL) {
        free(result);
    }

    return status;
}