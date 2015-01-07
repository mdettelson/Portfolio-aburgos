/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                               alu                                 *
 *                                                                   *
 *                File: alu.c                                        *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *        
 *             Purpose: Executes UM instructions involving           *
 *                      arithmetic and basic data manipulation       *
 *                      that does not involve accessing the          *
 *                      segmented memory.                            *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "alu.h"


extern void cond_move(unsigned ra, unsigned rb, unsigned rc, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t*)UArray_at(registers, ra);
        uint32_t *reg_b = (uint32_t*)UArray_at(registers, rb);
        uint32_t *reg_c = (uint32_t*)UArray_at(registers, rc);
        
        if ( *reg_c != 0 ) {
                *reg_a = *reg_b;
        }
        
                
}

extern void addition(unsigned ra, unsigned rb, unsigned  rc, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t*)UArray_at(registers, ra);
        uint32_t *reg_b = (uint32_t*)UArray_at(registers, rb);
        uint32_t *reg_c = (uint32_t*)UArray_at(registers, rc);
        
        uint32_t addition_value = (*reg_b + *reg_c);
        *reg_a = addition_value;
        
}
extern void multiply(unsigned ra, unsigned rb, unsigned rc, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t*)UArray_at(registers, ra);
        uint32_t *reg_b = (uint32_t*)UArray_at(registers, rb);
        uint32_t *reg_c = (uint32_t*)UArray_at(registers, rc);
        
        uint32_t multi_value = (*reg_b * *reg_c);
        *reg_a = multi_value;
        
        
}

extern void nand(unsigned ra, unsigned rb, unsigned rc, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t*)UArray_at(registers, ra);
        uint32_t *reg_b = (uint32_t*)UArray_at(registers, rb);
        uint32_t *reg_c = (uint32_t*)UArray_at(registers, rc);

        uint32_t nand_value = ~(*reg_b & *reg_c);
        *reg_a = nand_value;
        
}

extern void division(unsigned ra, unsigned rb, unsigned rc, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t*)UArray_at(registers, ra);
        uint32_t *reg_b = (uint32_t*)UArray_at(registers, rb);
        uint32_t *reg_c = (uint32_t*)UArray_at(registers, rc);
        
        assert(*reg_c != 0);
        uint32_t divison_value = *reg_b / *reg_c;
        *reg_a = divison_value;
}

extern void load_value(unsigned ra, uint32_t value, UArray_T registers)
{
        uint32_t *reg_a = (uint32_t *)UArray_at(registers, ra);
        *reg_a = value; 
}