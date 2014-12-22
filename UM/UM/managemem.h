/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                           managemem                               *
 *                                                                   *
 *                File: managemem.c                                  *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: Header file for the managemem module,        *
 *                      which is responsible for maintaining the     *
 *                      segmented memory as well as handling UM      *
 *                      executions and other functions               *
 *                      that access the memory                       *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * *
 *   D I R E C T I V E S   *
 * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uarray.h"
#include "seq.h"
#include "assert.h"
#include "bitpack.h"


typedef uint32_t Um_instruction;
typedef uint32_t *word;
typedef uint32_t Um_segmentID;

typedef struct Memory *Memory;


// TODO: MOVE THIS STRUCTURE DECLARATION




/* * * * * * * * * * * * * * * * * * * * * * * * *
 *   F U N C T I O N   D E C L A R A T I O N S   *
 * * * * * * * * * * * * * * * * * * * * * * * * */

extern Memory initialize_memory();

extern Um_instruction fetch_instruction(uint32_t *program_counter, Memory mem);


/*   U M   I N S T R U C T I O N S   */

extern void segmented_load  (unsigned ra, unsigned rb, unsigned rc, 
                             UArray_T registers, Memory mem);

extern void segmented_store (unsigned ra, unsigned rb, unsigned rc, 
                             UArray_T registers, Memory mem);

extern void map_segment     (unsigned ra, unsigned rb, 
                             UArray_T registers, Memory mem);

extern void unmap_segment   (unsigned ra, UArray_T registers, Memory mem);

extern void load_program    (unsigned ra, unsigned rb, UArray_T registers, 
                             Memory mem, uint32_t *program_counter);

extern void free_memory     (Memory mem);