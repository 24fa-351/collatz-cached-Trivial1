#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// Function to compute Collatz steps
int collatz_steps(int n) {
    int steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n /= 2;
        } else {
            n = 3 * n + 1;
        }
        steps++;
    }
    return steps;
}

// ARC Cache Structures
typedef struct {
    int key;
    int value;
} CacheEntry;

typedef struct {
    CacheEntry *data;
    int size;
    int capacity;
} CacheList;

typedef struct {
    CacheList T1; // Recent data
    CacheList T2; // Frequent data
    CacheList B1; // Ghost entries for T1
    CacheList B2; // Ghost entries for T2
    int p;        // Balancing pointer
} ARC;

// Cache Management
void init_cache_list(CacheList *list, int capacity) {
    list->data = malloc(sizeof(CacheEntry) * capacity);
    list->size = 0;
    list->capacity = capacity;
}

void free_cache_list(CacheList *list) {
    free(list->data);
    list->size = 0;
    list->capacity = 0;
}

void init_arc(ARC *arc, int capacity) {
    init_cache_list(&arc->T1, capacity / 2);
    init_cache_list(&arc->T2, capacity / 2);
    init_cache_list(&arc->B1, capacity / 2);
    init_cache_list(&arc->B2, capacity / 2);
    arc->p = 0;
}

void free_arc(ARC *arc) {
    free_cache_list(&arc->T1);
    free_cache_list(&arc->T2);
    free_cache_list(&arc->B1);
    free_cache_list(&arc->B2);
}

bool is_in_cache(CacheList *list, int key, int *index) {
    for (int i = 0; i < list->size; i++) {
        if (list->data[i].key == key) {
            if (index) *index = i;
            return true;
        }
    }
    return false;
}

void move_to_front(CacheList *list, int index) {
    CacheEntry temp = list->data[index];
    for (int i = index; i > 0; i--) {
        list->data[i] = list->data[i - 1];
    }
    list->data[0] = temp;
}

void add_to_cache(CacheList *list, int key, int value) {
    if (list->size < list->capacity) {
        list->data[list->size++] = (CacheEntry){.key = key, .value = value};
    } else {
        for (int i = list->size - 1; i > 0; i--) {
            list->data[i] = list->data[i - 1];
        }
        list->data[0] = (CacheEntry){.key = key, .value = value};
    }
}

int collatz_steps_cached(int n, ARC *arc) {
    int index;
    int steps;

    // Check T1 and T2
    if (is_in_cache(&arc->T1, n, &index)) {
        steps = arc->T1.data[index].value;
        move_to_front(&arc->T1, index);
        return steps;
    }
    if (is_in_cache(&arc->T2, n, &index)) {
        steps = arc->T2.data[index].value;
        move_to_front(&arc->T2, index);
        return steps;
    }

    // Calculate steps
    steps = collatz_steps(n);

    // Handle ghost caches B1 and B2
    if (is_in_cache(&arc->B1, n, &index)) {
        arc->p = (arc->p + 1 < arc->T1.capacity) ? arc->p + 1 : arc->T1.capacity;
        add_to_cache(&arc->T2, n, steps);
        return steps;
    }
    if (is_in_cache(&arc->B2, n, &index)) {
        arc->p = (arc->p - 1 > 0) ? arc->p - 1 : 0;
        add_to_cache(&arc->T2, n, steps);
        return steps;
    }

    // Add to T1
    add_to_cache(&arc->T1, n, steps);
    return steps;
}

void process_file(const char *filename, ARC *arc) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[256];
    printf("Number,Steps\n");
    while (fgets(line, sizeof(line), file)) {
        int num = atoi(line);
        int steps = arc ? collatz_steps_cached(num, arc) : collatz_steps(num);
        printf("%d,%d\n", num, steps);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <N> <MIN> <MAX> [cache_policy] [cache_size] [file]\n", argv[0]);
        return 1;
    }

    // Extract filename argument (if provided)
    char *filename = (argc > 6) ? argv[6] : NULL;

    // If a filename is provided, bypass N, MIN, MAX validation
    if (filename) {
        char *cache_policy = (argc > 4) ? argv[4] : "none";
        int cache_size = (argc > 5) ? atoi(argv[5]) : 0;

        if (strcmp(cache_policy, "arc") == 0) {
            ARC arc;
            init_arc(&arc, cache_size);
            process_file(filename, &arc);
            free_arc(&arc);
        } else {
            process_file(filename, NULL);
        }
        return 0; // Exit after processing the file
    }

    // Extract and validate arguments for random testing
    int N = atoi(argv[1]);
    int MIN = atoi(argv[2]);
    int MAX = atoi(argv[3]);

    if (N <= 0 || MIN <= 0 || MAX <= 0 || MIN > MAX) {
        fprintf(stderr, "Invalid input. Ensure N, MIN, and MAX are positive, and MIN <= MAX.\n");
        return 1;
    }

    // Extract cache arguments
    char *cache_policy = (argc > 4) ? argv[4] : "none";
    int cache_size = (argc > 5) ? atoi(argv[5]) : 0;

    srand(time(NULL));

    // Initialize ARC if needed
    ARC arc;
    if (strcmp(cache_policy, "arc") == 0) {
        init_arc(&arc, cache_size);
    }

    // Perform random tests
    printf("Number,Steps\n");
    for (int i = 0; i < N; i++) {
        int num = MIN + rand() % (MAX - MIN + 1);
        int steps = (strcmp(cache_policy, "arc") == 0) ? collatz_steps_cached(num, &arc) : collatz_steps(num);
        printf("%d,%d\n", num, steps);
    }

    if (strcmp(cache_policy, "arc") == 0) {
        free_arc(&arc);
    }

    return 0;
}