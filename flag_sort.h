// Copyright 2021 Charlie Shenton
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef FLAG_SORT_H
#define FLAG_SORT_H

#include <stddef.h>

// Sort an array of count entries of size stride with uint32_t sort keys at byte offset.
void flag_sort_u32(void *array, size_t count, size_t stride, size_t offset);

#endif // FLAG_SORT_H

#ifdef FLAG_SORT_IMPL

#include <stdint.h>

// Performs the distribute step on the provided array, writing offsets
static void flag_sort__distribute_u32(uint8_t *byte_array, size_t count, size_t stride, size_t offset, size_t depth, size_t offsets[257])
{
    // Initialise a temp array of counts to zero
    size_t counts[256] = {0};

    // Compute counts for each byte's bin
    for (size_t i = 0; i < count; i++)
    {
        uint32_t val = *((uint32_t *)&byte_array[stride * i + offset]);
        uint8_t byte = (val >> ((3 - depth) * 8)) & 0xff;
        counts[byte]++;
    }

    // Use counts to compute bin start offsets
    size_t sum = 0;
    for (size_t i = 0; i < 256; i++)
    {
        offsets[i] = sum;
        sum += counts[i];
    }
    offsets[256] = sum;

    size_t idx = 0;
    while (idx < count)
    {
        // Determine which bucket the value at idx should be in
        uint32_t val = *((uint32_t *)&byte_array[stride * idx + offset]);
        uint8_t byte = (val >> ((3 - depth) * 8)) & 0xff;

        size_t bucket_lo = offsets[byte];
        size_t bucket_hi = offsets[byte + 1];

        // If we're in the right bucket move on
        if (bucket_lo <= idx && idx < bucket_hi)
        {
            idx++;
            continue;
        }

        // Otherwise swap this idx to its correct bucket
        size_t swap_idx = bucket_hi - counts[byte];
        for (size_t i = 0; i < stride; i++)
        {
            uint8_t temp = byte_array[stride * idx + i];
            byte_array[stride * idx + i] = byte_array[stride * swap_idx + i];
            byte_array[stride * swap_idx + i] = temp;
        }
        counts[byte]--;
    }
}

void flag_sort_u32(void *array, size_t count, size_t stride, size_t offset)
{
    // Interpret the input as bytes
    uint8_t *byte_array = (uint8_t *)array;

    // Allocate space on the stack for bucket offsets
    size_t offsets[4][257];

    // Compute the max value in the array
    uint32_t max_val = 0;
    for (size_t i = 0; i < count; i++)
    {
        uint32_t val = byte_array[stride * i + offset];
        max_val = (max_val > val) ? max_val : val;
    }

    // Perform a distribute step on the top level array to get our first layer
    flag_sort__distribute_u32(byte_array, count, stride, offset, 0, offsets[0]);

    for (size_t i = 0; i < 256; i++)
    {
        size_t lo_i = offsets[0][i];
        size_t hi_i = offsets[0][i + 1];
        size_t count_i = hi_i - lo_i;
        uint8_t *bytes_i = &byte_array[lo_i * stride];
        if (count_i == 0)
        {
            continue;
        }

        flag_sort__distribute_u32(bytes_i, count_i, stride, offset, 1, offsets[1]);

        for (size_t j = 0; j < 256; j++)
        {
            size_t lo_j = offsets[1][j];
            size_t hi_j = offsets[1][j + 1];
            size_t count_j = hi_j - lo_j;
            uint8_t *bytes_j = &bytes_i[lo_j * stride];
            if (count_j == 0)
            {
                continue;
            }

            flag_sort__distribute_u32(bytes_j, count_j, stride, offset, 2, offsets[2]);

            for (size_t k = 0; k < 256; k++)
            {
                size_t lo_k = offsets[2][k];
                size_t hi_k = offsets[2][k + 1];
                size_t count_k = hi_k - lo_k;
                uint8_t *bytes_k = &bytes_j[lo_k * stride];
                if (count_k == 0)
                {
                    continue;
                }

                flag_sort__distribute_u32(bytes_k, count_k, stride, offset, 3, offsets[3]);
            }
        }
    }
}

#endif // FLAG_SORT_IMPL