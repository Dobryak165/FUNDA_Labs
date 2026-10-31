#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_EPSILON,
    ERROR_MEMORY,
    ERROR_OVERFLOW
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:     
            printf("Ошибка: нулевой указатель\n"); 
            break;
        case ERROR_COUNT:    
            printf("Ошибка: неверная степень\n"); 
            break;
        case ERROR_EPSILON:  
            printf("Ошибка: точность вне (0; 1)\n"); 
            break;
        case ERROR_MEMORY:   
            printf("Ошибка: нет памяти\n"); 
            break;
        case ERROR_OVERFLOW: 
            printf("Ошибка: переполнение\n"); 
            break;
        default:             
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

ErrorStatus shift_polynomial_core(double **result_g, double a, double epsilon, int n, const double *f_coeffs) {
    if (result_g == NULL || f_coeffs == NULL) return ERROR_NULL;
    if (n < 0) return ERROR_COUNT;
    if (epsilon <= 0.0 || epsilon >= 1.0) return ERROR_EPSILON;

    double *temp = (double *)malloc((n + 1) * sizeof(double));
    if (temp == NULL) return ERROR_MEMORY;

    double *g = (double *)malloc((n + 1) * sizeof(double));
    if (g == NULL) { 
        free(temp); 
        return ERROR_MEMORY; 
    }

    for (int i = 0; i <= n; i++) {
        temp[n - i] = f_coeffs[i];
    }

    for (int k = 0; k <= n; k++) {
        double acc = 0.0;
        for (int i = 0; i <= n - k; i++) {
            acc = acc * a + temp[i];
            if (isinf(acc) || isnan(acc)) { 
                free(temp); 
                free(g); 
                return ERROR_OVERFLOW; 
            }
            temp[i] = acc;
        }
        g[k] = temp[n - k];
        
        if (fabs(g[k]) < epsilon) {
            g[k] = 0.0;
        }
    }

    free(temp);
    *result_g = g;
    return OK;
}

ErrorStatus shift_polynomial(double **result_g, double a, double epsilon, int n, ...) {
    if (result_g == NULL) return ERROR_NULL;
    if (n < 0) return ERROR_COUNT;
    if (epsilon <= 0.0 || epsilon >= 1.0) return ERROR_EPSILON;

    double *temp_coeffs = (double *)malloc((n + 1) * sizeof(double));
    if (temp_coeffs == NULL) return ERROR_MEMORY;

    va_list args;
    va_start(args, n);
    for (int i = 0; i <= n; i++) {
        temp_coeffs[i] = va_arg(args, double);
    }
    va_end(args);

    ErrorStatus status = shift_polynomial_core(result_g, a, epsilon, n, temp_coeffs);

    free(temp_coeffs);
    
    return status;
}


void print_shift(double a) {
    if (a > 0) printf("(x - %g)", a);
    else if (a < 0) printf("(x + %g)", -a);
    else printf("x");
}

void print_polynomial(const double *coeffs, int n, double a, double epsilon) {
    printf("g(x) = ");
    int first = 1;
    for (int i = 0; i <= n; i++) {
        if (fabs(coeffs[i]) < epsilon) continue; 
        if (!first) printf(coeffs[i] > 0 ? " + " : " - ");
        else if (coeffs[i] < 0) printf("-");
        
        double abs_val = fabs(coeffs[i]);
        if (i == 0) printf("%g", abs_val);
        else if (i == 1) {
            if (fabs(abs_val - 1.0) > epsilon) printf("%g", abs_val);
            print_shift(a);
        } else {
            if (fabs(abs_val - 1.0) > epsilon) printf("%g", abs_val);
            print_shift(a);
            printf("^%d", i);
        }
        first = 0;
    }
    if (first) printf("0");
    printf("\n");
}

int main(void) {
    double *g_var = NULL;
    int test_n = 4;    
    double test_a = 2.0;  
    
    ErrorStatus status1 = shift_polynomial(&g_var, test_a, 1e-6, test_n, 5.0, -1.0, 0.0, 4.0, -3.0);

    if (status1 == OK) {
        printf("При сдвиге a = %g результат:\n  ", test_a);
        print_polynomial(g_var, test_n, test_a, 1e-6);
        free(g_var);
    } else {
        print_error(status1);
    }

    int n;
    printf("Введите степень многочлена: ");
    if (scanf("%d", &n) != 1 || n < 0) { 
        print_error(ERROR_COUNT); 
        return 1; 
    }

    double a, eps;
    printf("Введите значение a (сдвиг): ");
    if (scanf("%lf", &a) != 1) { 
        print_error(ERROR_COUNT); 
        return 1; 
    }

    printf("Введите epsilon (точность, например, 1e-6): ");
    if (scanf("%lf", &eps) != 1 || eps <= 0.0 || eps >= 1.0) { 
        print_error(ERROR_EPSILON); 
        return 1; 
    }

    double *coeffs = (double *)malloc((n + 1) * sizeof(double));
    if (coeffs == NULL) { 
        print_error(ERROR_MEMORY); 
        return 1; 
    }

    printf("Введите коэффициенты от свободного члена f0 до старшей степени f%d:\n", n);
    for (int i = 0; i <= n; i++) {
        printf("  f%d = ", i);
        if (scanf("%lf", &coeffs[i]) != 1) { 
            free(coeffs); 
            print_error(ERROR_COUNT); 
            return 1; 
        }
    }

    double *g = NULL;
    
    ErrorStatus status2 = shift_polynomial_core(&g, a, eps, n, coeffs);

    if (status2 == OK) {
        printf("\nРезультат переразложения:\n  ");
        print_polynomial(g, n, a, eps);
        free(g);
    } else {
        print_error(status2);
    }

    free(coeffs);
    return 0;
}
