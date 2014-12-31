#include <stdlib.h>
#include <a2plain.h>
#include "uarray2.h"


struct a2 {
       A2Methods_smallapplyfun *apply;
       void *cl;
};

typedef void applyfun(int i, int j, UArray2_T array2, void *elem, void *cl);

typedef struct a2 *a2fun_closure;

static void inner_apply(int i, int j, UArray2_T array2, void *elem, void *cl);


static A2Methods_UArray2 new(int width, int height, int size) {
       return UArray2_new(width, height, size);
}

static A2Methods_UArray2 new_with_blocksize(int width,
                                            int height,
                                            int size,
                                            int blocksize)
{
       (void) blocksize;
       return UArray2_new(width, height, size);
}

static void a2free(A2Methods_UArray2 * array2p)
{
       UArray2_free((UArray2_T *) array2p);
}

static int width(A2Methods_UArray2 array2)
{
       return UArray2_width(array2);
}

static int height(A2Methods_UArray2 array2)
{
       return UArray2_height(array2);
}

static int size(A2Methods_UArray2 array2)
{
       return UArray2_size(array2);
}

static int blocksize(A2Methods_UArray2 array2)
{
       (void)array2;
       return 1;
}

static A2Methods_Object *at (A2Methods_UArray2 array2, int i, int j)
{
       return UArray2_at(array2, i, j);
}

static void map_row_major(A2Methods_UArray2 array2, A2Methods_applyfun apply, 
                  void *cl)
{
       UArray2_map_row_major(array2,(applyfun *) apply, cl);
}

static void map_col_major(A2Methods_UArray2 array2, A2Methods_applyfun apply,
                  void *cl)
{
       UArray2_map_col_major(array2,(applyfun *) apply, cl);
}

/* Small_map_row_major
 *
 * Maps to an index on the larger array via row major and maps through the 
 * array that index holds
 * 
 * Success: Returns nothing and frees closure
 */

static void small_map_row_major(A2Methods_UArray2 array2, 
                                A2Methods_smallapplyfun apply, void *cl)
{

       a2fun_closure closure;
       closure = malloc(sizeof(*closure));
       closure->apply = apply;
       closure->cl = cl;

       UArray2_map_row_major(array2, inner_apply, closure);

       free(closure);
}

/* Small_map_col_major
 *
 * Maps to an index on the larger array via col major and maps through 
 * the array that index holds
 * 
 * Success: returns nothing, frees closure
 */

static void small_map_col_major(A2Methods_UArray2 array2,
                                A2Methods_smallapplyfun apply, void *cl)
{
       a2fun_closure closure;
       closure = malloc(sizeof(*closure));
       closure->apply = apply;
       closure->cl = cl;

       UArray2_map_col_major(array2, inner_apply, closure);

       free(closure);
}

/* Inner_apply
 *
 * Serves as the apply function being called on the indices of the larger array
 * which hold array pointers as their elements
 *
 */
static void inner_apply(int i, int j, UArray2_T array2, void *elem, void *cl)
{       
       (void)array2;
       (void)i;
       (void)j;

       a2fun_closure closure = (a2fun_closure)cl;
       closure->apply(elem, closure->cl);

}


/* A2Methods_T
 * 
 * Struct that holds pointers to functions created in uarray2b
 */

static struct A2Methods_T uarray2_methods_plain_struct = {
       new,
       new_with_blocksize,
       a2free,
       width,
       height,
       size,
       blocksize,
       at,
       map_row_major,                   // map_row_major
       map_col_major,
       NULL,
       map_row_major,                   // map_col_major
       small_map_row_major,     // small_map_row_major
       small_map_col_major,    // small_map_col_major
       NULL,
       small_map_row_major,                  // small_map_default
};
// finally the payoff: here is the exported pointer to the struct
A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
