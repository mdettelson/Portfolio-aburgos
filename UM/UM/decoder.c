/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                            decoder                                *
 *                                                                   *
 *                File: decoder.c                                    *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: Decodes 32-bit UM instructions by            *
 *                      extracting the opcode and relevant           *
 *                      information about the registers based on     *
 *                      the opcode                                   *
 *                                                                   *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "decoder.h"

static void store_regs(uint32_t codeword, instruction decoded);
static void store_three_regs(uint32_t codeword, instruction decoded);
static void check_registers(instruction decoded, unsigned used_registers);


/* 
 * Extracts the opcode from the 32-bit UM instruction and calls the 
 * appropriate functions to store information about the registers 
 */
extern void decode(uint32_t codeword, instruction decoded) 
{
        uint32_t opcode = Bitpack_getu(codeword, 4, 28);
        assert(opcode < 14);
        
        decoded->opcode = opcode;

        if ( opcode >= 7 ) {
                store_regs(codeword, decoded);

        } else {
                store_three_regs(codeword, decoded);
        }
}

/* 
 * Stores information about the registers for instructions involving 
 * less than three registers 
 */
static void store_regs(uint32_t codeword, instruction decoded)
{
        
        if ( decoded->opcode == HALT ) {
                
                decoded->ra = -1;
                decoded->rb = -1;
                decoded->rc = -1;
        
        } else if ( decoded->opcode == MAPSEG || 
                    decoded->opcode == LOADPROG ) {
                /* instructions using two registers */
                decoded->ra = -1;
                decoded->rb = Bitpack_getu(codeword, 3, 3);
                decoded->rc = Bitpack_getu(codeword, 3, 0);
                check_registers(decoded, 2);
                
        } else if ( decoded->opcode == UNMAPSEG || 
                   decoded->opcode == IN || 
                   decoded->opcode == OUT ) {
                
                /* instructions using 1 register */
                decoded->ra = -1;
                decoded->rb = -1;
                decoded->rc = Bitpack_getu(codeword, 3, 0);
        
                check_registers(decoded, 1);
                
        } else if ( decoded->opcode == LOADVAL ) {
                decoded->ra = Bitpack_getu(codeword, 3, 25);
                assert(decoded->ra <= 7);
                // TODO: check value?!
                decoded->value = Bitpack_getu(codeword, 25, 0);
        }
}

/* Stores register info for UM instructions that involve three registers */
static void store_three_regs(uint32_t codeword, instruction decoded)
{
        decoded->ra = Bitpack_getu(codeword, 3, 6);
        decoded->rb = Bitpack_getu(codeword, 3, 3);
        decoded->rc = Bitpack_getu(codeword, 3, 0);
        check_registers(decoded, 3);
}

/* 
 * Checks that values stored are valid and that the registers accessed exist. 
 * Checks are completed based on the number of registers used by a UM
 * instruction
 */
static void check_registers(instruction decoded, unsigned used_registers) 
{
        /* checks registers based on the number of registers used */
        assert(decoded->rc <= 7);
        if ( used_registers == 1 ) {
                return;
        }
        
        assert(decoded->rb <= 7);
        if ( used_registers == 2 ) {
                return;
        }
        assert(decoded->ra <= 7);

}