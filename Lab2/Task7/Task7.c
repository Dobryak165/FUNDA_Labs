#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define EPSILON 1e-6

typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_MEMORY,
    ERROR_INVALID_INPUT
} ErrorStatus;

typedef struct {
    double *coords;
    int dim;
} Vector;

typedef struct {
    double **data;
    int dim;
} Matrix;

typedef double (*NormFunc)(const Vector *v, const void *context);

typedef struct {
    NormFunc func;
    double max_value;
    const Vector **max_vectors;
    int max_count;
} NormResult;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:          
            printf("Ошибка: нулевой указатель\n"); 
            break;
        case ERROR_COUNT:         
            printf("Ошибка: неверное количество\n"); 
            break;
        case ERROR_MEMORY:        
            printf("Ошибка выделения памяти\n"); 
            break;
        case ERROR_INVALID_INPUT: 
            printf("Ошибка: неверные входные данные\n"); 
            break;
        default:                  
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

double norm_inf(const Vector *v, const void *context) {
    (void)context;
    if (v == NULL || v->coords == NULL) return 0.0;
    double max_val = 0.0;
    for (int i = 0; i < v->dim; i++) {
        double abs_val = fabs(v->coords[i]);
        if (abs_val > max_val) max_val = abs_val;
    }
    return max_val;
}

double norm_p(const Vector *v, const void *context) {
    if (v == NULL || v->coords == NULL || context == NULL) return 0.0;
    double p = *(const double *)context;
    if (p < 1.0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < v->dim; i++) sum += pow(fabs(v->coords[i]), p);
    return pow(sum, 1.0 / p);
}

double norm_a(const Vector *v, const void *context) {
    if (v == NULL || v->coords == NULL || context == NULL) return 0.0;
    const Matrix *m = (const Matrix *)context;
    double sum = 0.0;
    for (int i = 0; i < v->dim; i++) {
        double ax_i = 0.0;
        for (int j = 0; j < v->dim; j++) ax_i += m->data[i][j] * v->coords[j];
        sum += ax_i * v->coords[i];
    }
    if (sum < 0.0) sum = 0.0;
    return sqrt(sum);
}

ErrorStatus find_longest_vectors_core(int n, int v_count, int norm_count, const Vector **vectors_array, NormFunc *funcs, const void **contexts, NormResult **out_results) {
    if (out_results == NULL || vectors_array == NULL || funcs == NULL || contexts == NULL) return ERROR_NULL;
    if (v_count <= 0 || norm_count <= 0 || n <= 0) return ERROR_COUNT;

    for (int i = 0; i < v_count; i++) {
        if (vectors_array[i] == NULL || vectors_array[i]->dim != n || vectors_array[i]->coords == NULL) {
            return ERROR_INVALID_INPUT;
        }
    }

    NormResult *results = (NormResult *)malloc(norm_count * sizeof(NormResult));
    if (results == NULL) return ERROR_MEMORY;

    for (int i = 0; i < norm_count; i++) {
        results[i].func = funcs[i];
        results[i].max_value = -1.0;
        results[i].max_vectors = NULL;
        results[i].max_count = 0;

        for (int j = 0; j < v_count; j++) {
            double val = funcs[i](vectors_array[j], contexts[i]);
            if (val > results[i].max_value) results[i].max_value = val;
        }

        int count = 0;
        for (int j = 0; j < v_count; j++) {
            double val = funcs[i](vectors_array[j], contexts[i]);
            if (fabs(val - results[i].max_value) < EPSILON) count++;
        }

        results[i].max_count = count;
        results[i].max_vectors = (const Vector **)malloc(count * sizeof(Vector *));
        if (results[i].max_vectors == NULL) {
            for (int k = 0; k < i; k++) free((void *)results[k].max_vectors);
            free(results);
            return ERROR_MEMORY;
        }

        int idx = 0;
        for (int j = 0; j < v_count; j++) {
            double val = funcs[i](vectors_array[j], contexts[i]);
            if (fabs(val - results[i].max_value) < EPSILON) {
                results[i].max_vectors[idx++] = vectors_array[j];
            }
        }
    }

    *out_results = results;
    return OK;
}

ErrorStatus vfind_longest_vectors(int n, int v_count, int norm_count, NormResult **out_results, va_list args) {
    const Vector **vectors_array = (const Vector **)malloc(v_count * sizeof(Vector *));
    NormFunc *funcs = (NormFunc *)malloc(norm_count * sizeof(NormFunc));
    const void **contexts = (const void **)malloc(norm_count * sizeof(void *));

    if (!vectors_array || !funcs || !contexts) {
        free(vectors_array); free(funcs); free(contexts);
        return ERROR_MEMORY;
    }

    for (int i = 0; i < v_count; i++) {
        vectors_array[i] = va_arg(args, const Vector *);
    }

    for (int i = 0; i < norm_count; i++) {
        funcs[i] = va_arg(args, NormFunc);
        contexts[i] = va_arg(args, const void *);
    }

    ErrorStatus status = find_longest_vectors_core(n, v_count, norm_count, vectors_array, funcs, contexts, out_results);

    free(vectors_array);
    free(funcs);
    free(contexts);
    return status;
}

ErrorStatus find_longest_vectors(int n, int v_count, int norm_count, NormResult **out_results, ...) {
    va_list args;
    va_start(args, out_results);
    ErrorStatus status = vfind_longest_vectors(n, v_count, norm_count, out_results, args);
    va_end(args);
    return status;
}

void free_norm_results(NormResult *results, int norm_count) {
    if (results != NULL) {
        for (int i = 0; i < norm_count; i++) free((void *)results[i].max_vectors);
        free(results);
    }
}

void print_vector(const Vector *v) {
    printf("(");
    for (int i = 0; i < v->dim; i++) {
        printf("%g", v->coords[i]);
        if (i < v->dim - 1) printf(", ");
    }
    printf(")");
}

int main(void) { 
    double t_coords1[] = {1.0, -2.0};
    double t_coords2[] = {3.0, 0.0};
    Vector var_v1 = {t_coords1, 2};
    Vector var_v2 = {t_coords2, 2};
    
    double p_test = 2.0;
    double matrix_flat_test[] = {1.0, 0.0, 0.0, 1.0};
    double *matrix_rows_test[] = {&matrix_flat_test[0], &matrix_flat_test[2]};
    Matrix A_test = {matrix_rows_test, 2};

    NormResult *res_var = NULL;
    ErrorStatus status1 = find_longest_vectors(2, 2, 3, &res_var,  &var_v1, &var_v2, norm_inf, NULL, norm_p, &p_test, norm_a, &A_test);       
    if (status1 == OK) {
        const char *norm_names[] = {"Норма бесконечности (max |x_j|)", "Норма p", "Норма A"};
        for (int i = 0; i < 3; i++) {
            printf("\n--- %s ---\n", norm_names[i]);
            printf("Максимальное значение нормы: %g\n", res_var[i].max_value);
            printf("Самые длинные векторы (%d шт.):\n", res_var[i].max_count);
            for (int j = 0; j < res_var[i].max_count; j++) {
                printf("  Вектор %d: ", j + 1);
                print_vector(res_var[i].max_vectors[j]);
                printf("\n");
            }
        }
        free_norm_results(res_var, 3);
    } else {
        print_error(status1);
    }
    
    int n, v_count;
    printf("Введите размерность пространства n: ");
    if (scanf("%d", &n) != 1 || n <= 0) { print_error(ERROR_INVALID_INPUT); return 1; }

    printf("Введите количество векторов: ");
    if (scanf("%d", &v_count) != 1 || v_count <= 0) { print_error(ERROR_COUNT); return 1; }
    
    Vector *vectors = (Vector *)malloc(v_count * sizeof(Vector));
    const Vector **vec_pointers = (const Vector **)malloc(v_count * sizeof(Vector *));
    if (!vectors || !vec_pointers) { free(vectors); free(vec_pointers); print_error(ERROR_MEMORY); return 1; }
    
    for (int i = 0; i < v_count; i++) {
        vectors[i].dim = n;
        vectors[i].coords = (double *)malloc(n * sizeof(double));
        if (!vectors[i].coords) {
            for (int k = 0; k < i; k++) free(vectors[k].coords);
            free(vectors); free(vec_pointers); print_error(ERROR_MEMORY); return 1;
        }
        vec_pointers[i] = &vectors[i];
        
        printf("Вектор %d (введите %d координат):\n", i + 1, n);
        for (int j = 0; j < n; j++) {
            printf("  [%d]: ", j);
            if (scanf("%lf", &vectors[i].coords[j]) != 1) {
                for (int k = 0; k <= i; k++) free(vectors[k].coords);
                free(vectors); free(vec_pointers); print_error(ERROR_INVALID_INPUT); return 1;
            }
        }
    }
    
    double p;
    printf("\nВведите параметр p (p >= 1): ");
    if (scanf("%lf", &p) != 1 || p < 1.0) {
        for (int k = 0; k < v_count; k++) free(vectors[k].coords);
        free(vectors); free(vec_pointers); print_error(ERROR_INVALID_INPUT); return 1;
    }
    
    Matrix A;
    A.dim = n;
    A.data = (double **)malloc(n * sizeof(double *));
    double *A_flat = (double *)malloc(n * n * sizeof(double));
    if (!A.data || !A_flat) {
        free(A.data); free(A_flat);
        for (int k = 0; k < v_count; k++) free(vectors[k].coords);
        free(vectors); free(vec_pointers); print_error(ERROR_MEMORY); return 1;
    }
    for (int i = 0; i < n; i++) A.data[i] = A_flat + i * n;
    
    printf("\nВведите матрицу A (%dx%d):\n", n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("  A[%d][%d]: ", i, j);
            if (scanf("%lf", &A.data[i][j]) != 1) {
                free(A.data); free(A_flat);
                for (int k = 0; k < v_count; k++) free(vectors[k].coords);
                free(vectors); free(vec_pointers); print_error(ERROR_INVALID_INPUT); return 1;
            }
        }
    }
    
    NormFunc funcs[] = {norm_inf, norm_p, norm_a};
    const void *contexts[] = {NULL, &p, &A};
    int norm_count = 3;
    NormResult *results = NULL;
    
    ErrorStatus status2 = find_longest_vectors_core(n, v_count, norm_count, vec_pointers, funcs, contexts, &results);
    
    if (status2 != OK) {
        print_error(status2);
    } else {
        const char *norm_names[] = {"Норма бесконечности", "Норма p", "Норма A"};
        for (int i = 0; i < norm_count; i++) {
            printf("\n--- %s ---\n", norm_names[i]);
            printf("Максимальное значение: %g\n", results[i].max_value);
            printf("Самые длинные векторы (%d шт.):\n", results[i].max_count);
            for (int j = 0; j < results[i].max_count; j++) {
                printf("  Вектор %d: ", j + 1);
                print_vector(results[i].max_vectors[j]);
                printf("\n");
            }
        }
        free_norm_results(results, norm_count);
    }
    
    free(A.data); free(A_flat);
    for (int i = 0; i < v_count; i++) free(vectors[i].coords);
    free(vectors); free(vec_pointers);
    
    return 0;
}