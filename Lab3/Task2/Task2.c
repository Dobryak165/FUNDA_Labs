#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef enum {
    OK = 0,
    END_OF_FILE,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_INPUT,
    ERROR_MEMORY,
    ERROR_READ,
    ERROR_WRITE,
    ERROR_OVERFLOW,
    ERROR_DUPLICATE
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
        case ERROR_READ:
            printf("Ошибка чтения файла\n");
            break;
        case ERROR_WRITE:
            printf("Ошибка записи в поток\n");
            break;
        case ERROR_OVERFLOW:
            printf("Слишком большое числовое значение\n");
            break;
        case ERROR_DUPLICATE:
            printf("Идентификатор уже существует\n");
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

typedef struct {
    size_t id;
    int start;
    int finish;
} Report;

ErrorStatus parse_clock(const char *text, int *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    if (strlen(text) != 5 || text[2] != ':') return ERROR_INPUT;
    for (size_t i = 0; i < 5; i++) {
        if (i != 2 && (text[i] < '0' || text[i] > '9')) return ERROR_INPUT;
    }
    int hour = (text[0] - '0') * 10 + text[1] - '0';
    int minute = (text[3] - '0') * 10 + text[4] - '0';
    if (hour > 23 || minute > 59) return ERROR_INPUT;
    *result = hour * 60 + minute;
    return OK;
}

int compare_id(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Report *a = first;
    const Report *b = second;
    return (a->id > b->id) - (a->id < b->id);
}

int compare_finish(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Report *a = first;
    const Report *b = second;
    if (a->finish != b->finish) return (a->finish > b->finish) - (a->finish < b->finish);
    return (a->start > b->start) - (a->start < b->start);
}

ErrorStatus maximum_reports(const Report *reports, size_t count, size_t halls, size_t *result) {
    if (reports == NULL || result == NULL) return ERROR_NULL;
    if (count == 0 || halls == 0) return ERROR_INPUT;
    if (count > SIZE_MAX / sizeof(Report)) return ERROR_OVERFLOW;
    size_t used_halls = halls < count ? halls : count;
    if (used_halls > SIZE_MAX / sizeof(int)) return ERROR_OVERFLOW;
    Report *sorted = malloc(count * sizeof(Report));
    int *finish = calloc(used_halls, sizeof(int));
    if (sorted == NULL || finish == NULL) {
        free(sorted);
        free(finish);
        return ERROR_MEMORY;
    }
    ErrorStatus status = OK;
    memcpy(sorted, reports, count * sizeof(Report));
    for (size_t i = 0; i < count; i++) {
        if (sorted[i].id == 0 || sorted[i].start < 0 || sorted[i].finish >= 1440 || sorted[i].start >= sorted[i].finish) status = ERROR_INPUT;
    }
    qsort(sorted, count, sizeof(Report), compare_id);
    for (size_t i = 1; i < count; i++) {
        if (sorted[i - 1].id == sorted[i].id) status = ERROR_DUPLICATE;
    }
    size_t accepted = 0;
    if (status == OK) {
        qsort(sorted, count, sizeof(Report), compare_finish);
        for (size_t i = 0; i < count; i++) {
            size_t best = used_halls;
            for (size_t j = 0; j < used_halls; j++) {
                if (finish[j] <= sorted[i].start && (best == used_halls || finish[j] > finish[best])) best = j;
            }
            if (best != used_halls) {
                finish[best] = sorted[i].finish;
                accepted++;
            }
        }
        *result = accepted;
    }
    free(sorted);
    free(finish);
    return status;
}

ErrorStatus read_reports(FILE *input, Report **result, size_t *count, size_t *halls) {
    if (input == NULL || result == NULL || count == NULL || halls == NULL) return ERROR_NULL;
    *result = NULL;
    char *line = NULL;
    ErrorStatus status = read_line(input, &line);
    char *fields[3];
    size_t fields_count = 0;
    size_t n = 0;
    size_t k = 0;
    if (status == OK) status = split_fields(line, fields, 2, &fields_count);
    if (status == OK && fields_count != 2) status = ERROR_INPUT;
    if (status == OK) status = parse_size(fields[0], &n);
    if (status == OK) status = parse_size(fields[1], &k);
    free(line);
    if (status == END_OF_FILE) status = ERROR_INPUT;
    if (status != OK) return status;
    if (n == 0 || k == 0) return ERROR_INPUT;
    if (n > SIZE_MAX / sizeof(Report)) return ERROR_OVERFLOW;
    Report *reports = malloc(n * sizeof(Report));
    if (reports == NULL) return ERROR_MEMORY;
    for (size_t i = 0; i < n && status == OK; i++) {
        line = NULL;
        status = read_line(input, &line);
        if (status == END_OF_FILE) status = ERROR_INPUT;
        if (status == OK) status = split_fields(line, fields, 3, &fields_count);
        if (status == OK && fields_count != 3) status = ERROR_INPUT;
        if (status == OK) status = parse_size(fields[0], &reports[i].id);
        if (status == OK) status = parse_clock(fields[1], &reports[i].start);
        if (status == OK) status = parse_clock(fields[2], &reports[i].finish);
        free(line);
    }
    if (status != OK) {
        free(reports);
        return status;
    }
    *result = reports;
    *count = n;
    *halls = k;
    return OK;
}

int main(int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        print_error(ERROR_COUNT);
        return 1;
    }
    Report *reports = NULL;
    size_t count = 0;
    size_t halls = 0;
    size_t result = 0;
    ErrorStatus status = read_reports(stdin, &reports, &count, &halls);
    if (status == OK) status = maximum_reports(reports, count, halls, &result);
    if (status == OK && printf("%zu\n", result) < 0) status = ERROR_WRITE;
    if (status == OK && fflush(stdout) != 0) status = ERROR_WRITE;
    free(reports);
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
