/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                                 io                                *
 *                                                                   *
 *                File: io.c                                         *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: Handles receiving input from standard        *
 *                      input and outputting values to standard      *
 *                      output                                       *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "io.h"

/* 
 * Gets characters from standard input and stores them in the designated
 * register
 */
extern void input(unsigned rc, UArray_T registers)
{
        uint32_t input_value = fgetc(stdin);
        
        if ( input_value == (unsigned)EOF ) {
                input_value = 0;
                input_value = ~input_value;
        } else {
                assert(input_value <= 255);
        }
        
        uint32_t *store_value  = UArray_at(registers, rc);
        *store_value = input_value;
}

/* 
 * Takes a character from the designated register and outputs it to standard
 * output
 */
extern void output(unsigned rc, UArray_T registers) 
{
        uint32_t *output_value = UArray_at(registers, rc);
    
        assert(*output_value <= 255);
        fputc(*output_value, stdout);
}