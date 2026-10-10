#define __USE_MINGW_ANSI_STDIO 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_ARGC,
    ERROR_FLAG,
    ERROR_NUMBER_RANGE,
    ERROR_NUMBER_FORMAT,
    ERROR_MATH,
    ERROR_RESTRICTION
} ErrorStatus;

ErrorStatus validate_args(int argc, char *argv[]) {
    if (argc < 4) {
        return ERROR_ARGC;
    }

    const char *flag_l = argv[argc - 1];
    if ((strlen(flag_l) != 2) || ((flag_l[0] != '-') && (flag_l[0] != '/'))) {
        return ERROR_FLAG;
    }

    char flag = flag_l[1];
    if (flag == 'm') {
        if (argc != 4) return ERROR_ARGC;
    } else if (flag == 'q' || flag == 't') {
        if(argc != 6) return ERROR_ARGC;
    } else {
        return ERROR_FLAG;
    }

    return OK;
}

ErrorStatus parse_double(const char *str, double *out) {
    char *endptr;
    double val = strtod(str, &endptr);

    if (*endptr != '\0' || endptr == str) {
        return ERROR_NUMBER_FORMAT;
    }

    if (val < -DBL_MAX || val > DBL_MAX) {
        return ERROR_NUMBER_RANGE;
    }

    *out = val;
    return OK;
}

ErrorStatus parse_int(const char *str, int *out) {
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


ErrorStatus solve_quadratic(double a, double b, double c, double eps, double *x1, double *x2, int *roots) {
    if (fabs(a) < eps) {
        if (fabs(b) < eps) {
            return ERROR_MATH;
        }

        *x1 = -c / b;
        *roots = 1;
        return OK;
    }

    double d = b*b - 4*a*c;

    if (d < -eps) {
        *roots = 0;
    } else if (fabs(d) <= eps) {
        *x1 = -b / (2*a);
        *roots = 1;
    } else {
        *x1 = (-b + sqrt(d)) / (2*a);
        *x2 = (-b - sqrt(d)) / (2*a);
        *roots = 2;
    }

    return OK;
}

ErrorStatus check_multiple(int a, int b, int *is_multiple) {
    if (b == 0) {
        return ERROR_RESTRICTION;
    }

    *is_multiple = (a%b == 0);
    return OK;
}

ErrorStatus check_right_triangle(double a, double b, double c, double eps, int *is_right) {
    if(a <= eps || b <= eps || c <= eps) {
        *is_right = 0;
        return OK;
    }

    *is_right = 0;
    if(fabs(a*a + b*b - c*c) < eps) *is_right = 1;
    else if (fabs(a*a + c*c - b*b) < eps) *is_right = 1;
    else if (fabs(b*b + c*c - a*a) < eps) *is_right = 1;
    
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
        printf("Число вне диапозона \n");
        break;
    case ERROR_NUMBER_FORMAT:
        printf("Введено не число\n");
        break;
    case ERROR_MATH:
        printf("Уравнение не имеет решения\n");
        break;
    case ERROR_RESTRICTION:
        printf("Не выполнено условие ограничения\n");
        break;
    default:
        printf("Ну тут капец, что-то другое");
    }
}

void print_quadratic_result(double eps, double a, double b, double c) {
    printf("Решение уравнения: \n");

    double perms[6][3] = {
        {a, b, c}, {a, c, b},
        {b, a, c}, {b, c, a},
        {c, a, b}, {c, b, a}
    };
    
    for (int i = 0; i < 6; i++) {
        int is_dup = 0;
        for (int j = 0; j < i; j++) {
            if (perms[i][0] == perms[j][0] && perms[i][1] == perms[j][1] && perms[i][2] == perms[j][2]) {
                is_dup = 1; break;
            }
        }
        if (is_dup) continue;

        double x1, x2;
        int roots;
        ErrorStatus status = solve_quadratic(perms[i][0], perms[i][1], perms[i][2], eps, &x1, &x2, &roots);

        if (status == ERROR_MATH) {
            printf("(%.4g, %.4g, %.4g): Нет решений \n", perms[i][0], perms[i][1], perms[i][2]);
        } else if (roots == 0) {
            printf("(%.4g, %.4g, %.4g): Нет действительных корней \n", perms[i][0], perms[i][1], perms[i][2]);
        } else if (roots == 1) {
            printf("(%.4g, %.4g, %.4g): x = %.4g \n", perms[i][0], perms[i][1], perms[i][2], x1);
        } else {
            printf("(%.4g, %.4g, %.4g): x1 = %.4g, x2 = %.4g \n", perms[i][0], perms[i][1], perms[i][2], x1, x2);
        }
    }
}

void print_multiple_result(int a, int b, int is_multiple) {
    if (is_multiple) {
        printf("Число %d кратно числу %d\n", a, b);
    } else {
        printf("Число %d не кратно числу %d\n", a, b);
    }
}

void print_triangle_result(double a, double b, double c, int is_right) {
    if (is_right) {
        printf("Числа %.4g, %.4g, %.4g могут быть сторонами прямоугольного треугольника\n", a, b, c);
    } else {
        printf("Числа %.4g, %.4g, %.4g не могут быть сторонами прямоугольного треугольника\n", a, b, c);
    }
}

int main(int argc, char *argv[]) {
    ErrorStatus status = validate_args(argc, argv);
    if (status != OK) {
        print_errors(status);
        return 1;
    }

    char flag = argv[argc - 1][1];

    if (flag == 'q') {
        double eps, a, b, c;
        if (parse_double(argv[1], &eps) != OK || parse_double(argv[2], &a) != OK ||
            parse_double(argv[3], &b) != OK || parse_double(argv[4], &c) != OK) {
            print_errors(ERROR_NUMBER_FORMAT);
            return 1;
        }
        print_quadratic_result(eps, a, b, c);
    } 
    else if (flag == 'm') {
        int a, b;
        if (parse_int(argv[1], &a) != OK || parse_int(argv[2], &b) != OK) {
            print_errors(ERROR_NUMBER_FORMAT);
            return 1;
        }
        int is_mult;
        status = check_multiple(a, b, &is_mult);
        if (status != OK) {
            print_errors(status);
            return 1;
        }
        print_multiple_result(a, b, is_mult);
    } 
    else if (flag == 't') {
        double eps, a, b, c;
        if (parse_double(argv[1], &eps) != OK || parse_double(argv[2], &a) != OK ||
            parse_double(argv[3], &b) != OK || parse_double(argv[4], &c) != OK) {
            print_errors(ERROR_NUMBER_FORMAT);
            return 1;
        }
        int is_right;
        status = check_right_triangle(a, b, c, eps, &is_right);
        if (status != OK) {
            print_errors(status);
            return 1;
        }
        print_triangle_result(a, b, c, is_right);
    }

    return 0;
}
