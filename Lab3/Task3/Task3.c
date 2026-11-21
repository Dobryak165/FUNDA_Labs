#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef enum {
    OK = 0,
    ERROR_NULL,
    ERROR_COUNT,
    ERROR_INPUT,
    ERROR_MEMORY,
    ERROR_OPEN,
    ERROR_READ,
    ERROR_WRITE,
    ERROR_OVERFLOW,
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
        case ERROR_EMPTY:
            printf("Коллекция пуста\n");
            break;
        default:
            printf("Неизвестная ошибка\n");
            break;
    }
}

typedef struct {
    uint64_t weight;
    int parent;
    unsigned int bit;
} HuffmanNode;

typedef struct {
    int items[256];
    size_t count;
    const HuffmanNode *nodes;
    size_t node_count;
} MinHeap;

ErrorStatus heap_push(MinHeap *heap, int index) {
    if (heap == NULL || heap->nodes == NULL) return ERROR_NULL;
    if (index < 0 || (size_t)index >= heap->node_count || heap->count >= 256) return ERROR_INPUT;
    size_t place = heap->count++;
    while (place > 0) {
        size_t parent = (place - 1) / 2;
        if (heap->nodes[heap->items[parent]].weight <= heap->nodes[index].weight) break;
        heap->items[place] = heap->items[parent];
        place = parent;
    }
    heap->items[place] = index;
    return OK;
}

ErrorStatus heap_pop(MinHeap *heap, int *result) {
    if (heap == NULL || heap->nodes == NULL || result == NULL) return ERROR_NULL;
    if (heap->count == 0) return ERROR_EMPTY;
    if (heap->count > 256) return ERROR_INPUT;
    *result = heap->items[0];
    int last = heap->items[--heap->count];
    if (heap->count == 0) return OK;
    size_t place = 0;
    while (place * 2 + 1 < heap->count) {
        size_t child = place * 2 + 1;
        if (child + 1 < heap->count && heap->nodes[heap->items[child + 1]].weight < heap->nodes[heap->items[child]].weight) child++;
        if (heap->nodes[last].weight <= heap->nodes[heap->items[child]].weight) break;
        heap->items[place] = heap->items[child];
        place = child;
    }
    heap->items[place] = last;
    return OK;
}

ErrorStatus read_frequencies(FILE *input, uint64_t frequencies[256], uint64_t *size) {
    if (input == NULL || frequencies == NULL || size == NULL) return ERROR_NULL;
    memset(frequencies, 0, 256 * sizeof(uint64_t));
    *size = 0;
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch < 0 || ch > 255) return ERROR_INPUT;
        if (*size == UINT64_MAX) return ERROR_OVERFLOW;
        frequencies[ch]++;
        (*size)++;
    }
    if (!feof(input)) return ERROR_READ;
    return OK;
}

ErrorStatus build_codes(const uint64_t frequencies[256], char codes[256][256]) {
    if (frequencies == NULL || codes == NULL) return ERROR_NULL;
    memset(codes, 0, 256 * 256);
    HuffmanNode *nodes = calloc(511, sizeof(HuffmanNode));
    if (nodes == NULL) return ERROR_MEMORY;
    int leaves[256];
    MinHeap heap = {{0}, 0, nodes, 0};
    ErrorStatus status = OK;
    for (int byte = 0; byte < 256 && status == OK; byte++) {
        leaves[byte] = -1;
        if (frequencies[byte] != 0) {
            int index = (int)heap.node_count++;
            nodes[index].weight = frequencies[byte];
            nodes[index].parent = -1;
            leaves[byte] = index;
            status = heap_push(&heap, index);
        }
    }
    while (heap.count > 1 && status == OK) {
        int left = -1;
        int right = -1;
        status = heap_pop(&heap, &left);
        if (status == OK) status = heap_pop(&heap, &right);
        if (status == OK) {
            if (nodes[left].weight > UINT64_MAX - nodes[right].weight) status = ERROR_OVERFLOW;
            else {
                int index = (int)heap.node_count++;
                nodes[index].weight = nodes[left].weight + nodes[right].weight;
                nodes[index].parent = -1;
                nodes[left].parent = index;
                nodes[left].bit = 0;
                nodes[right].parent = index;
                nodes[right].bit = 1;
                status = heap_push(&heap, index);
            }
        }
    }
    if (status == OK) {
        for (int byte = 0; byte < 256; byte++) {
            if (leaves[byte] == -1) continue;
            char reversed[256];
            size_t length = 0;
            int current = leaves[byte];
            while (nodes[current].parent != -1) {
                reversed[length++] = nodes[current].bit == 0 ? '0' : '1';
                current = nodes[current].parent;
            }
            if (length == 0) reversed[length++] = '0';
            for (size_t j = 0; j < length; j++) codes[byte][j] = reversed[length - j - 1];
            codes[byte][length] = '\0';
        }
    }
    free(nodes);
    return status;
}

ErrorStatus encoded_size(const uint64_t frequencies[256], const char *codes, uint64_t *bits, uint64_t *bytes) {
    if (frequencies == NULL || codes == NULL || bits == NULL || bytes == NULL) return ERROR_NULL;
    uint64_t total = 0;
    for (size_t i = 0; i < 256; i++) {
        size_t length = strlen(codes + i * 256);
        if (frequencies[i] == 0) continue;
        if (length == 0 || length > 255) return ERROR_INPUT;
        if (frequencies[i] > (UINT64_MAX - total) / length) return ERROR_OVERFLOW;
        total += frequencies[i] * length;
    }
    *bits = total;
    *bytes = total / 8 + (total % 8 != 0);
    return OK;
}

ErrorStatus print_statistics(FILE *output, const uint64_t frequencies[256], const char *codes, uint64_t original, uint64_t bits, uint64_t bytes) {
    if (output == NULL || frequencies == NULL || codes == NULL) return ERROR_NULL;
    if (fprintf(output, "Байт Частота Код\n") < 0) return ERROR_WRITE;
    for (size_t i = 0; i < 256; i++) {
        if (frequencies[i] != 0 && fprintf(output, "%zu %llu %s\n", i, (unsigned long long)frequencies[i], codes + i * 256) < 0) return ERROR_WRITE;
    }
    long double saving = original == 0 ? 0.0L : 100.0L * (1.0L - (long double)bytes / (long double)original);
    if (fprintf(output, "Исходный размер: %llu байт\nЗакодировано: %llu бит\nЗакодировано с округлением: %llu байт\nЭкономия: %.6Lf%%\n", (unsigned long long)original, (unsigned long long)bits, (unsigned long long)bytes, saving) < 0) return ERROR_WRITE;
    return OK;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_error(ERROR_COUNT);
        printf("Использование: ./Task3 <файл>\n");
        return 1;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        print_error(ERROR_OPEN);
        return 1;
    }
    uint64_t frequencies[256];
    char codes[256][256];
    uint64_t original = 0;
    uint64_t bits = 0;
    uint64_t bytes = 0;
    ErrorStatus status = read_frequencies(input, frequencies, &original);
    if (fclose(input) != 0 && status == OK) status = ERROR_READ;
    if (status == OK) status = build_codes(frequencies, codes);
    if (status == OK) status = encoded_size(frequencies, (const char *)&codes, &bits, &bytes);
    if (status == OK) status = print_statistics(stdout, frequencies, (const char *)&codes, original, bits, bytes);
    if (status == OK && fflush(stdout) != 0) status = ERROR_WRITE;
    if (status != OK) {
        print_error(status);
        return 1;
    }
    return 0;
}
