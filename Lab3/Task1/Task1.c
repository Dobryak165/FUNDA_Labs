#include <stdio.h>
#include <stddef.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_BASE,
    ERROR_BUFFER,
    ERROR_COUNT,
    ERROR_INPUT,
    ERROR_OVERFLOW,
    ERROR_OUTPUT
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:
            printf("Нулевой указатель\n");
            break;
        case ERROR_BASE:
            printf("r должен быть в диапазоне от 1 до 5\n");
            break;
        case ERROR_BUFFER:
            printf("Недостаточный размер массива\n");
            break;
        case ERROR_COUNT:
            printf("Использование: ./Task1 <число> <r>, где r от 1 до 5\n");
            break;
        case ERROR_INPUT:
            printf("Ожидалось неотрицательное целое число из цифр 0..9\n");
            break;
        case ERROR_OVERFLOW:
            printf("Число превышает максимум типа unsigned int\n");
            break;
        case ERROR_OUTPUT:
            printf("Ошибка вывода\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
            break;
    }
}

ErrorStatus parse_unsigned(const char *text, unsigned int *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    if (*text == '\0') return ERROR_INPUT;

    unsigned int value = 0;
    while (*text != '\0') {
        if (*text < '0' || *text > '9') return ERROR_INPUT;
        unsigned int digit = (unsigned int)(*text - '0');
        if (value > (UINT_MAX - digit) / 10u) return ERROR_OVERFLOW;
        value = value * 10u + digit;
        text++;
    }
    *result = value;
    return OK;
}

ErrorStatus convert_to_base(unsigned int number, unsigned int r, char *result, size_t capacity, size_t *length) {

    if (result == NULL || length == NULL) return ERROR_NULL;
    if (r < 1 || r > 5) return ERROR_BASE;
    if (capacity < 2) return ERROR_BUFFER;

    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
    unsigned int mask = ~(~0u << r);

    *length = 0;
    result[0] = '\0';

    if ((number >> r) != 0) {
        ErrorStatus status = convert_to_base(number >> r, r, result, capacity, length);
        if (status != OK) return status;
    }

    size_t next = *length;
    size_t bit = 1;
    while ((next & bit) != 0) {
        next ^= bit;
        bit <<= 1;
    }
    next ^= bit;

    if (next >= capacity) return ERROR_BUFFER;
    result[*length] = digits[number & mask];
    result[next] = '\0';
    *length = next;
    return OK;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_error(ERROR_COUNT);
        return 1;
    }

    unsigned int number = 0;
    unsigned int r = 0;

    ErrorStatus status = parse_unsigned(argv[1], &number);
    if (status == OK) status = parse_unsigned(argv[2], &r);
    if (status != OK) {
        print_error(status);
        return 1;
    }

    if (r < 1 || r > 5) {
        print_error(ERROR_BASE);
        return 1;
    }
    unsigned int base = 1u << r;


    char result[sizeof(unsigned int) * CHAR_BIT + 1];
    size_t length = 0;
    status = convert_to_base(number, r, result, sizeof(result), &length);
    if (status != OK) {
        print_error(status);
        return 1;
    }

    if (printf("%u -> %s (основание %u)\n", number, result, base) < 0) {
        print_error(ERROR_OUTPUT);
        return 1;
    }
    return 0;
}