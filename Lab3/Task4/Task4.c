#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

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
    ERROR_NOT_FOUND,
    ERROR_DUPLICATE,
    ERROR_TIME
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
        case ERROR_NOT_FOUND:
            printf("Запись не найдена\n");
            break;
        case ERROR_DUPLICATE:
            printf("Идентификатор уже существует\n");
            break;
        case ERROR_TIME:
            printf("Некорректная дата или недоступно системное время\n");
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

ErrorStatus ask_line(const char *prompt, char **result) {
    if (prompt == NULL || result == NULL) return ERROR_NULL;
    if (printf("%s", prompt) < 0 || fflush(stdout) != 0) return ERROR_WRITE;
    return read_line(stdin, result);
}

typedef struct {
    char *data;
    int length;
} String;

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} DateTime;

typedef struct {
    String city;
    String street;
    unsigned int house;
    String building;
    unsigned int apartment;
    String index;
} Address;

typedef struct {
    Address recipient;
    double weight;
    String id;
    String created;
    String delivery;
    String deadline;
    DateTime created_at;
    DateTime deadline_at;
} Mail;

typedef struct {
    Address *address;
    Mail *items;
    size_t count;
    size_t capacity;
} Post;

ErrorStatus string_create(const char *text, String *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    size_t length = strlen(text);
    if (length > INT_MAX) return ERROR_OVERFLOW;
    char *data = malloc(length + 1);
    if (data == NULL) return ERROR_MEMORY;
    memcpy(data, text, length + 1);
    result->data = data;
    result->length = (int)length;
    return OK;
}

ErrorStatus string_destroy(String *string) {
    if (string == NULL) return ERROR_NULL;
    free(string->data);
    string->data = NULL;
    string->length = 0;
    return OK;
}

ErrorStatus string_valid(const String *string) {
    if (string == NULL || string->data == NULL) return ERROR_NULL;
    if (string->length < 0 || strlen(string->data) != (size_t)string->length) return ERROR_INPUT;
    return OK;
}

ErrorStatus string_order(const String *first, const String *second, int *result) {
    if (result == NULL) return ERROR_NULL;
    ErrorStatus status = string_valid(first);
    if (status == OK) status = string_valid(second);
    if (status != OK) return status;
    if (first->length != second->length) *result = (first->length > second->length) - (first->length < second->length);
    else {
        int order = strcmp(first->data, second->data);
        *result = (order > 0) - (order < 0);
    }
    return OK;
}

ErrorStatus string_equal(const String *first, const String *second, int *result) {
    if (result == NULL) return ERROR_NULL;
    ErrorStatus status = string_valid(first);
    if (status == OK) status = string_valid(second);
    if (status != OK) return status;
    *result = strcmp(first->data, second->data) == 0;
    return OK;
}

ErrorStatus string_copy(const String *source, String *destination) {
    if (destination == NULL) return ERROR_NULL;
    ErrorStatus status = string_valid(source);
    if (status != OK) return status;
    String copy = {0};
    status = string_create(source->data, &copy);
    if (status != OK) return status;
    string_destroy(destination);
    *destination = copy;
    return OK;
}

ErrorStatus string_clone(const String *source, String **result) {
    if (result == NULL) return ERROR_NULL;
    *result = NULL;
    ErrorStatus status = string_valid(source);
    if (status != OK) return status;
    String *copy = calloc(1, sizeof(String));
    if (copy == NULL) return ERROR_MEMORY;
    status = string_create(source->data, copy);
    if (status != OK) {
        free(copy);
        return status;
    }
    *result = copy;
    return OK;
}

ErrorStatus string_concat(String *first, const String *second) {
    ErrorStatus status = string_valid(first);
    if (status == OK) status = string_valid(second);
    if (status != OK) return status;
    if (first->length > INT_MAX - second->length) return ERROR_OVERFLOW;
    size_t length = (size_t)first->length + (size_t)second->length;
    char *joined = malloc(length + 1);
    if (joined == NULL) return ERROR_MEMORY;
    memcpy(joined, first->data, (size_t)first->length);
    memcpy(joined + first->length, second->data, (size_t)second->length + 1);
    free(first->data);
    first->data = joined;
    first->length = (int)length;
    return OK;
}

ErrorStatus string_demo(FILE *output) {
    if (output == NULL) return ERROR_NULL;
    String first = {0};
    String second = {0};
    String existing = {0};
    String *new_copy = NULL;
    int order = 0;
    int equal = 0;
    ErrorStatus status = string_create("Hello", &first);
    if (status == OK) status = string_create("World", &second);
    if (status == OK) status = string_create("old", &existing);
    if (status == OK) status = string_copy(&first, &existing);
    if (status == OK) status = string_clone(&first, &new_copy);
    if (status == OK) status = string_order(&first, &second, &order);
    if (status == OK) status = string_equal(&first, &existing, &equal);
    if (status == OK) status = string_concat(&first, &second);
    if (status == OK && fprintf(output, "Копия: %s | Новая копия: %s | Порядок: %d | Равны: %d | Конкатенация: %s\n", existing.data, new_copy->data, order, equal, first.data) < 0) status = ERROR_WRITE;
    string_destroy(&first);
    string_destroy(&second);
    string_destroy(&existing);
    if (new_copy != NULL) string_destroy(new_copy);
    free(new_copy);
    return status;
}

ErrorStatus split_pipes(char *line, char **fields, size_t expected) {
    if (line == NULL || fields == NULL) return ERROR_NULL;
    if (expected == 0) return ERROR_INPUT;
    size_t count = 0;
    char *start = line;
    while (1) {
        if (count == expected) return ERROR_INPUT;
        char *separator = strchr(start, '|');
        if (separator != NULL) *separator = '\0';
        while (isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start);
        while (end > start && isspace((unsigned char)end[-1])) *--end = '\0';
        fields[count++] = start;
        if (separator == NULL) break;
        start = separator + 1;
    }
    return count == expected ? OK : ERROR_INPUT;
}

ErrorStatus digits_exact(const char *text, size_t length) {
    if (text == NULL) return ERROR_NULL;
    if (strlen(text) != length) return ERROR_INPUT;
    for (size_t i = 0; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') return ERROR_INPUT;
    }
    return OK;
}

ErrorStatus parse_datetime(const char *text, DateTime *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    if (strlen(text) != 19) return ERROR_TIME;
    for (size_t i = 0; i < 19; i++) {
        if (i == 2 || i == 5 || i == 13 || i == 16) {
            if (text[i] != ':') return ERROR_TIME;
        } else if (i == 10) {
            if (text[i] != ' ') return ERROR_TIME;
        } else if (text[i] < '0' || text[i] > '9') return ERROR_TIME;
    }
    DateTime value;
    value.day = (text[0] - '0') * 10 + text[1] - '0';
    value.month = (text[3] - '0') * 10 + text[4] - '0';
    value.year = (text[6] - '0') * 1000 + (text[7] - '0') * 100 + (text[8] - '0') * 10 + text[9] - '0';
    value.hour = (text[11] - '0') * 10 + text[12] - '0';
    value.minute = (text[14] - '0') * 10 + text[15] - '0';
    value.second = (text[17] - '0') * 10 + text[18] - '0';
    if (value.year == 0 || value.month < 1 || value.month > 12 || value.hour > 23 || value.minute > 59 || value.second > 59) return ERROR_TIME;
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (value.year % 400 == 0 || (value.year % 4 == 0 && value.year % 100 != 0)) days[1] = 29;
    if (value.day < 1 || value.day > days[value.month - 1]) return ERROR_TIME;
    *result = value;
    return OK;
}

ErrorStatus compare_datetime(const DateTime *first, const DateTime *second, int *result) {
    if (first == NULL || second == NULL || result == NULL) return ERROR_NULL;
    const int a[] = {first->year, first->month, first->day, first->hour, first->minute, first->second};
    const int b[] = {second->year, second->month, second->day, second->hour, second->minute, second->second};
    *result = 0;
    for (size_t i = 0; i < 6; i++) {
        if (a[i] != b[i]) {
            *result = (a[i] > b[i]) - (a[i] < b[i]);
            break;
        }
    }
    return OK;
}

ErrorStatus current_datetime(DateTime *result) {
    if (result == NULL) return ERROR_NULL;
    time_t now = time(NULL);
    if (now == (time_t)-1) return ERROR_TIME;
    const struct tm *calendar = localtime(&now);
    if (calendar == NULL) return ERROR_TIME;
    char buffer[20];
    if (strftime(buffer, sizeof(buffer), "%d:%m:%Y %H:%M:%S", calendar) != 19) return ERROR_TIME;
    return parse_datetime(buffer, result);
}

ErrorStatus address_destroy(Address *address) {
    if (address == NULL) return ERROR_NULL;
    string_destroy(&address->city);
    string_destroy(&address->street);
    string_destroy(&address->building);
    string_destroy(&address->index);
    memset(address, 0, sizeof(Address));
    return OK;
}

ErrorStatus address_from_fields(char *const fields[6], Address *result) {
    if (fields == NULL || result == NULL) return ERROR_NULL;
    for (size_t i = 0; i < 6; i++) {
        if (fields[i] == NULL) return ERROR_NULL;
    }
    if (*fields[0] == '\0' || *fields[1] == '\0') return ERROR_INPUT;
    size_t house = 0;
    size_t apartment = 0;
    ErrorStatus status = parse_size(fields[2], &house);
    if (status == OK) status = parse_size(fields[4], &apartment);
    if (status == OK && (house == 0 || apartment == 0 || house > UINT_MAX || apartment > UINT_MAX)) status = ERROR_INPUT;
    if (status == OK) status = digits_exact(fields[5], 6);
    Address address = {0};
    if (status == OK) status = string_create(fields[0], &address.city);
    if (status == OK) status = string_create(fields[1], &address.street);
    if (status == OK) status = string_create(fields[3], &address.building);
    if (status == OK) status = string_create(fields[5], &address.index);
    if (status != OK) {
        address_destroy(&address);
        return status;
    }
    address.house = (unsigned int)house;
    address.apartment = (unsigned int)apartment;
    *result = address;
    return OK;
}

ErrorStatus mail_destroy(Mail *mail) {
    if (mail == NULL) return ERROR_NULL;
    address_destroy(&mail->recipient);
    string_destroy(&mail->id);
    string_destroy(&mail->created);
    string_destroy(&mail->delivery);
    string_destroy(&mail->deadline);
    memset(mail, 0, sizeof(Mail));
    return OK;
}

ErrorStatus parse_mail(char *line, Mail *result) {
    if (line == NULL || result == NULL) return ERROR_NULL;
    char *fields[10];
    Mail mail = {0};
    ErrorStatus status = split_pipes(line, fields, 10);
    if (status == OK) status = address_from_fields(fields, &mail.recipient);
    if (status == OK) {
        char *end = NULL;
        if (*fields[6] == '\0' || *fields[6] == '-') status = ERROR_INPUT;
        else {
            mail.weight = strtod(fields[6], &end);
            if (end == fields[6] || *end != '\0' || !isfinite(mail.weight)) status = ERROR_INPUT;
        }
    }
    if (status == OK) status = digits_exact(fields[7], 14);
    if (status == OK) status = parse_datetime(fields[8], &mail.created_at);
    if (status == OK) status = parse_datetime(fields[9], &mail.deadline_at);
    int order = 0;
    if (status == OK) status = compare_datetime(&mail.deadline_at, &mail.created_at, &order);
    if (status == OK && order < 0) status = ERROR_TIME;
    if (status == OK) status = string_create(fields[7], &mail.id);
    if (status == OK) status = string_create(fields[8], &mail.created);
    if (status == OK) status = string_create("", &mail.delivery);
    if (status == OK) status = string_create(fields[9], &mail.deadline);
    if (status != OK) {
        mail_destroy(&mail);
        return status;
    }
    *result = mail;
    return OK;
}

ErrorStatus post_destroy(Post *post) {
    if (post == NULL) return ERROR_NULL;
    for (size_t i = 0; i < post->count; i++) mail_destroy(&post->items[i]);
    free(post->items);
    if (post->address != NULL) address_destroy(post->address);
    free(post->address);
    memset(post, 0, sizeof(Post));
    return OK;
}

ErrorStatus find_mail(const Post *post, const char *id, size_t *result) {
    if (post == NULL || id == NULL || result == NULL) return ERROR_NULL;
    ErrorStatus status = digits_exact(id, 14);
    if (status != OK) return status;
    if (post->count != 0 && post->items == NULL) return ERROR_NULL;
    for (size_t i = 0; i < post->count; i++) {
        if (strcmp(post->items[i].id.data, id) == 0) {
            *result = i;
            return OK;
        }
    }
    return ERROR_NOT_FOUND;
}

ErrorStatus compare_mail(const Mail *first, const Mail *second, int *result) {
    if (first == NULL || second == NULL || result == NULL) return ERROR_NULL;
    if (first->recipient.index.data == NULL || second->recipient.index.data == NULL || first->id.data == NULL || second->id.data == NULL) return ERROR_NULL;
    int order = strcmp(first->recipient.index.data, second->recipient.index.data);
    *result = order != 0 ? order : strcmp(first->id.data, second->id.data);
    return OK;
}

ErrorStatus add_mail(Post *post, Mail *mail) {
    if (post == NULL || mail == NULL) return ERROR_NULL;
    size_t found = 0;
    ErrorStatus status = find_mail(post, mail->id.data, &found);
    if (status == OK) return ERROR_DUPLICATE;
    if (status != ERROR_NOT_FOUND) return status;
    if (post->count == post->capacity) {
        size_t capacity = post->capacity == 0 ? 8 : post->capacity;
        if (capacity > SIZE_MAX / 2 / sizeof(Mail)) return ERROR_OVERFLOW;
        capacity *= 2;
        Mail *temp = realloc(post->items, capacity * sizeof(Mail));
        if (temp == NULL) return ERROR_MEMORY;
        post->items = temp;
        post->capacity = capacity;
    }
    size_t place = 0;
    while (place < post->count) {
        int order = 0;
        status = compare_mail(&post->items[place], mail, &order);
        if (status != OK) return status;
        if (order >= 0) break;
        place++;
    }
    memmove(post->items + place + 1, post->items + place, (post->count - place) * sizeof(Mail));
    post->items[place] = *mail;
    post->count++;
    memset(mail, 0, sizeof(Mail));
    return OK;
}

ErrorStatus remove_mail(Post *post, const char *id) {
    if (post == NULL || id == NULL) return ERROR_NULL;
    size_t index = 0;
    ErrorStatus status = find_mail(post, id, &index);
    if (status != OK) return status;
    mail_destroy(&post->items[index]);
    memmove(post->items + index, post->items + index + 1, (post->count - index - 1) * sizeof(Mail));
    post->count--;
    return OK;
}

ErrorStatus deliver_mail(Post *post, const char *id) {
    if (post == NULL || id == NULL) return ERROR_NULL;
    size_t index = 0;
    ErrorStatus status = find_mail(post, id, &index);
    if (status != OK) return status;
    Mail *mail = &post->items[index];
    if (mail->delivery.length != 0) return ERROR_INPUT;
    DateTime now;
    status = current_datetime(&now);
    if (status != OK) return status;
    int order = 0;
    status = compare_datetime(&now, &mail->created_at, &order);
    if (status != OK) return status;
    if (order < 0) return ERROR_TIME;
    char buffer[20];
    if (snprintf(buffer, sizeof(buffer), "%02d:%02d:%04d %02d:%02d:%02d", now.day, now.month, now.year, now.hour, now.minute, now.second) != 19) return ERROR_TIME;
    String delivery = {0};
    status = string_create(buffer, &delivery);
    if (status != OK) return status;
    string_destroy(&mail->delivery);
    mail->delivery = delivery;
    return OK;
}

int compare_creation(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Mail *const *a = first;
    const Mail *const *b = second;
    int order = 0;
    if (compare_datetime(&(*a)->created_at, &(*b)->created_at, &order) != OK) return 0;
    if (order != 0) return order;
    return strcmp((*a)->id.data, (*b)->id.data);
}

ErrorStatus select_mail(const Post *post, size_t mode, const DateTime *now, const Mail ***result, size_t *count) {
    if (post == NULL || now == NULL || result == NULL || count == NULL) return ERROR_NULL;
    if (mode != 1 && mode != 2) return ERROR_INPUT;
    *result = NULL;
    *count = 0;
    if (post->count == 0) return OK;
    if (post->items == NULL) return ERROR_NULL;
    if (post->count > SIZE_MAX / sizeof(Mail *)) return ERROR_OVERFLOW;
    const Mail **selected = malloc(post->count * sizeof(Mail *));
    if (selected == NULL) return ERROR_MEMORY;
    for (size_t i = 0; i < post->count; i++) {
        const Mail *mail = &post->items[i];
        int delivered = mail->delivery.length != 0;
        int order = 0;
        ErrorStatus status = compare_datetime(&mail->deadline_at, now, &order);
        if (status != OK) {
            free(selected);
            return status;
        }
        if ((mode == 1 && delivered) || (mode == 2 && !delivered && order < 0)) selected[(*count)++] = mail;
    }
    if (*count > 1) qsort(selected, *count, sizeof(Mail *), compare_creation);
    *result = selected;
    return OK;
}

ErrorStatus print_mail(FILE *output, const Mail *mail) {
    if (output == NULL || mail == NULL) return ERROR_NULL;
    const Address *address = &mail->recipient;
    if (fprintf(output, "ID %s | %s, %s, дом %u, корпус %s, кв. %u, индекс %s | вес %.6f\nСоздано: %s | Срок: %s | Вручено: %s\n", mail->id.data, address->city.data, address->street.data, address->house, address->building.data, address->apartment, address->index.data, mail->weight, mail->created.data, mail->deadline.data, mail->delivery.length == 0 ? "нет" : mail->delivery.data) < 0) return ERROR_WRITE;
    return OK;
}

ErrorStatus post_dialog(Post *post) {
    if (post == NULL || post->address == NULL) return ERROR_NULL;
    ErrorStatus status = OK;
    while (status == OK) {
        char *line = NULL;
        status = ask_line("\n0 Выход | 1 Все | 2 Добавить | 3 Удалить | 4 Найти | 5 Доставленные | 6 Просроченные | 7 Вручить | 8 String\n> ", &line);
        if (status == END_OF_FILE) return OK;
        size_t command = 0;
        if (status == OK) status = parse_size(line, &command);
        free(line);
        if (status == OK && command == 0) return OK;
        if (status == OK && command == 1) {
            for (size_t i = 0; i < post->count && status == OK; i++) status = print_mail(stdout, &post->items[i]);
        } else if (status == OK && command == 2) {
            line = NULL;
            status = ask_line("Город|улица|дом|корпус|квартира|индекс|вес|id|создано|срок\n> ", &line);
            Mail mail = {0};
            if (status == OK) status = parse_mail(line, &mail);
            if (status == OK) status = add_mail(post, &mail);
            mail_destroy(&mail);
            free(line);
        } else if (status == OK && (command == 3 || command == 4 || command == 7)) {
            line = NULL;
            status = ask_line("Почтовый id из 14 цифр: ", &line);
            if (status == OK && command == 3) status = remove_mail(post, line);
            else if (status == OK && command == 7) status = deliver_mail(post, line);
            else if (status == OK) {
                size_t index = 0;
                status = find_mail(post, line, &index);
                if (status == OK) status = print_mail(stdout, &post->items[index]);
            }
            free(line);
        } else if (status == OK && (command == 5 || command == 6)) {
            DateTime now;
            const Mail **selected = NULL;
            size_t count = 0;
            status = current_datetime(&now);
            if (status == OK) status = select_mail(post, command == 5 ? 1 : 2, &now, &selected, &count);
            for (size_t i = 0; i < count && status == OK; i++) status = print_mail(stdout, selected[i]);
            free(selected);
        } else if (status == OK && command == 8) status = string_demo(stdout);
        else if (status == OK) status = ERROR_INPUT;
        if (status == END_OF_FILE) return OK;
        if (status == ERROR_INPUT || status == ERROR_OVERFLOW || status == ERROR_NOT_FOUND || status == ERROR_DUPLICATE || status == ERROR_TIME) {
            print_error(status);
            status = OK;
        }
    }
    return status;
}

int main(int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        print_error(ERROR_COUNT);
        return 1;
    }
    Post post = {0};
    post.address = calloc(1, sizeof(Address));
    if (post.address == NULL) {
        print_error(ERROR_MEMORY);
        return 1;
    }
    char *line = NULL;
    ErrorStatus status = ask_line("Адрес отделения: город|улица|дом|корпус|квартира|индекс\n> ", &line);
    char *fields[6];
    if (status == OK) status = split_pipes(line, fields, 6);
    if (status == OK) status = address_from_fields(fields, post.address);
    free(line);
    if (status == OK) status = post_dialog(&post);
    if (status == END_OF_FILE) status = OK;
    post_destroy(&post);
    if (status == OK && fflush(stdout) != 0) status = ERROR_WRITE;
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
