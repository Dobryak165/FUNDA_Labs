#define __USE_MINGW_ANSI_STDIO 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_ARGC,
    ERROR_FLAG,
    ERROR_NUMBER_RANGE,
    ERROR_NUMBER_FORMAT,
    ERROR_MATH_OVERFLOW,
    ERROR_RESTRICTION
} ErrorStatus;

ErrorStatus validate_args(int argc, char *argv[]) {
    if (argc != 3) {
        return ERROR_ARGC;
    }

    if ((strlen(argv[2]) != 2) || ((argv[2][0] != '-') && (argv[2][0] != '/'))) {
        return ERROR_FLAG;
    }

    char flag = argv[2][1];
    if (flag != 'h' && flag != 'p' && flag != 's' && flag != 'e' && flag != 'a' && flag != 'f') {
        return ERROR_FLAG;
    }

    return OK;
}

ErrorStatus parse_number(const char *str, unsigned int *x) {
    char *endptr;
    long value = strtol(str, &endptr, 10);

    if (*endptr != '\0' || endptr == str) {
        return ERROR_NUMBER_FORMAT;
    }
    
    if (value <= 0 || value > UINT_MAX) {
        return ERROR_NUMBER_RANGE;
    }

    *x = (unsigned int)value;
    return OK;
}

ErrorStatus calc_multiples(unsigned int x, unsigned int *result, int *count) {
    *count = 0;
    for (unsigned int i = x; i <= 100; i += x) {
        result[*count] = i;
        (*count)++;
    }

    return OK;
}

ErrorStatus check_prime(unsigned int x, int *is_prime) {
    *is_prime = 1;
    if (x < 2) {
        *is_prime = 0;
        return OK;
    }
    
    for (unsigned int i = 2; i <= x / i; i++) {
        if (x % i == 0) {
            *is_prime = 0;
            break;
        }
    }

    return OK;
}

ErrorStatus get_hex_digits(unsigned int x, char *buffer, size_t buffer_size) {
    int hex = snprintf(buffer, buffer_size, "%x", x);
    if (hex < 0  || (size_t)hex >= buffer_size) {
        return ERROR_MATH_OVERFLOW;
    }
    
    return OK;
}

ErrorStatus table_powers(unsigned int x, unsigned long long power[10][10]) {
    if (x > 10) {
        return ERROR_RESTRICTION;
    }

    for (int base = 1; base <= 10; base++) {
        unsigned long long value = 1;
        for (unsigned int pow = 1; pow <= x; pow++) {
            if (value > ULLONG_MAX / base) {
                return ERROR_MATH_OVERFLOW;
            }
            value *= base;
            power[base-1][pow-1] = value;
        }
    }

    return OK;
}

ErrorStatus calc_sum(unsigned int x, unsigned long long *sum) {
    *sum = (unsigned long long)x * ((unsigned long long)x + 1) / 2;
    return OK;
}

ErrorStatus calc_factorial(unsigned int x, unsigned long long *factorial) {
    if (x > 20) return ERROR_MATH_OVERFLOW;
    *factorial = 1;
    for (unsigned int i = 2; i <= x; i++) {
        *factorial *= i;
    }

    return OK;
}

void print_errors(ErrorStatus error) {
    switch (error)
    {
    case ERROR_ARGC:
        printf("Неправильно кол-во аргументов\n");
        break;
    case ERROR_FLAG:
        printf("Недопустимый флаг\n");
        break;
    case ERROR_NUMBER_RANGE:
        printf("Число должно быть >0\n");
        break;
    case ERROR_NUMBER_FORMAT:
        printf("Введено не число\n");
        break;
    case ERROR_MATH_OVERFLOW:
        printf("Слишком большое число, переполнение\n");
        break;
    case ERROR_RESTRICTION:
        printf("Не выполнено условие ограничения\n");
        break;
    default:
        printf("Ну тут капец, что-то другое");
    }
}

void print_multiples(unsigned int x) {
    unsigned int results[100];
    int count = 0;
    calc_multiples(x, results, &count);

    if (count == 0) {
        printf("Чисел кратных %u не найдено", x);
    } else {
        printf("Числа кратные %u: ", x);
        for (int i = 0; i < count; i++) {
            printf("%u ", results[i]);
        }
    }
}

void print_prime(unsigned int x) {
    int is_prime;
    check_prime(x, &is_prime);
    
    if (is_prime) {
        printf("Число %u простое \n", x);
    } else {
        printf("Число %u составное \n", x);
    }
}

void print_hex_digits(unsigned int x) {
    char buffer[32];
    ErrorStatus status = get_hex_digits(x, buffer, sizeof(buffer));

    if (status == OK) {
        printf("Шестнадцатиричные цифры числа %u: ", x);
        for (size_t i = 0; i < strlen(buffer); i++) {
            printf("%c ", buffer[i]);
        }
    } else {
        print_errors(status);
    }
}

void print_table_powers(unsigned int x) {
    unsigned long long power[10][10];
    ErrorStatus status = table_powers(x, power);

    if (status == ERROR_RESTRICTION) {
        printf("Не выполнено условие ограничения \n");
    } else if (status == OK) {
        printf("Таблица степеней: \n");
        for (int base = 1; base <= 10; base++) {
            printf("Основание %d: \n", base);
            for (unsigned int exp = 1; exp <= x; exp++) {
                printf("%llu ", power[base-1][exp-1]);
            }
            printf("\n");
        }
    } else {
        print_errors(status);
    }
}

void print_sum(unsigned int x) {
    unsigned long long sum;
    calc_sum(x, &sum);
    printf("Сумма чисел от 1 до %u: %llu", x, sum);
}

void print_factorial(unsigned int x) {
    unsigned long long factorial;
    ErrorStatus status = calc_factorial(x, &factorial);

    if (status == ERROR_MATH_OVERFLOW) {
        printf("Слишком большое число, переполнение");
    } else {
        printf("Факториал %u: %llu", x, factorial);
    }
}

int main(int argc, char *argv[]) {
    ErrorStatus status = validate_args(argc, argv);

    if (status != OK) {
        print_errors(status);
        return 1;
    }

    unsigned int x;
    status = parse_number(argv[1], &x);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    char flag = argv[2][1];

    switch (flag)
    {
    case 'h':
        print_multiples(x);
        break;
    case 'p':
        print_prime(x);
        break;
    case 's':
        print_hex_digits(x);
        break;
    case 'e':
        print_table_powers(x);
        break;
    case 'a':
        print_sum(x);
        break;
    case 'f':
        print_factroial(x);
        break;
    
    default:
        print_errors(ERROR_FLAG);
        return 1;
    }
}