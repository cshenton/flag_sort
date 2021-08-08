#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FLAG_SORT_IMPL
#include "flag_sort.h"

// An entry with id, which we want to preserve, and key, which we want to sort on.
typedef struct entry_t
{
    uint32_t id;
    uint32_t key;
} entry_t;

int main()
{
    const size_t count = 1000000;
    const uint32_t seed = 42;
    srand(seed);

    // Allocate space for the array
    entry_t *entries = malloc(sizeof(*entries) * count);
    if (!entries)
    {
        printf("Out of Memory\n");
        return 1;
    }

    // Fill it in with sequential ids and random keys
    for (int i = 0; i < count; i++)
    {
        entries[i] = (entry_t){
            .id = i,
            .key = rand(),
        };
    }

    // Run the flag sort, the interface requires only byte sizes and offsets
    clock_t start = clock();
    flag_sort_u32(entries, count, sizeof(entry_t), offsetof(entry_t, key));
    clock_t diff = clock() - start;

    // Validate that the array is sorted
    for (int i = 1; i < count; i++)
    {
        if (entries[i - 1].key > entries[i].key)
        {
            printf("Not Sorted\n");
            return 1;
        }
    }

    printf("%zu entries sorted in %ld ms\n", count, diff * 1000 / CLOCKS_PER_SEC);
    free(entries);
    return 0;
}