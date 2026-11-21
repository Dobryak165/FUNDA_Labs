#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

typedef enum {
    OK = 0,
    END_OF_FILE,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_MEMORY,
    ERROR_OPEN,
    ERROR_READ,
    ERROR_WRITE,
    ERROR_EXPRESSION,
    ERROR_PATH
} ErrorStatus;

typedef enum {
    EXPECT_NAME,
    AFTER_NAME,
    AFTER_GROUP
} ParserState;

typedef struct Node {
    char *name;
    struct Node *parent;
    struct Node *first_child;
    struct Node *last_child;
    struct Node *next_sibling;
} Node;

void print_error(ErrorStatus status) {
    switch (status) {
        case ERROR_NULL:
            printf("Нулевой указатель\n");
            break;
        case ERROR_COUNT:
            printf("Использование: ./Task10 <входной файл> <выходной файл>\n");
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
            printf("Ошибка записи или закрытия выходного файла\n");
            break;
        case ERROR_EXPRESSION:
            printf("Некорректное скобочное выражение\n");
            break;
        case ERROR_PATH:
            printf("Пути должны быть непустыми и различаться\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
            break;
    }
}

ErrorStatus read_line(FILE *input, char **line) {
    if (input == NULL || line == NULL) return ERROR_NULL;
    *line = NULL;
    size_t capacity = 128;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (buffer == NULL) return ERROR_MEMORY;

    int ch;
    ErrorStatus status = OK;
    while (status == OK && (ch = fgetc(input)) != EOF && ch != '\n') {
        if (ch == '\0') {
            status = ERROR_EXPRESSION;
        } else {
            if (length == capacity - 1) {
                if (capacity > SIZE_MAX / 2) {
                    status = ERROR_MEMORY;
                } else {
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
    buffer[length] = '\0';
    *line = buffer;
    return OK;
}

ErrorStatus create_node(const char *start, size_t length, Node **result) {
    if (start == NULL || result == NULL) return ERROR_NULL;
    *result = NULL;
    if (length == 0 || length == SIZE_MAX) return ERROR_EXPRESSION;
    Node *node = malloc(sizeof(Node));
    if (node == NULL) return ERROR_MEMORY;
    node->name = malloc(length + 1);
    if (node->name == NULL) {
        free(node);
        return ERROR_MEMORY;
    }
    memcpy(node->name, start, length);
    node->name[length] = '\0';
    node->parent = NULL;
    node->first_child = NULL;
    node->last_child = NULL;
    node->next_sibling = NULL;
    *result = node;
    return OK;
}

ErrorStatus free_tree(Node **root) {
    if (root == NULL) return ERROR_NULL;
    Node *node = *root;
    while (node != NULL) {
        if (node->first_child != NULL) {
            Node *child = node->first_child;
            node->first_child = NULL;
            node = child;
        } else {
            Node *next = node->next_sibling;
            if (next == NULL) next = node->parent;
            free(node->name);
            free(node);
            node = next;
        }
    }
    *root = NULL;
    return OK;
}

ErrorStatus build_tree(const char *text, Node **result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    *result = NULL;
    Node *root = NULL;
    Node *parent = NULL;
    Node *current = NULL;
    ParserState state = EXPECT_NAME;
    ErrorStatus status = OK;

    while (*text != '\0' && status == OK) {
        if (isspace((unsigned char)*text)) {
            text++;
        } else if (state == EXPECT_NAME) {
            const char *start = text;
            size_t length = 0;
            while (*text != '\0' && !isspace((unsigned char)*text) && *text != '(' && *text != ')' && *text != ',') {
                text++;
                length++;
            }
            Node *node = NULL;
            status = create_node(start, length, &node);
            if (status == OK) {
                node->parent = parent;
                if (parent == NULL) root = node;
                else {
                    if (parent->first_child == NULL) parent->first_child = node;
                    else parent->last_child->next_sibling = node;
                    parent->last_child = node;
                }
                current = node;
                state = AFTER_NAME;
            }
        } else if (*text == '(' && state == AFTER_NAME) {
            parent = current;
            state = EXPECT_NAME;
            text++;
        } else if (*text == ',' && parent != NULL) {
            state = EXPECT_NAME;
            text++;
        } else if (*text == ')' && parent != NULL) {
            current = parent;
            parent = parent->parent;
            state = AFTER_GROUP;
            text++;
        } else {
            status = ERROR_EXPRESSION;
        }
    }
    if (status == OK && (root == NULL || parent != NULL || state == EXPECT_NAME)) status = ERROR_EXPRESSION;
    if (status != OK) {
        free_tree(&root);
        return status;
    }
    *result = root;
    return OK;
}

ErrorStatus print_tree(FILE *output, const Node *root) {
    if (output == NULL || root == NULL) return ERROR_NULL;
    const Node *node = root;
    size_t depth = 0;
    while (node != NULL) {
        for (size_t i = 0; i < depth; i++) {
            if (fputs("    ", output) == EOF) return ERROR_WRITE;
        }
        if (fprintf(output, "%s\n", node->name) < 0) return ERROR_WRITE;
        if (node->first_child != NULL) {
            node = node->first_child;
            depth++;
        } else {
            while (node != root && node->next_sibling == NULL) {
                node = node->parent;
                depth--;
            }
            if (node == root) node = NULL;
            else node = node->next_sibling;
        }
    }
    return OK;
}

ErrorStatus process_file(FILE *input, FILE *output) {
    if (input == NULL || output == NULL) return ERROR_NULL;
    ErrorStatus status = OK;
    size_t number = 0;
    while (status == OK) {
        char *line = NULL;
        status = read_line(input, &line);
        if (status == END_OF_FILE) return OK;
        if (status == OK) {
            Node *root = NULL;
            status = build_tree(line, &root);
            if (status == OK) {
                number++;
                if (fprintf(output, "Выражение %zu: %s\n", number, line) < 0) status = ERROR_WRITE;
                if (status == OK) status = print_tree(output, root);
                if (status == OK && fputc('\n', output) == EOF) status = ERROR_WRITE;
            }
            free_tree(&root);
        }
        free(line);
    }
    return status;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_error(ERROR_COUNT);
        return 1;
    }
    if (argv[1][0] == '\0' || argv[2][0] == '\0' || strcmp(argv[1], argv[2]) == 0) {
        print_error(ERROR_PATH);
        return 1;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        print_error(ERROR_OPEN);
        return 1;
    }
    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
        fclose(input);
        print_error(ERROR_OPEN);
        return 1;
    }
    ErrorStatus status = process_file(input, output);
    if (fclose(input) != 0 && status == OK) status = ERROR_READ;
    if (fclose(output) != 0 && status == OK) status = ERROR_WRITE;
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
