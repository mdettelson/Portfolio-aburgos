/* UArray2b.c
 * Authors: Chris Penny, Andrew Burgos
 *
 */
 
#include "uarray2b.h"
#include "uarray.h"
#include "uarray2.h"
#include "assert.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"
#include "math.h"

#define T UArray2b_T

struct T {
        UArray2_T array;
        int height;
        int width;
        int blocksize;
        int size;
};

typedef struct CL {
        void (*apply) (int i, int j, UArray2b_T array2, void *elem, void *cl);
        void *cl;
        int block_i;
        int block_j;
        T block_array;
} *Closure_T;

void create_blocks(int i, int j, UArray2_T block, void *elems, void *cl);
void free_blocks(int i, int j, UArray2_T block, void *elems, void *cl);
void map_blocks(UArray2_T array2, int w, int h, int block_i, 
                        int block_j, int blocksize, void *cl);
void map_cells(int i, int j, UArray2_T array2, void *elem, void *cl);
unsigned next_pow_2(unsigned x);

typedef struct Info {
        int blocksize;
        int size;
} *Info;

/* UArray2b_new
 *
 * Allocates a UArray2 array that holds UArray2 arrays within its indices
 * that is created blocked 
 *
 * Success: returns UArray2 array holding UArray2 blocked arrays 
 *
 */

T UArray2b_new(int width, int height, int size, int blocksize) 
{
        assert(width > 0 && height > 0 && size > 0 && blocksize > 0);

        T new_array;
        new_array = malloc(sizeof(*new_array));

        new_array->width = width;
        new_array->height = height;
        new_array->size = size;
        new_array->blocksize = blocksize;

        int block_width = ceil((float)width / blocksize);
        int block_height = ceil((float)height / blocksize);
        int nbytes = blocksize * blocksize * size;

        new_array->array = UArray2_new(block_width, block_height,
                                       nbytes);

        Info blockinfo;
        blockinfo = malloc(sizeof(*blockinfo));
        blockinfo->blocksize = blocksize;
        blockinfo->size = size;
        
        UArray2_map_row_major(new_array->array, create_blocks,
                              blockinfo);
        free(blockinfo);

        return new_array;
}

/* Apply function to create blocks in main array */
void create_blocks(int i, int j, UArray2_T block, void *elems, void *cl)
{
        Info info = (Info)cl;
        (void)i;
        (void)j;
        (void)block;
        UArray2_T *current = (UArray2_T*)elems;

        *current = UArray2_new(info->blocksize, 
                                      info->blocksize, info->size);
}

/* UArray2b_new_64k_block
 *
 * new blocked 2d array: blocksize as large as possible provided
 * block occupies at most 64KB (if possible)
 * 
 * Success: returns array used as inner array with correct block formats
 *
 */

T UArray2b_new_64K_block(int width, int height, int size)
{
        assert(height > 0);
        assert(width > 0);
        assert(size > 0);

        const int MAX_SIZE = 64000;
        int area = width * height;
        int total_size = area * size;   
        int blocksize;

        if( total_size > MAX_SIZE ) {
                /* with size of 12, we calculated this as
                   the correct blocksize */
                blocksize = 64;
        } else {
                blocksize = ceil(sqrt(area));
        }

        return UArray2b_new(width, height, size, blocksize);
}

/* Found online - returns the next highest power of two */
unsigned next_pow_2(unsigned x)
{
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x++;
        
        return x;
}

/* UArray2b_free
 *
 * Frees the outer arra 0 errors from 0 contexts (suppressed: 6 from 6)
 *
 * Success: array2b which holds the inner arrays within its indices is freed
 *
 */

void UArray2b_free  (T *array2b)
{
        assert(array2b != NULL);
        assert(*array2b != NULL);
        UArray2_map_row_major((*array2b)->array, free_blocks, NULL);

        UArray2_free(&((*array2b)->array));

        free(*array2b);

        *array2b = NULL;
}

/* Free_blocks
 * 
 * Goes into each index in the larger array and frees the UArray2 that is 
 * being pointed to by the larger array
 *
 * Success: Elems in the inner UArray2 are freed
 *
 */

void free_blocks(int i, int j, UArray2_T array, void *elems, void *cl)
{
        (void)i;
        (void)j;
        (void)array;
        (void)cl;

        UArray2_T *block = (UArray2_T*)elems;
        UArray2_free(block);
}

int UArray2b_width (T array2b)
{
        assert(array2b != NULL);

        return array2b->width;
}

int UArray2b_height(T array2b)
{
        assert(array2b != NULL);

        return array2b->height;  
}

int UArray2b_size  (T array2b)
{
        assert(array2b != NULL);

        return array2b->size;   
}

int UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);

        return array2b->blocksize;   
}

/* UArray2b_at
 *
 * Success: return a pointer to the cell in column i, row j;
 * index out of range is a checked run-time error
 * 
 * Failure: results from a null array or invalid indices being called
 *
 */

void *UArray2b_at(T array2b, int i, int j)
{
 
        assert(array2b != NULL && i >= 0 && i < array2b->width &&
                                   j >= 0 && j < array2b->height);

        int block_i = floor(i / array2b->blocksize);
        int block_j = floor(j / array2b->blocksize);

        UArray2_T block = UArray2_at(array2b->array, block_i, block_j);

        int cell_i = i % array2b->blocksize;
        int cell_j = j % array2b->blocksize;

        return UArray2_at(block, cell_i, cell_j);
}

/* UArray2b_map
 *
 * For loops through the outer array, calling map_blocks on each indice it 
 * passes over in the outer array. Allocates struct Closure to be passed
 * into map_block
 *
 * Success: Returns nothing, applies on each index, frees closure
 *
 */

void  UArray2b_map(T array2b,
                   void apply(int i, int j, T array2b, void *elem, void *cl),
                   void *cl)
{

        Closure_T closure;
        UArray2_T *block;
        int w = array2b->width;
        int h = array2b->height;
        int blocksize = array2b->blocksize;
        int n_blocks_w = ceil((float)w / blocksize);
        int n_blocks_h = ceil((float)h / blocksize);

        closure = malloc(sizeof(*closure));
        closure->apply = apply;
        closure->cl = cl;
        closure->block_array = array2b;
 
        for( int block_i = 0; block_i < n_blocks_w; block_i++ ) {
                for( int block_j = 0; block_j < n_blocks_h; block_j++ ) {
                        closure->block_i = block_i;
                        closure->block_j = block_j;
                        block = UArray2_at(array2b->array, block_i,
                                           block_j);
                        map_blocks(*block, w, h, block_i, block_j,
                                   blocksize, closure);
                }
        }
        
        free(closure);
}

/* Map_blocks
 *
 * Mapping function used to iterate through inner UArray2
 *
 * Success: Returns nothing, maps to each cell within inner array and calls
 * apply function
 * 
 * Failure: Results from a null array being passed in
 *
 */

void map_blocks(UArray2_T array2, int w, int h, int block_i, 
                int block_j, int blocksize, void *cl)
{
        assert(array2);
        int end_i = blocksize;
        int end_j = blocksize;
 
        if ( block_i * blocksize + blocksize > w ) {
                end_i = (w-1) % blocksize;
        }
        if ( block_j * blocksize + blocksize > h ) {
                end_j = (h-1) % blocksize;
        }

        for (int j = 0; j < end_j; j++) {
                for (int i = 0; i < end_i; i++) {
                        map_cells(i, j, array2, 
                                  UArray2_at(array2, i, j), cl);
                }
        }
}

/* Map_cells
 *
 * Iterates through each cell within a block. Will not go further than 
 * array width as to avoid iterating through empty cells
 *
 * Success: each cell has applied called on it
 *
 */


void map_cells(int cell_i, int cell_j, UArray2_T block, 
                   void *block_elem, void *cl)
{       
        (void)block;
        Closure_T closure = (Closure_T)cl;
        int block_i = closure->block_i;
        int block_j = closure->block_j;
        int blocksize = closure->block_array->blocksize;
        int true_i = block_i * blocksize + cell_i;
        int true_j = block_j * blocksize + cell_j;
 
        if( (true_i >= closure->block_array->width) ||
            (true_j >= closure->block_array->height) ) {
                return;
        }
        
        closure->apply(true_i, true_j, 
                       closure->block_array, block_elem, closure->cl);
}

