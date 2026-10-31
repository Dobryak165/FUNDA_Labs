#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef enum {
    OK = 0,
    ERROR_INVALID_INPUT,
    ERROR_MEMORY,
    ERROR_ARGC,
    ERROR_FLAG,
    ERROR_FILE_OPEN,
    ERROR_FILE_WRITE,
    ERROR_FORMAT
} ErrorStatus;

typedef struct {
    unsigned int id;
    char *name;
    char *surname;
    double salary;
} Employee;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_INVALID_INPUT: 
            printf("Неверные входные данные\n"); 
            break;
        case ERROR_MEMORY: 
            printf("Не удалось выделить память\n"); 
            break;
        case ERROR_ARGC: 
            printf("Неверное количество аргументов\n"); 
            break;
        case ERROR_FLAG: 
            printf("Неверный формат флага\n"); 
            break;
        case ERROR_FILE_OPEN: 
            printf("Не удалось открыть файл\n"); 
            break;
        case ERROR_FILE_WRITE: 
            printf("Не удалось записать в файл\n"); 
            break;
        case ERROR_FORMAT: 
            printf("Неверный формат данных\n"); 
            break;
        default: 
            printf("Неизвестная ошибка\n"); 
            break;
    }
}

ErrorStatus validate_args(int argc, char *argv[]) {
    if (argc != 4) return ERROR_ARGC;
    const char *flag = argv[2];
    if ((flag[0] != '-' && flag[0] != '/') || flag[2] != '\0') return ERROR_FLAG;
    char action = flag[1];
    if (action != 'a' && action != 'd') return ERROR_FLAG;
    if (strcmp(argv[1], argv[3]) == 0) return ERROR_INVALID_INPUT; 
    return OK;
}

ErrorStatus parse_unsigned_int(const char *str, unsigned int *result) {
    if (str == NULL || result == NULL || str[0] == '\0') return ERROR_INVALID_INPUT;
    unsigned int value = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return ERROR_INVALID_INPUT;
        unsigned int digit = str[i] - '0';
        if (value > (UINT_MAX - digit) / 10) return ERROR_INVALID_INPUT;
        value = value * 10 + digit;
    }
    *result = value;
    return OK;
}

ErrorStatus parse_double(const char *str, double *result) {
    if (str == NULL || result == NULL || str[0] == '\0') return ERROR_INVALID_INPUT;
    double value = 0.0;
    double fraction = 1.0;
    int has_dot = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (c == '.' || c == ',') {
            if (has_dot) return ERROR_INVALID_INPUT;
            has_dot = 1;
        } else if (c >= '0' && c <= '9') {
            if (!has_dot) value = value * 10.0 + (c - '0');
            else {
                fraction /= 10.0;
                value += (c - '0') * fraction;
            }
        } else {
            return ERROR_INVALID_INPUT;
        }
    }
    *result = value;
    return OK;
}

int is_valid_latin(const char *s) {
    if (s == NULL || s[0] == '\0') return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return 0;
    }
    return 1;
}

int compare_doubles(double a, double b, double epsilon) {
    if (a - b > epsilon) return 1;
    if (b - a > epsilon) return -1;
    return 0;
}

int compare_asc(const void *a, const void *b) {
    const Employee *e1 = (const Employee *)a;
    const Employee *e2 = (const Employee *)b;
    double eps = 1e-6;

    int result = compare_doubles(e1->salary, e2->salary, eps);
    if (result != 0) return result;

    int cmp_surname = strcmp(e1->surname, e2->surname);
    if (cmp_surname != 0) return cmp_surname;

    int cmp_name = strcmp(e1->name, e2->name);
    if (cmp_name != 0) return cmp_name;

    if (e1->id < e2->id) return -1;
    if (e1->id > e2->id) return 1;
    return 0;
}

int compare_desc(const void *a, const void *b) {
    return -compare_asc(a, b);
}

ErrorStatus parse_employee(char *line, Employee *emp) {
    char *id_str = strtok(line, " \t\r\n");
    char *name_str = strtok(NULL, " \t\r\n");
    char *surname_str = strtok(NULL, " \t\r\n");
    char *salary_str = strtok(NULL, " \t\r\n");

    if (!id_str || !name_str || !surname_str || !salary_str) return ERROR_FORMAT;
    if (strtok(NULL, " \t\r\n") != NULL) return ERROR_FORMAT;

    unsigned int id;
    if (parse_unsigned_int(id_str, &id) != OK) return ERROR_FORMAT;
    if (!is_valid_latin(name_str)) return ERROR_FORMAT;
    if (!is_valid_latin(surname_str)) return ERROR_FORMAT;
    
    double salary;
    if (parse_double(salary_str, &salary) != OK) return ERROR_FORMAT;

    emp->id = id;

    emp->name = strdup(name_str); 
    if (emp->name == NULL) return ERROR_MEMORY;
    
    emp->surname = strdup(surname_str);
    if (emp->surname == NULL) {
        free(emp->name);
        return ERROR_MEMORY;
    }
    
    emp->salary = salary;
    return OK;
}

ErrorStatus load_from_file(const char *filename, Employee **employees, int *count) {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) return ERROR_FILE_OPEN;

    Employee *arr = NULL;
    int cnt = 0;
    int cap = 0;
    char line[4096];

    while (fgets(line, sizeof(line), fin) != NULL) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\r' || *p == '\0') continue;

        Employee emp;
        ErrorStatus status = parse_employee(p, &emp);
        if (status != OK) {
            for (int i = 0; i < cnt; i++) {
                free(arr[i].name);
                free(arr[i].surname);
            }
            free(arr);
            fclose(fin);
            return status;
        }

        if (cnt == cap) {
            cap = (cap == 0) ? 10 : cap * 2;
            Employee *new_arr = realloc(arr, cap * sizeof(Employee));
            if (new_arr == NULL) {
                free(emp.name);
                free(emp.surname);
                for (int i = 0; i < cnt; i++) {
                    free(arr[i].name);
                    free(arr[i].surname);
                }
                free(arr);
                fclose(fin);
                return ERROR_MEMORY;
            }
            arr = new_arr;
        }
        arr[cnt++] = emp;
    }

    fclose(fin);
    *employees = arr;
    *count = cnt;
    return OK;
}

ErrorStatus save_to_file(const char *filename, const Employee *employees, int count) {
    FILE *fout = fopen(filename, "w");
    if (fout == NULL) return ERROR_FILE_OPEN;

    for (int i = 0; i < count; i++) {
        if (fprintf(fout, "%u %s %s %g\n", employees[i].id, employees[i].name, employees[i].surname, employees[i].salary) < 0) {
            fclose(fout);
            return ERROR_FILE_WRITE;
        }
    }

    if (fclose(fout) != 0) return ERROR_FILE_WRITE;
    return OK;
}

int main(int argc, char *argv[]) {

    ErrorStatus status = validate_args(argc, argv);
    if (status != OK) {
        print_error(status);
        return status;
    }

    const char *input_file = argv[1];
    char action = argv[2][1];
    const char *output_file = argv[3];

    Employee *employees = NULL;
    int count = 0;

    status = load_from_file(input_file, &employees, &count);
    if (status != OK) {
        print_error(status);
        return status;
    }

    if (action == 'a') {
        qsort(employees, count, sizeof(Employee), compare_asc);
    } else {
        qsort(employees, count, sizeof(Employee), compare_desc);
    }

    status = save_to_file(output_file, employees, count);

    for (int i = 0; i < count; i++) {
        free(employees[i].name);
        free(employees[i].surname);
    }
    free(employees);

    if (status != OK) {
        print_error(status);
        return status;
    }

    printf("Успешно обработано записей: %d\n", count);
    return OK;
}