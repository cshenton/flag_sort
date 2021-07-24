# flag_sort

C99 Single Header american flag sort implementation.

I often find myself wanting an inplace radix sort implementation, however off the shelf
implementations often only support sorting plain arrays of ints. Real world use cases
typically require sorting data alongside the integer keys, so this library exposes a
simple sorting function that makes that easy.

This library gives you a radix sort implementation which does not allocate heap memory,
does not rely on recursion in its implementation, and is very portable thanks to being
a single C99 header.


# Usage

To use `flag_sort.h`, include it wherever it is needed, and in _one_ source file, define
`FLAG_SORT_IMPL` before including the header to drag in the implementation.

```c
#define FLAG_SORT_IMPL
#include "flag_sort.h"
```

The sorting function is quite simple.

```c
void flag_sort_u32(void *array, size_t count, size_t stride, size_t offset);
```

So, if I had a structure like the one below:

```c
typedef struct entry_t
{
    uint32_t id;
    uint32_t key;
} entry_t;
```

And I wanted to sort an array of 1000 `entry_t` on the `key` field, I would write:

```c
size_t entry_count = 1000;
entry_t *entries = (entry_t *)malloc(entry_count * sizeof(entry_t));
// Check for allocation failures, then Fill in the data

flag_sort_u32(entries, entry_count, sizeof(entry_t), offsetof(entry_t, key));
```

See `example.c` for a full worked example.

This library, though it runs at a respectable speed (roughly 20ns per sorted `entry_t` on a
single core of an `i9 108050K`, compiled at `-O2` with clang), is not quite as fast as I would
like. As I use it elsewhere I'll likely do some performance passes on it and add 16, 64 bit and
signed integer sorting functions.