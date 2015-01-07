/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                                 io                                *
 *                                                                   *
 *                File: io.c                                         *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: Header file for the io module, which         *
 *                      handles receiving input from standard        *
 *                      input and outputting values to standard      *
 *                      output                                       *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "assert.h"
#include "uarray.h"


extern void input  (unsigned ra, UArray_T registers);
extern void output (unsigned ra, UArray_T registers);
