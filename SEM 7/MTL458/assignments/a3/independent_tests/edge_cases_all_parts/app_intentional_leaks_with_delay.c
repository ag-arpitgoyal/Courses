#include "myalloc.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Drop-in application test that leaves two known allocations at fixed offsets unfreed,
       plus frees others with delay enabled, and checks my_leak_report() output. */
    my_set_delay(4);
    char *keep1 = my_malloc(50);   /* 0:32 A 128 50 */
    char *temp1 = my_malloc(100);  /* 0:160 A 144 100 */
    char *keep2 = my_malloc(200);  /* 0:304 A 240 200 */
    char *temp2 = my_malloc(80);   /* 0:544 A 128 80 */

    strcpy(keep1, "hello");
    strcpy(temp1, "temporary");
    strcpy(keep2, "world");
    strcpy(temp2, "discard");

    my_free(temp1);
    my_free(temp2);

    printf("%s %s\n", keep1, keep2);
    my_leak_report();
    return 0;
}
