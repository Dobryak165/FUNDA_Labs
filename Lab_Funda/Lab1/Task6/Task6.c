#define __USE_MINGW_ANSI_STDIO 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_BASE,
    ERROR_INVALID,
    ERROR_MATH,
    ERROR_MEMORY
} ErrorStatus;

ErrorStatus is_convex_polygon(int *result, int n, ...) {
    if (result == NULL) {
        return ERROR_NULL;
    }
    if (n < 3) {
        return ERROR_COUNT;
    }

    double *xs = (double *)malloc(n * sizeof(double));
    double *ys = (double *)malloc(n * sizeof(double));
    if (xs == NULL || ys == NULL) {
        free(xs);
        free(ys);
        return ERROR_MEMORY;
    }

    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        xs[i] = va_arg(args, double);
        ys[i] = va_arg(args, double);
    }
    va_end(args);

    int sign = 0;
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;
        double cross = (xs[j] - xs[i]) * (ys[k] - ys[j]) - (ys[j] - ys[i]) * (xs[k] - xs[j]);

        int s = 0;
        if (cross > 1e-9) {
            s = 1;
        } else if (cross < -1e-9) {
            s = -1;
        }

        if (s != 0) {
            if (sign == 0) {
                sign = s;
            } else if (sign != s) {
                free(xs);
                free(ys);
                *result = 0;
                return OK;
            }
        }
    }

    free(xs);
    free(ys);
    *result = 1;
    return OK;
}

ErrorStatus poly_value(double *result, double x, int n, ...) {
    if (result == NULL) {
        return ERROR_NULL;
    }
    if (n < 0) {
        return ERROR_COUNT;
    }

    va_list args;
    va_start(args, n);
    double val = 0.0;
    for (int i = 0; i <= n; i++) {
        double coeff = va_arg(args, double);
        val = val * x + coeff;
    }
    va_end(args);

    *result = val;
    return OK;
}

ErrorStatus is_kaprekar_number(const char *str, int base, int *result) {
    if (str == NULL || result == NULL) {
        return ERROR_NULL;
    }
    if (base < 2 || base > 36) {
        return ERROR_BASE;
    }

    char *endptr;
    long long num = strtoll(str, &endptr, base);
    if (*endptr != '\0' || endptr == str) {
        return ERROR_INVALID;
    }
    if (num < 0) {
        return ERROR_INVALID;
    }
    if (num == 0) {
        *result = 0;
        return OK;
    }

    long long sq = num * num;
    char digits[64];
    int len = 0;
    long long tmp = sq;
    const char *alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";

    while (tmp > 0) {
        digits[len] = alphabet[tmp % base];
        len++;
        tmp = tmp / base;
    }

    for (int i = 0; i < len / 2; i++) {
        char t = digits[i];
        digits[i] = digits[len - 1 - i];
        digits[len - 1 - i] = t;
    }
    digits[len] = '\0';

    for (int split = 1; split < len; split++) {
        char left_str[64];
        char right_str[64];

        memcpy(left_str, digits, split);
        left_str[split] = '\0';

        memcpy(right_str, digits + split, len - split);
        right_str[len - split] = '\0';

        long long left = strtoll(left_str, NULL, base);
        long long right = strtoll(right_str, NULL, base);

        if (left > 0 && right > 0 && left + right == num) {
            *result = 1;
            return OK;
        }
    }

    *result = 0;
    return OK;
}

ErrorStatus find_kaprekar(int base, int n, int *results, ...) {
    if (results == NULL) {
        return ERROR_NULL;
    }
    if (base < 2 || base > 36) {
        return ERROR_BASE;
    }
    if (n < 0) {
        return ERROR_COUNT;
    }

    va_list args;
    va_start(args, results);
    for (int i = 0; i < n; i++) {
        const char *s = va_arg(args, const char *);
        ErrorStatus st = is_kaprekar_number(s, base, &results[i]);
        if (st != OK) {
            va_end(args);
            return st;
        }
    }
    va_end(args);
    return OK;
}

ErrorStatus geometric_mean(double *result, int count, ...) {
    if (result == NULL) {
        return ERROR_NULL;
    }
    if (count <= 0) {
        return ERROR_COUNT;
    }

    va_list args;
    va_start(args, count);
    double product = 1.0;

    for (int i = 0; i < count; i++) {
        double x = va_arg(args, double);
        if (x <= 0) {
            va_end(args);
            return ERROR_INVALID;
        }
        product = product * x;
    }
    va_end(args);

    *result = pow(product, 1.0 / count);
    return OK;
}

double fast_pow(double base, int exp) {
    if (exp == 0) {
        return 1.0;
    }
    if (exp < 0) {
        return 1.0 / fast_pow(base, -exp);
    }
    if (exp % 2 == 0) {
        double half = fast_pow(base, exp / 2);
        return half * half;
    }
    return base * fast_pow(base, exp - 1);
}

typedef double (*MathFunc)(double);

ErrorStatus bisection(double *root, double a, double b, double eps, MathFunc f) {
    if (root == NULL || f == NULL) {
        return ERROR_NULL;
    }
    if (eps <= 0) {
        return ERROR_INVALID;
    }

    double fa = f(a);
    double fb = f(b);
    if (fa * fb > 0) {
        return ERROR_MATH;
    }

    while ((b - a) > eps) {
        double mid = (a + b) / 2.0;
        double fm = f(mid);

        if (fabs(fm) < eps) {
            *root = mid;
            return OK;
        }
        if (fa * fm < 0) {
            b = mid;
            fb = fm;
        } else {
            a = mid;
            fa = fm;
        }
    }

    *root = (a + b) / 2.0;
    return OK;
}

double func1(double x) {
    return x * x - 4.0;
}

double func2(double x) {
    return x * x * x - x - 2.0;
}

double func3(double x) {
    return cos(x) - x;
}

void print_errors(ErrorStatus error) {
    switch (error) {
        case ERROR_NULL:    
            printf("Нулевой указатель\n"); 
            break;
        case ERROR_COUNT:   
            printf("Неверное количество\n"); 
            break;
        case ERROR_BASE:    
            printf("Неверное основание\n"); 
            break;
        case ERROR_INVALID: 
            printf("Неверный аргумент\n"); 
            break;
        case ERROR_MATH:    
            printf("Решение не найдено\n"); 
            break;
        case ERROR_MEMORY:  
            printf("Нет памяти\n"); 
            break;
        default:            
            printf("Ну тут капец, что-то другое\n");
    }
}

int main(void) {
    ErrorStatus status;
    int convex;

    status = is_convex_polygon(&convex, 4, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
    if (status == OK) printf("Квадрат: %s\n", convex ? "выпуклый" : "не выпуклый");
    else print_errors(status);

    status = is_convex_polygon(&convex, 5, 0.0, 0.0, 2.0, 0.0, 1.0, 0.5, 2.0, 2.0, 0.0, 2.0);
    if (status == OK) printf("Звезда: %s\n", convex ? "выпуклый" : "не выпуклый");
    else print_errors(status);

    status = is_convex_polygon(&convex, 3, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);
    if (status == OK) printf("Треугольник: %s\n", convex ? "выпуклый" : "не выпуклый");
    else print_errors(status);

    status = is_convex_polygon(&convex, 2, 0.0, 0.0, 1.0, 0.0);
    if (status == OK) printf("2 точки: %s\n", convex ? "выпуклый" : "не выпуклый");
    else print_errors(status);

    double val;

    status = poly_value(&val, 2.0, 3, 2.0, -3.0, 4.0, -5.0);
    if (status == OK) printf("P(2) = %.4g\n", val);
    else print_errors(status);

    status = poly_value(&val, 1.0, 0, 42.0);
    if (status == OK) printf("P(1) = %.4g\n", val);
    else print_errors(status);

    int kresults[8];

    status = find_kaprekar(10, 8, kresults, "9", "10", "45", "55", "99", "100", "297", "500");
    if (status == OK) {
        const char *names[8] = {"9", "10", "45", "55", "99", "100", "297", "500"};
        for (int i = 0; i < 8; i++) {
            printf("%s - %s\n", names[i], kresults[i] ? "Капрекар" : "не Капрекар");
        }
    } else {
        print_errors(status);
    }

    double gm;

    status = geometric_mean(&gm, 3, 1.0, 2.0, 4.0);
    if (status == OK) printf("(1, 2, 4) = %.4g\n", gm);
    else print_errors(status);

    status = geometric_mean(&gm, 2, 2.0, 8.0);
    if (status == OK) printf("(2, 8) = %.4g\n", gm);
    else print_errors(status);

    status = geometric_mean(&gm, 3, 1.0, -2.0, 4.0);
    if (status == OK) printf("(1, -2, 4) = %.4g\n", gm);
    else print_errors(status);

    printf("2 ^ 10  = %.4g\n", fast_pow(2.0, 10));
    printf("2 ^ -3  = %.4g\n", fast_pow(2.0, -3));
    printf("3 ^ 0   = %.4g\n", fast_pow(3.0, 0));
    printf("1.5 ^ 7 = %.4g\n", fast_pow(1.5, 7));
    printf("-2 ^ 5  = %.4g\n", fast_pow(-2.0, 5));

    double root;

    status = bisection(&root, 0.0, 5.0, 1e-6, func1);
    if (status == OK) printf("x^2 - 4 на [0, 5]: x = %.6g\n", root);
    else print_errors(status);

    status = bisection(&root, 1.0, 2.0, 1e-6, func2);
    if (status == OK) printf("x^3 - x - 2 на [1, 2]: x = %.6g\n", root);
    else print_errors(status);

    status = bisection(&root, 0.0, 1.0, 1e-6, func3);
    if (status == OK) printf("cos(x) - x на [0, 1]: x = %.6g\n", root);
    else print_errors(status);

    status = bisection(&root, 10.0, 20.0, 1e-6, func1);
    if (status == OK) printf("x^2 - 4 на [10, 20]: x = %.6g\n", root);
    else print_errors(status);

    return 0;
}