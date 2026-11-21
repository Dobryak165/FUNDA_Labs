#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>

typedef enum {
    OK = 0,
    END_OF_FILE,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_INPUT,
    ERROR_MEMORY,
    ERROR_OPEN,
    ERROR_READ,
    ERROR_WRITE,
    ERROR_OVERFLOW,
    ERROR_NOT_FOUND,
    ERROR_EMPTY
} ErrorStatus;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:
            printf("Нулевой указатель\n");
            break;
        case ERROR_COUNT:
            printf("Неверное количество аргументов\n");
            break;
        case ERROR_INPUT:
            printf("Некорректные входные данные\n");
            break;
        case ERROR_MEMORY:
            printf("Не удалось выделить память\n");
            break;
        case ERROR_OPEN:
            printf("Не удалось открыть файл\n");
            break;
        case ERROR_READ:
            printf("Ошибка чтения файла\n");
            break;
        case ERROR_WRITE:
            printf("Ошибка записи в поток\n");
            break;
        case ERROR_OVERFLOW:
            printf("Слишком большое числовое значение\n");
            break;
        case ERROR_NOT_FOUND:
            printf("Запись не найдена\n");
            break;
        case ERROR_EMPTY:
            printf("Коллекция пуста\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
            break;
    }
}

ErrorStatus read_line(FILE *input, char **result) {
    if (input == NULL || result == NULL) return ERROR_NULL;
    *result = NULL;
    size_t capacity = 128;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL) return ERROR_MEMORY;
    int ch = 0;
    ErrorStatus status = OK;
    while (status == OK && (ch = fgetc(input)) != EOF && ch != '\n') {
        if (ch == '\0') status = ERROR_INPUT;
        else {
            if (length == capacity - 1) {
                if (capacity > SIZE_MAX / 2) status = ERROR_OVERFLOW;
                else {
                    char *temp = realloc(buffer, capacity * 2);
                    if (temp == NULL) status = ERROR_MEMORY;
                    else {
                        buffer = temp;
                        capacity *= 2;
                    }
                }
            }
            if (status == OK) buffer[length++] = (char)ch;
        }
    }
    if (status == OK && ch == EOF && !feof(input)) status = ERROR_READ;
    if (status == OK && ch == EOF && length == 0) status = END_OF_FILE;
    if (status != OK) {
        free(buffer);
        return status;
    }
    if (length > 0 && buffer[length - 1] == '\r') length--;
    buffer[length] = '\0';
    *result = buffer;
    return OK;
}

ErrorStatus parse_size(const char *text, size_t *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    if (*text == '\0') return ERROR_INPUT;
    size_t value = 0;
    while (*text != '\0') {
        if (*text < '0' || *text > '9') return ERROR_INPUT;
        size_t digit = (size_t)(*text - '0');
        if (value > (SIZE_MAX - digit) / 10) return ERROR_OVERFLOW;
        value = value * 10 + digit;
        text++;
    }
    *result = value;
    return OK;
}

ErrorStatus copy_text(const char *source, char **result) {
    if (source == NULL || result == NULL) return ERROR_NULL;
    *result = NULL;
    size_t length = strlen(source);
    if (length == SIZE_MAX) return ERROR_OVERFLOW;
    char *copy = malloc(length + 1);
    if (copy == NULL) return ERROR_MEMORY;
    memcpy(copy, source, length + 1);
    *result = copy;
    return OK;
}

ErrorStatus split_fields(char *line, char **fields, size_t capacity, size_t *count) {
    if (line == NULL || fields == NULL || count == NULL) return ERROR_NULL;
    *count = 0;
    char *cursor = line;
    while (*cursor != '\0') {
        while (isspace((unsigned char)*cursor)) cursor++;
        if (*cursor == '\0') break;
        if (*count == capacity) return ERROR_INPUT;
        fields[(*count)++] = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) cursor++;
        if (*cursor != '\0') *cursor++ = '\0';
    }
    return OK;
}

ErrorStatus ask_line(const char *prompt, char **result) {
    if (prompt == NULL || result == NULL) return ERROR_NULL;
    if (printf("%s", prompt) < 0 || fflush(stdout) != 0) return ERROR_WRITE;
    return read_line(stdin, result);
}

typedef struct {
    int id;
    char *name;
    char *surname;
    char *group;
    unsigned char *grades;
} Student;

typedef struct {
    Student *items;
    size_t count;
    size_t capacity;
} StudentArray;

ErrorStatus valid_latin(const char *text) {
    if (text == NULL) return ERROR_NULL;
    if (*text == '\0') return ERROR_INPUT;
    for (; *text != '\0'; text++) {
        if (!((*text >= 'A' && *text <= 'Z') || (*text >= 'a' && *text <= 'z'))) return ERROR_INPUT;
    }
    return OK;
}

ErrorStatus destroy_student(Student *student) {
    if (student == NULL) return ERROR_NULL;
    free(student->name);
    free(student->surname);
    free(student->group);
    free(student->grades);
    memset(student, 0, sizeof(Student));
    return OK;
}

ErrorStatus destroy_students(StudentArray *array) {
    if (array == NULL) return ERROR_NULL;
    for (size_t i = 0; i < array->count; i++) destroy_student(&array->items[i]);
    free(array->items);
    memset(array, 0, sizeof(StudentArray));
    return OK;
}

ErrorStatus parse_student(char *line, Student *result) {
    if (line == NULL || result == NULL) return ERROR_NULL;
    char *fields[9];
    size_t count = 0;
    ErrorStatus status = split_fields(line, fields, 9, &count);
    if (status == OK && count != 9) status = ERROR_INPUT;
    size_t id = 0;
    Student student = {0};
    if (status == OK) status = parse_size(fields[0], &id);
    if (status == OK && id > INT_MAX) status = ERROR_OVERFLOW;
    if (status == OK) status = valid_latin(fields[1]);
    if (status == OK) status = valid_latin(fields[2]);
    if (status == OK) status = copy_text(fields[1], &student.name);
    if (status == OK) status = copy_text(fields[2], &student.surname);
    if (status == OK) status = copy_text(fields[3], &student.group);
    if (status == OK) {
        student.id = (int)id;
        student.grades = malloc(5 * sizeof(unsigned char));
        if (student.grades == NULL) status = ERROR_MEMORY;
    }
    for (size_t i = 0; i < 5 && status == OK; i++) {
        size_t grade = 0;
        status = parse_size(fields[4 + i], &grade);
        if (status == OK && grade > UCHAR_MAX) status = ERROR_INPUT;
        if (status == OK) student.grades[i] = (unsigned char)grade;
    }
    if (status != OK) {
        destroy_student(&student);
        return status;
    }
    *result = student;
    return OK;
}

ErrorStatus append_student(StudentArray *array, Student *student) {
    if (array == NULL || student == NULL) return ERROR_NULL;
    if (array->count == array->capacity) {
        size_t capacity = array->capacity == 0 ? 8 : array->capacity;
        if (capacity > SIZE_MAX / 2 / sizeof(Student)) return ERROR_OVERFLOW;
        capacity *= 2;
        Student *temp = realloc(array->items, capacity * sizeof(Student));
        if (temp == NULL) return ERROR_MEMORY;
        array->items = temp;
        array->capacity = capacity;
    }
    array->items[array->count++] = *student;
    memset(student, 0, sizeof(Student));
    return OK;
}

ErrorStatus load_students(FILE *input, StudentArray *result) {
    if (input == NULL || result == NULL) return ERROR_NULL;
    StudentArray array = {0};
    ErrorStatus status = OK;
    while (status == OK) {
        char *line = NULL;
        status = read_line(input, &line);
        if (status == END_OF_FILE) break;
        Student student = {0};
        if (status == OK) status = parse_student(line, &student);
        if (status == OK) status = append_student(&array, &student);
        destroy_student(&student);
        free(line);
    }
    if (status == END_OF_FILE) status = OK;
    if (status != OK) {
        destroy_students(&array);
        return status;
    }
    *result = array;
    return OK;
}

ErrorStatus student_average(const Student *student, double *result) {
    if (student == NULL || student->grades == NULL || result == NULL) return ERROR_NULL;
    unsigned int sum = 0;
    for (size_t i = 0; i < 5; i++) sum += student->grades[i];
    *result = sum / 5.0;
    return OK;
}

int compare_student_id(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Student *a = first;
    const Student *b = second;
    return (a->id > b->id) - (a->id < b->id);
}

int compare_student_surname(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Student *a = first;
    const Student *b = second;
    return strcmp(a->surname, b->surname);
}

int compare_student_name(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Student *a = first;
    const Student *b = second;
    return strcmp(a->name, b->name);
}

int compare_student_group(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Student *a = first;
    const Student *b = second;
    return strcmp(a->group, b->group);
}

ErrorStatus sort_students(StudentArray *array, int (*compare)(const void *, const void *)) {
    if (array == NULL || compare == NULL) return ERROR_NULL;
    if (array->count != 0 && array->items == NULL) return ERROR_NULL;
    if (array->count > 1) qsort(array->items, array->count, sizeof(Student), compare);
    return OK;
}

ErrorStatus find_students(const StudentArray *array, size_t field, const char *value, size_t **indices, size_t *count) {
    if (array == NULL || value == NULL || indices == NULL || count == NULL) return ERROR_NULL;
    *indices = NULL;
    *count = 0;
    if (field < 1 || field > 4 || *value == '\0') return ERROR_INPUT;
    size_t id = 0;
    ErrorStatus status = OK;
    if (field == 1) {
        status = parse_size(value, &id);
        if (status == OK && id > INT_MAX) status = ERROR_OVERFLOW;
    } else if (field == 2 || field == 3) status = valid_latin(value);
    if (status != OK) return status;
    if (array->count == 0) return OK;
    if (array->items == NULL) return ERROR_NULL;
    if (array->count > SIZE_MAX / sizeof(size_t)) return ERROR_OVERFLOW;
    size_t *found = malloc(array->count * sizeof(size_t));
    if (found == NULL) return ERROR_MEMORY;
    for (size_t i = 0; i < array->count; i++) {
        const Student *student = &array->items[i];
        int matches = 0;
        if (field == 1) matches = (size_t)student->id == id;
        if (field == 2) matches = strcmp(student->surname, value) == 0;
        if (field == 3) matches = strcmp(student->name, value) == 0;
        if (field == 4) matches = strcmp(student->group, value) == 0;
        if (matches) found[(*count)++] = i;
    }
    *indices = found;
    return OK;
}

ErrorStatus find_above_average(const StudentArray *array, double epsilon, size_t **indices, size_t *count, double *mean) {
    if (array == NULL || indices == NULL || count == NULL || mean == NULL) return ERROR_NULL;
    if (!isfinite(epsilon) || epsilon <= 0.0) return ERROR_INPUT;
    *indices = NULL;
    *count = 0;
    *mean = 0.0;
    if (array->count == 0) return ERROR_EMPTY;
    if (array->items == NULL) return ERROR_NULL;
    if (array->count > SIZE_MAX / sizeof(size_t)) return ERROR_OVERFLOW;
    double total = 0.0;
    for (size_t i = 0; i < array->count; i++) {
        double average = 0.0;
        ErrorStatus status = student_average(&array->items[i], &average);
        if (status != OK) return status;
        total += average;
    }
    *mean = total / array->count;
    size_t *found = malloc(array->count * sizeof(size_t));
    if (found == NULL) return ERROR_MEMORY;
    for (size_t i = 0; i < array->count; i++) {
        double average = 0.0;
        student_average(&array->items[i], &average);
        if (average - *mean > epsilon) found[(*count)++] = i;
    }
    *indices = found;
    return OK;
}

ErrorStatus print_student(FILE *output, const Student *student) {
    if (output == NULL || student == NULL) return ERROR_NULL;
    if (student->name == NULL || student->surname == NULL || student->group == NULL) return ERROR_NULL;
    double average = 0.0;
    ErrorStatus status = student_average(student, &average);
    if (status != OK) return status;
    if (fprintf(output, "%d %s %s %s | средний балл %.6f\n", student->id, student->surname, student->name, student->group, average) < 0) return ERROR_WRITE;
    return OK;
}

ErrorStatus export_above(FILE *trace, const StudentArray *array, double epsilon) {
    if (trace == NULL || array == NULL) return ERROR_NULL;
    size_t *indices = NULL;
    size_t count = 0;
    double mean = 0.0;
    ErrorStatus status = find_above_average(array, epsilon, &indices, &count, &mean);
    if (status == OK && fprintf(trace, "Выше общего среднего %.6f:\n", mean) < 0) status = ERROR_WRITE;
    for (size_t i = 0; i < count && status == OK; i++) {
        const Student *student = &array->items[indices[i]];
        if (fprintf(trace, "%s %s\n", student->surname, student->name) < 0) status = ERROR_WRITE;
    }
    free(indices);
    if (status == OK && fflush(trace) != 0) status = ERROR_WRITE;
    return status;
}

ErrorStatus student_dialog(StudentArray *array, FILE *trace, double epsilon) {
    if (array == NULL || trace == NULL) return ERROR_NULL;
    ErrorStatus status = OK;
    while (status == OK) {
        char *line = NULL;
        status = ask_line("\n0 Выход | 1 Все | 2 Поиск | 3 Сортировка | 4 Запись по id | 5 Выше среднего\n> ", &line);
        if (status == END_OF_FILE) return OK;
        size_t command = 0;
        if (status == OK) status = parse_size(line, &command);
        free(line);
        if (status != OK) {
            if (status == ERROR_INPUT || status == ERROR_OVERFLOW) {
                print_error(status);
                status = OK;
                continue;
            }
            return status;
        }
        if (command == 0) return OK;
        if (command == 1) {
            for (size_t i = 0; i < array->count && status == OK; i++) status = print_student(stdout, &array->items[i]);
        } else if (command == 2 || command == 3 || command == 4) {
            size_t field = 1;
            if (command != 4) {
                line = NULL;
                status = ask_line("Поле: 1 id, 2 фамилия, 3 имя, 4 группа: ", &line);
                if (status == OK) status = parse_size(line, &field);
                free(line);
                if (status == OK && (field < 1 || field > 4)) status = ERROR_INPUT;
            }
            if (status == OK && command == 3) {
                int (*comparators[4])(const void *, const void *) = {compare_student_id, compare_student_surname, compare_student_name, compare_student_group};
                status = sort_students(array, comparators[field - 1]);
            } else if (status == OK) {
                line = NULL;
                status = ask_line("Значение: ", &line);
                size_t *indices = NULL;
                size_t count = 0;
                if (status == OK) status = find_students(array, field, line, &indices, &count);
                if (status == OK && count == 0) status = ERROR_NOT_FOUND;
                for (size_t i = 0; i < count && status == OK; i++) status = print_student(command == 4 ? trace : stdout, &array->items[indices[i]]);
                if (status == OK && command == 4 && fflush(trace) != 0) status = ERROR_WRITE;
                free(indices);
                free(line);
            }
        } else if (command == 5) status = export_above(trace, array, epsilon);
        else status = ERROR_INPUT;
        if (status == END_OF_FILE) return OK;
        if (status == ERROR_INPUT || status == ERROR_OVERFLOW || status == ERROR_NOT_FOUND || status == ERROR_EMPTY) {
            print_error(status);
            status = OK;
        }
    }
    return status;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_error(ERROR_COUNT);
        printf("Использование: ./Task5 <студенты> <трассировка>\n");
        return 1;
    }
    if (*argv[1] == '\0' || *argv[2] == '\0' || strcmp(argv[1], argv[2]) == 0) {
        print_error(ERROR_INPUT);
        return 1;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        print_error(ERROR_OPEN);
        return 1;
    }
    StudentArray array = {0};
    ErrorStatus status = load_students(input, &array);
    if (fclose(input) != 0 && status == OK) status = ERROR_READ;
    FILE *trace = NULL;
    if (status == OK) {
        trace = fopen(argv[2], "wb");
        if (trace == NULL) status = ERROR_OPEN;
    }
    if (status == OK) status = student_dialog(&array, trace, 1e-9);
    if (trace != NULL && fclose(trace) != 0 && status == OK) status = ERROR_WRITE;
    destroy_students(&array);
    if (status == OK && fflush(stdout) != 0) status = ERROR_WRITE;
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
