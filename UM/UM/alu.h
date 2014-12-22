/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                               alu                                 *
 *                                                                   *
 *                File: alu.c                                        *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *        
 *             Purpose: Header for the ALU module, which             *
 *                      executes UM instructions involving           *
 *                      arithmetic and basic data manipulation       *
 *                      that does not involve accessing the          *
 *                      segmented memory.                            *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "assert.h"
#include "uarray.h"

extern void cond_move  (unsigned ra, unsigned rb,
                        unsigned rc, UArray_T registers);
extern void addition   (unsigned ra, unsigned rb, 
                        unsigned  rc, UArray_T registers);
extern void multiply   (unsigned ra, unsigned rb,
                        unsigned rc, UArray_T registers);
extern void nand       (unsigned ra, unsigned rb, 
                        unsigned rc, UArray_T registers);
extern void division   (unsigned ra, unsigned rb, 
                        unsigned rc, UArray_T registers);
extern void load_value (unsigned ra, uint32_t value, UArray_T registers);

