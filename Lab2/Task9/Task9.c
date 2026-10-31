#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define EPSILON 1e-9
#define MAX_ITER 1000

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_BASE,
    ERROR_COUNT,
    ERROR_INVALID_INPUT,
    ERROR_MEMORY
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:           
            printf("Нулевой указатель\n"); 
            break;
        case ERROR_BASE:           
            printf("Основание должно быть в диапазоне [2..36]\n"); 
            break;
        case ERROR_COUNT:          
            printf("Неверное количество аргументов\n"); 
            break;
        case ERROR_INVALID_INPUT:  
            printf("Дробь должна быть в диапазоне (0; 1)\n"); 
            break;
        case ERROR_MEMORY:         
            printf("Не удалось выделить память\n"); 
            break;
        default:                   
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

ErrorStatus check_fractions_core(int base, int count, int *results, int *iters, const double *values_array) {
    if (base < 2 || base > 36) return ERROR_BASE;
    if (results == NULL || iters == NULL || count <= 0 || values_array == NULL) return ERROR_COUNT;

    for (int i = 0; i < count; i++) {
        double value = values_array[i];
        if (value <= 0.0 || value >= 1.0) return ERROR_INVALID_INPUT;

        double current = value;
        int is_finite = 0;
        int iter_count = MAX_ITER;
        
        double history[MAX_ITER]; 
        int history_count = 0;

        for (int j = 0; j < MAX_ITER; j++) {
            current *= base;
            double int_part;
            double frac_part = modf(current, &int_part);
            
            if (frac_part < EPSILON || frac_part > (1.0 - EPSILON)) {
                is_finite = 1;
                iter_count = j + 1;
                break;
            }

            int found = 0;
            for (int k = 0; k < history_count; k++) {
                if (fabs(history[k] - frac_part) < EPSILON) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                is_finite = 0;
                iter_count = j + 1;
                break;
            }

            if (history_count < MAX_ITER) {
                history[history_count++] = frac_part;
            }
            current = frac_part;
        }

        results[i] = is_finite;
        iters[i] = iter_count;
    }
    return OK;
}

ErrorStatus check_fractions(int base, int count, int *results, int *iters, ...) {
    if (base < 2 || base > 36) return ERROR_BASE;
    if (results == NULL || iters == NULL || count <= 0) return ERROR_COUNT;

    double *temp_array = (double *)malloc(count * sizeof(double));
    if (temp_array == NULL) return ERROR_MEMORY;

    va_list args;
    va_start(args, iters);
    for (int i = 0; i < count; i++) {
        temp_array[i] = va_arg(args, double);
    }
    va_end(args);

    ErrorStatus status = check_fractions_core(base, count, results, iters, temp_array);
    free(temp_array);
    return status;
}

int main(void) {
    int test_res[3];
    int test_iters[3];
    
    ErrorStatus status1 = check_fractions(2, 3, test_res, test_iters, 0.14, 0.125, 0.888);
    
    if (status1 == OK) {
        double test_vals[] = {0.14, 0.125, 0.888};
        for (int i = 0; i < 3; i++) {
            printf("  %.15g -> %s (итераций: %d)\n", test_vals[i], test_res[i] ? "Конечное" : "Бесконечное", test_iters[i]);
        }
    } else {
        print_error(status1);
    }

    int base;
    printf("Введите основание системы счисления (2..36): ");
    if (scanf("%d", &base) != 1 || base < 2 || base > 36) {
        print_error(ERROR_BASE);
        return 1;
    }

    int count;
    printf("Введите количество дробей для проверки: ");
    if (scanf("%d", &count) != 1 || count <= 0) {
        print_error(ERROR_COUNT);
        return 1;
    }

    double *values = (double *)malloc(count * sizeof(double));
    int *results = (int *)malloc(count * sizeof(int));
    int *iters = (int *)malloc(count * sizeof(int));
    
    if (!values || !results || !iters) {
        print_error(ERROR_MEMORY);
        free(values); free(results); free(iters);
        return 1;
    }

    printf("Введите %d дробей (в диапазоне от 0 до 1):\n", count);
    for (int i = 0; i < count; i++) {
        printf("Дробь %d: ", i + 1);
        if (scanf("%lf", &values[i]) != 1) {
            print_error(ERROR_INVALID_INPUT);
            free(values); free(results); free(iters);
            return 1;
        }
        if (values[i] <= 0.0 || values[i] >= 1.0) {
            print_error(ERROR_INVALID_INPUT);
            free(values); free(results); free(iters);
            return 1;
        }
    }

    ErrorStatus status2 = check_fractions_core(base, count, results, iters, values);

    if (status2 == OK) {
        printf("\nРезультат (основание %d):\n", base);
        for (int i = 0; i < count; i++) {
            printf("  %.10g -> %s (итераций: %d)\n", values[i], results[i] ? "Конечное" : "Бесконечное", iters[i]);
        }
    } else {
        print_error(status2);
    }

    free(values);
    free(results);
    free(iters);
    return 0;
}
