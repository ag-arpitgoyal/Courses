#include "myalloc.h"
#include <stdio.h>
#include <stdint.h>

int main(void) {
    size_t cap = 4;
    size_t len = 0;
    int64_t *arr = my_calloc(cap, sizeof(int64_t));

    for (int64_t i = 1; i <= 500; i++) {
        if (len == cap) {
            cap *= 2;
            arr = my_realloc(arr, cap * sizeof(int64_t));
        }
        arr[len++] = i * i;
    }

    /* Shrink array to exact size */
    arr = my_realloc(arr, len * sizeof(int64_t));

    int64_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += arr[i];
    }
    /* sum of i^2 for i=1..500 is 500 * 501 * 1001 / 6 = 41791750 */
    printf("len=%zu sum=%lld\n", len, (long long)sum);

    my_free(arr);
    my_leak_report();
    return 0;
}
