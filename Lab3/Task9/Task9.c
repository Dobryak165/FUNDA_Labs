#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

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

typedef struct Node {
    char *word;
    uint64_t count;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

typedef struct {
    Node *root;
    size_t size;
} WordTree;

typedef struct {
    Node **link;
    Node *parent;
} PendingSlot;

ErrorStatus parse_frequency(const char *text, uint64_t *result) {
    if (text == NULL || result == NULL) return ERROR_NULL;
    if (*text == '\0') return ERROR_INPUT;
    uint64_t value = 0;
    while (*text != '\0') {
        if (*text < '0' || *text > '9') return ERROR_INPUT;
        uint64_t digit = (uint64_t)(*text - '0');
        if (value > (UINT64_MAX - digit) / 10) return ERROR_OVERFLOW;
        value = value * 10 + digit;
        text++;
    }
    *result = value;
    return OK;
}

ErrorStatus destroy_tree(WordTree *tree) {
    if (tree == NULL) return ERROR_NULL;
    Node *node = tree->root;
    while (node != NULL) {
        if (node->left != NULL) {
            Node *left = node->left;
            node->left = left->right;
            left->right = node;
            node = left;
        } else {
            Node *next = node->right;
            free(node->word);
            free(node);
            node = next;
        }
    }
    tree->root = NULL;
    tree->size = 0;
    return OK;
}

ErrorStatus insert_word(WordTree *tree, const char *word) {
    if (tree == NULL || word == NULL) return ERROR_NULL;
    if (*word == '\0') return ERROR_INPUT;
    Node **link = &tree->root;
    Node *parent = NULL;
    while (*link != NULL) {
        int order = strcmp(word, (*link)->word);
        if (order == 0) {
            if ((*link)->count == UINT64_MAX) return ERROR_OVERFLOW;
            (*link)->count++;
            return OK;
        }
        parent = *link;
        link = order < 0 ? &parent->left : &parent->right;
    }
    if (tree->size == SIZE_MAX) return ERROR_OVERFLOW;
    Node *node = calloc(1, sizeof(Node));
    if (node == NULL) return ERROR_MEMORY;
    ErrorStatus status = copy_text(word, &node->word);
    if (status != OK) {
        free(node);
        return status;
    }
    node->count = 1;
    node->parent = parent;
    *link = node;
    tree->size++;
    return OK;
}

ErrorStatus find_word(const WordTree *tree, const char *word, const Node **result) {
    if (tree == NULL || word == NULL || result == NULL) return ERROR_NULL;
    if (*word == '\0') return ERROR_INPUT;
    *result = NULL;
    const Node *node = tree->root;
    while (node != NULL) {
        int order = strcmp(word, node->word);
        if (order == 0) {
            *result = node;
            return OK;
        }
        node = order < 0 ? node->left : node->right;
    }
    return ERROR_NOT_FOUND;
}

ErrorStatus first_node(const Node *root, const Node **result) {
    if (result == NULL) return ERROR_NULL;
    const Node *node = root;
    while (node != NULL && node->left != NULL) node = node->left;
    *result = node;
    return OK;
}

ErrorStatus next_node(const Node *node, const Node **result) {
    if (result == NULL) return ERROR_NULL;
    if (node == NULL) {
        *result = NULL;
        return OK;
    }
    if (node->right != NULL) return first_node(node->right, result);
    while (node->parent != NULL && node == node->parent->right) node = node->parent;
    *result = node->parent;
    return OK;
}

ErrorStatus collect_nodes(const WordTree *tree, const Node ***result) {
    if (tree == NULL || result == NULL) return ERROR_NULL;
    *result = NULL;
    if (tree->size == 0) return tree->root == NULL ? OK : ERROR_INPUT;
    if (tree->size > SIZE_MAX / sizeof(Node *)) return ERROR_OVERFLOW;
    const Node **nodes = malloc(tree->size * sizeof(Node *));
    if (nodes == NULL) return ERROR_MEMORY;
    const Node *node = NULL;
    first_node(tree->root, &node);
    size_t count = 0;
    while (node != NULL && count < tree->size) {
        nodes[count++] = node;
        next_node(node, &node);
    }
    if (count != tree->size || node != NULL) {
        free(nodes);
        return ERROR_INPUT;
    }
    *result = nodes;
    return OK;
}

ErrorStatus tree_depth(const WordTree *tree, size_t *result) {
    if (tree == NULL || result == NULL) return ERROR_NULL;
    *result = 0;
    if (tree->root == NULL) return OK;
    const Node *node = tree->root;
    size_t depth = 1;
    while (node != NULL) {
        if (depth > *result) *result = depth;
        if (node->left != NULL) {
            node = node->left;
            depth++;
        } else if (node->right != NULL) {
            node = node->right;
            depth++;
        } else {
            while (node->parent != NULL && (node == node->parent->right || node->parent->right == NULL)) {
                node = node->parent;
                depth--;
            }
            if (node->parent == NULL) node = NULL;
            else node = node->parent->right;
        }
    }
    return OK;
}

ErrorStatus word_extremes(const WordTree *tree, const Node **shortest, const Node **longest) {
    if (tree == NULL || shortest == NULL || longest == NULL) return ERROR_NULL;
    if (tree->root == NULL) return ERROR_EMPTY;
    *shortest = tree->root;
    *longest = tree->root;
    const Node *node = NULL;
    first_node(tree->root, &node);
    while (node != NULL) {
        if (strlen(node->word) < strlen((*shortest)->word)) *shortest = node;
        if (strlen(node->word) > strlen((*longest)->word)) *longest = node;
        next_node(node, &node);
    }
    return OK;
}

ErrorStatus parse_separators(int argc, char *argv[], unsigned char separators[256]) {
    if (argv == NULL || separators == NULL) return ERROR_NULL;
    if (argc < 3) return ERROR_COUNT;
    memset(separators, 0, 256);
    for (int i = 2; i < argc; i++) {
        if (argv[i] == NULL) return ERROR_NULL;
        size_t length = strlen(argv[i]);
        unsigned char ch = 0;
        if (length == 1) ch = (unsigned char)argv[i][0];
        else if (strcmp(argv[i], "\\n") == 0) ch = '\n';
        else if (strcmp(argv[i], "\\r") == 0) ch = '\r';
        else if (strcmp(argv[i], "\\t") == 0) ch = '\t';
        else return ERROR_INPUT;
        separators[ch] = 1;
    }
    return OK;
}

ErrorStatus load_words(FILE *input, const unsigned char separators[256], WordTree *result) {
    if (input == NULL || separators == NULL || result == NULL) return ERROR_NULL;
    WordTree tree = {0};
    size_t capacity = 128;
    size_t length = 0;
    char *word = malloc(capacity);
    if (word == NULL) return ERROR_MEMORY;
    ErrorStatus status = OK;
    int ch = 0;
    while (status == OK && (ch = fgetc(input)) != EOF) {
        if (ch <= 0 || ch > 255) status = ERROR_INPUT;
        else if (separators[ch]) {
            if (length != 0) {
                word[length] = '\0';
                status = insert_word(&tree, word);
                length = 0;
            }
        } else {
            if (length == capacity - 1) {
                if (capacity > SIZE_MAX / 2) status = ERROR_OVERFLOW;
                else {
                    char *temp = realloc(word, capacity * 2);
                    if (temp == NULL) status = ERROR_MEMORY;
                    else {
                        word = temp;
                        capacity *= 2;
                    }
                }
            }
            if (status == OK) word[length++] = (char)ch;
        }
    }
    if (status == OK && ch == EOF && !feof(input)) status = ERROR_READ;
    if (status == OK && length != 0) {
        word[length] = '\0';
        status = insert_word(&tree, word);
    }
    free(word);
    if (status != OK) {
        destroy_tree(&tree);
        return status;
    }
    *result = tree;
    return OK;
}

int compare_frequency(const void *first, const void *second) {
    if (first == NULL || second == NULL) return 0;
    const Node *const *a = first;
    const Node *const *b = second;
    if ((*a)->count != (*b)->count) return ((*a)->count < (*b)->count) - ((*a)->count > (*b)->count);
    return strcmp((*a)->word, (*b)->word);
}

ErrorStatus print_words(FILE *output, const WordTree *tree, size_t limit, int by_frequency) {
    if (output == NULL || tree == NULL) return ERROR_NULL;
    const Node **nodes = NULL;
    ErrorStatus status = collect_nodes(tree, &nodes);
    if (status != OK) return status;
    if (by_frequency && tree->size > 1) qsort(nodes, tree->size, sizeof(Node *), compare_frequency);
    if (limit > tree->size) limit = tree->size;
    for (size_t i = 0; i < limit && status == OK; i++) {
        if (fprintf(output, "%s: %" PRIu64 "\n", nodes[i]->word, nodes[i]->count) < 0) status = ERROR_WRITE;
    }
    free(nodes);
    return status;
}

ErrorStatus save_tree(const WordTree *tree, const char *path) {
    if (tree == NULL || path == NULL) return ERROR_NULL;
    if (*path == '\0') return ERROR_INPUT;
    if (tree->size == SIZE_MAX || tree->size + 1 > SIZE_MAX / sizeof(Node *)) return ERROR_OVERFLOW;
    const Node **stack = malloc((tree->size + 1) * sizeof(Node *));
    if (stack == NULL) return ERROR_MEMORY;
    FILE *output = fopen(path, "wb");
    if (output == NULL) {
        free(stack);
        return ERROR_OPEN;
    }
    ErrorStatus status = fputs("BST1\n", output) == EOF ? ERROR_WRITE : OK;
    size_t count = 0;
    stack[count++] = tree->root;
    while (count != 0 && status == OK) {
        const Node *node = stack[--count];
        if (node == NULL) {
            if (fputs("#\n", output) == EOF) status = ERROR_WRITE;
        } else {
            size_t length = strlen(node->word);
            if (fprintf(output, "N %" PRIu64 " %zu\n", node->count, length) < 0) status = ERROR_WRITE;
            if (status == OK && fwrite(node->word, 1, length, output) != length) status = ERROR_WRITE;
            if (status == OK && fputc('\n', output) == EOF) status = ERROR_WRITE;
            stack[count++] = node->right;
            stack[count++] = node->left;
        }
    }
    free(stack);
    if (fclose(output) != 0 && status == OK) status = ERROR_WRITE;
    return status;
}

ErrorStatus restore_tree(const char *path, WordTree *result) {
    if (path == NULL || result == NULL) return ERROR_NULL;
    if (*path == '\0') return ERROR_INPUT;
    FILE *input = fopen(path, "rb");
    if (input == NULL) return ERROR_OPEN;
    WordTree tree = {0};
    char *line = NULL;
    ErrorStatus status = read_line(input, &line);
    if (status == OK && strcmp(line, "BST1") != 0) status = ERROR_INPUT;
    free(line);
    size_t capacity = 16;
    size_t count = 0;
    PendingSlot *stack = NULL;
    if (status == OK) {
        stack = malloc(capacity * sizeof(PendingSlot));
        if (stack == NULL) status = ERROR_MEMORY;
        else stack[count++] = (PendingSlot){&tree.root, NULL};
    }
    while (count != 0 && status == OK) {
        PendingSlot slot = stack[--count];
        line = NULL;
        status = read_line(input, &line);
        if (status == OK && strcmp(line, "#") != 0) {
            char *fields[3];
            size_t fields_count = 0;
            size_t length = 0;
            uint64_t frequency = 0;
            status = split_fields(line, fields, 3, &fields_count);
            if (status == OK && (fields_count != 3 || strcmp(fields[0], "N") != 0)) status = ERROR_INPUT;
            if (status == OK) status = parse_frequency(fields[1], &frequency);
            if (status == OK) status = parse_size(fields[2], &length);
            if (status == OK && (frequency == 0 || length == 0 || length == SIZE_MAX || tree.size == SIZE_MAX)) status = ERROR_INPUT;
            Node *node = NULL;
            if (status == OK) {
                node = calloc(1, sizeof(Node));
                if (node == NULL) status = ERROR_MEMORY;
            }
            if (status == OK) {
                node->word = malloc(length + 1);
                if (node->word == NULL) status = ERROR_MEMORY;
            }
            if (status == OK) {
                if (fread(node->word, 1, length, input) != length) status = ERROR_READ;
                else {
                    node->word[length] = '\0';
                    if (memchr(node->word, '\0', length) != NULL || fgetc(input) != '\n') status = ERROR_INPUT;
                }
            }
            if (status == OK) {
                node->parent = slot.parent;
                node->count = frequency;
                *slot.link = node;
                tree.size++;
                if (count > capacity - 2) {
                    if (capacity > SIZE_MAX / 2 / sizeof(PendingSlot)) status = ERROR_OVERFLOW;
                    else {
                        PendingSlot *temp = realloc(stack, capacity * 2 * sizeof(PendingSlot));
                        if (temp == NULL) status = ERROR_MEMORY;
                        else {
                            stack = temp;
                            capacity *= 2;
                        }
                    }
                }
                if (status == OK) {
                    stack[count++] = (PendingSlot){&node->right, node};
                    stack[count++] = (PendingSlot){&node->left, node};
                }
            } else if (node != NULL) {
                free(node->word);
                free(node);
            }
        }
        free(line);
    }
    free(stack);
    if (status == END_OF_FILE) status = ERROR_INPUT;
    if (status == OK) {
        int ch = fgetc(input);
        if (ch != EOF) status = ERROR_INPUT;
        else if (!feof(input)) status = ERROR_READ;
    }
    if (fclose(input) != 0 && status == OK) status = ERROR_READ;
    if (status == OK) {
        const Node *previous = NULL;
        const Node *node = NULL;
        first_node(tree.root, &node);
        while (node != NULL && status == OK) {
            if (previous != NULL && strcmp(previous->word, node->word) >= 0) status = ERROR_INPUT;
            previous = node;
            next_node(node, &node);
        }
    }
    if (status != OK) {
        destroy_tree(&tree);
        return status;
    }
    *result = tree;
    return OK;
}

ErrorStatus word_dialog(WordTree *tree) {
    if (tree == NULL) return ERROR_NULL;
    ErrorStatus status = OK;
    while (status == OK) {
        char *line = NULL;
        status = ask_line("\n0 Выход | 1 Все | 2 Частота | 3 Топ n | 4 Длины | 5 Глубина | 6 Сохранить | 7 Восстановить\n> ", &line);
        if (status == END_OF_FILE) return OK;
        size_t command = 0;
        if (status == OK) status = parse_size(line, &command);
        free(line);
        if (status == OK && command == 0) return OK;
        if (status == OK && command == 1) status = print_words(stdout, tree, tree->size, 0);
        else if (status == OK && (command == 2 || command == 3 || command == 6 || command == 7)) {
            line = NULL;
            status = ask_line(command == 2 ? "Слово: " : command == 3 ? "n: " : "Путь: ", &line);
            if (status == OK && command == 2) {
                const Node *node = NULL;
                status = find_word(tree, line, &node);
                if (status == OK && printf("%s: %" PRIu64 "\n", node->word, node->count) < 0) status = ERROR_WRITE;
            } else if (status == OK && command == 3) {
                size_t limit = 0;
                status = parse_size(line, &limit);
                if (status == OK && limit == 0) status = ERROR_INPUT;
                if (status == OK) status = print_words(stdout, tree, limit, 1);
            } else if (status == OK && command == 6) status = save_tree(tree, line);
            else if (status == OK) {
                WordTree restored = {0};
                status = restore_tree(line, &restored);
                if (status == OK) {
                    destroy_tree(tree);
                    *tree = restored;
                }
            }
            free(line);
        } else if (status == OK && command == 4) {
            const Node *shortest = NULL;
            const Node *longest = NULL;
            status = word_extremes(tree, &shortest, &longest);
            if (status == OK && printf("Короткое: %s\nДлинное: %s\n", shortest->word, longest->word) < 0) status = ERROR_WRITE;
        } else if (status == OK && command == 5) {
            size_t depth = 0;
            status = tree_depth(tree, &depth);
            if (status == OK && printf("Глубина: %zu\n", depth) < 0) status = ERROR_WRITE;
        } else if (status == OK) status = ERROR_INPUT;
        if (status == END_OF_FILE) return OK;
        if (status == ERROR_INPUT || status == ERROR_NOT_FOUND || status == ERROR_EMPTY || status == ERROR_OPEN || status == ERROR_OVERFLOW) {
            print_error(status);
            status = OK;
        }
    }
    return status;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_error(ERROR_COUNT);
        printf("Использование: ./Task9 <текст> <разделитель> [разделитель ...]\n");
        return 1;
    }
    unsigned char separators[256];
    ErrorStatus status = parse_separators(argc, argv, separators);
    WordTree tree = {0};
    FILE *input = NULL;
    if (status == OK) {
        input = fopen(argv[1], "rb");
        if (input == NULL) status = ERROR_OPEN;
    }
    if (status == OK) status = load_words(input, separators, &tree);
    if (input != NULL && fclose(input) != 0 && status == OK) status = ERROR_READ;
    if (status == OK) status = print_words(stdout, &tree, tree.size, 0);
    if (status == OK) status = word_dialog(&tree);
    destroy_tree(&tree);
    if (status == OK && fflush(stdout) != 0) status = ERROR_WRITE;
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
