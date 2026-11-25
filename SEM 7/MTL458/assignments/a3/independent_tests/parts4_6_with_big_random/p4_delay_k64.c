/* k=64 with 65+ frees: 65th free evicts the first; WAF on delayed block; k=0 drains 64 blocks in FIFO order */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "myalloc.h"

static unsigned char STATIC_BUF[256];
static unsigned char *v0, *v1, *v2, *v3, *v4, *v5, *v6, *v7, *v8, *v9, *v10, *v11, *v12, *v13, *v14, *v15, *v16, *v17, *v18, *v19, *v20, *v21, *v22, *v23, *v24, *v25, *v26, *v27, *v28, *v29, *v30, *v31, *v32, *v33, *v34, *v35, *v36, *v37, *v38, *v39, *v40, *v41, *v42, *v43, *v44, *v45, *v46, *v47, *v48, *v49, *v50, *v51, *v52, *v53, *v54, *v55, *v56, *v57, *v58, *v59, *v60, *v61, *v62, *v63, *v64, *v65, *v66, *v67, *v68, *v69;

static int allb(const unsigned char *p, int v, size_t c)
{
    for (size_t i = 0; i < c; i++)
        if (p[i] != (unsigned char)v)
            return 0;
    return 1;
}

int main(void)
{
    unsigned char STACK_BUF[64];
    memset(STACK_BUF, 0, sizeof STACK_BUF);
    (void)STATIC_BUF; (void)allb;
    my_set_delay(64);
    v0 = my_malloc((size_t)10ull);
    v1 = my_malloc((size_t)10ull);
    v2 = my_malloc((size_t)10ull);
    v3 = my_malloc((size_t)10ull);
    v4 = my_malloc((size_t)10ull);
    v5 = my_malloc((size_t)10ull);
    v6 = my_malloc((size_t)10ull);
    v7 = my_malloc((size_t)10ull);
    v8 = my_malloc((size_t)10ull);
    v9 = my_malloc((size_t)10ull);
    v10 = my_malloc((size_t)10ull);
    v11 = my_malloc((size_t)10ull);
    v12 = my_malloc((size_t)10ull);
    v13 = my_malloc((size_t)10ull);
    v14 = my_malloc((size_t)10ull);
    v15 = my_malloc((size_t)10ull);
    v16 = my_malloc((size_t)10ull);
    v17 = my_malloc((size_t)10ull);
    v18 = my_malloc((size_t)10ull);
    v19 = my_malloc((size_t)10ull);
    v20 = my_malloc((size_t)10ull);
    v21 = my_malloc((size_t)10ull);
    v22 = my_malloc((size_t)10ull);
    v23 = my_malloc((size_t)10ull);
    v24 = my_malloc((size_t)10ull);
    v25 = my_malloc((size_t)10ull);
    v26 = my_malloc((size_t)10ull);
    v27 = my_malloc((size_t)10ull);
    v28 = my_malloc((size_t)10ull);
    v29 = my_malloc((size_t)10ull);
    v30 = my_malloc((size_t)10ull);
    v31 = my_malloc((size_t)10ull);
    v32 = my_malloc((size_t)10ull);
    v33 = my_malloc((size_t)10ull);
    v34 = my_malloc((size_t)10ull);
    v35 = my_malloc((size_t)10ull);
    v36 = my_malloc((size_t)10ull);
    v37 = my_malloc((size_t)10ull);
    v38 = my_malloc((size_t)10ull);
    v39 = my_malloc((size_t)10ull);
    v40 = my_malloc((size_t)10ull);
    v41 = my_malloc((size_t)10ull);
    v42 = my_malloc((size_t)10ull);
    v43 = my_malloc((size_t)10ull);
    v44 = my_malloc((size_t)10ull);
    v45 = my_malloc((size_t)10ull);
    v46 = my_malloc((size_t)10ull);
    v47 = my_malloc((size_t)10ull);
    v48 = my_malloc((size_t)10ull);
    v49 = my_malloc((size_t)10ull);
    v50 = my_malloc((size_t)10ull);
    v51 = my_malloc((size_t)10ull);
    v52 = my_malloc((size_t)10ull);
    v53 = my_malloc((size_t)10ull);
    v54 = my_malloc((size_t)10ull);
    v55 = my_malloc((size_t)10ull);
    v56 = my_malloc((size_t)10ull);
    v57 = my_malloc((size_t)10ull);
    v58 = my_malloc((size_t)10ull);
    v59 = my_malloc((size_t)10ull);
    v60 = my_malloc((size_t)10ull);
    v61 = my_malloc((size_t)10ull);
    v62 = my_malloc((size_t)10ull);
    v63 = my_malloc((size_t)10ull);
    v64 = my_malloc((size_t)10ull);
    v65 = my_malloc((size_t)10ull);
    v66 = my_malloc((size_t)10ull);
    v67 = my_malloc((size_t)10ull);
    v68 = my_malloc((size_t)10ull);
    v69 = my_malloc((size_t)10ull);
    my_free(v0);
    my_free(v1);
    my_free(v2);
    my_free(v3);
    my_free(v4);
    my_free(v5);
    my_free(v6);
    my_free(v7);
    my_free(v8);
    my_free(v9);
    my_free(v10);
    my_free(v11);
    my_free(v12);
    my_free(v13);
    my_free(v14);
    my_free(v15);
    my_free(v16);
    my_free(v17);
    my_free(v18);
    my_free(v19);
    my_free(v20);
    my_free(v21);
    my_free(v22);
    my_free(v23);
    my_free(v24);
    my_free(v25);
    my_free(v26);
    my_free(v27);
    my_free(v28);
    my_free(v29);
    my_free(v30);
    my_free(v31);
    my_free(v32);
    my_free(v33);
    my_free(v34);
    my_free(v35);
    my_free(v36);
    my_free(v37);
    my_free(v38);
    my_free(v39);
    my_free(v40);
    my_free(v41);
    my_free(v42);
    my_free(v43);
    my_free(v44);
    my_free(v45);
    my_free(v46);
    my_free(v47);
    my_free(v48);
    my_free(v49);
    my_free(v50);
    my_free(v51);
    my_free(v52);
    my_free(v53);
    my_free(v54);
    my_free(v55);
    my_free(v56);
    my_free(v57);
    my_free(v58);
    my_free(v59);
    my_free(v60);
    my_free(v61);
    my_free(v62);
    my_free(v63);
    my_free(v64);
    my_heap_dump();
    v1[0] = 0x00;
    my_free(v1);
    my_free(v65);
    my_set_delay(0);
    my_heap_dump();
    return 0;
}
