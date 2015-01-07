/* ummacros.c
 * 
 * James McCants and Andrew Burgos
 *
 * Contains the macro function implementations for our assembler.
 */

#include "ummacros.h"
#include "bitpack.h"

/*-----========================-----*/
/*-----=== HELPER FUNCTIONS ===-----*/
/*-----========================-----*/

/* a helper function that creates a word for a normal instruction */
uint32_t instr_word(unsigned op, unsigned A, unsigned B, unsigned C)
{
        uint32_t word = 0;
        word = Bitpack_newu(word, 4, 28, op);
        word = Bitpack_newu(word, 3, 6, A);
        word = Bitpack_newu(word, 3, 3, B);
        word = Bitpack_newu(word, 3, 0, C);
        return word;
}

/* a helper function that creates a word for a load_val instruction */
uint32_t load_word(unsigned op, unsigned A, unsigned val)
{
        uint32_t word = 0;
        word = Bitpack_newu(word, 4, 28, op);
        word = Bitpack_newu(word, 3, 25, A);
        word = Bitpack_newu(word, 25, 0, val);
        return word;
}

/* a helper function to check if a given temporary register is valid */
void check_tmp(Umsections_T asm, int tmp) {
        if (tmp == -1) {
                const char *msg = "Temp required but not given.\n";
                Umsections_error(asm, msg);
        }
}

/*-----=======================-----*/
/*-----=== MACRO FUNCTIONS ===-----*/
/*-----=======================-----*/

/* moves the val in reg B to reg A */
void move(Umsections_T asm, unsigned A, unsigned B)
{
        /*  if x = y -> x = y  */
        if (A == B) return;

        /*  y = y + 0 */
        Umsections_emit_word(asm, load_word(LV, A, 0));
        Umsections_emit_word(asm, instr_word(ADD, A, A, B));
}

/* moves the bit_comp of val in reg B into reg A */
void bitwise_compliment(Umsections_T asm, unsigned A, unsigned B)
{
        /*  ~x = ~(x ^ x)  */
        Umsections_emit_word(asm, instr_word(NAND, A, B, B));
}

/* moves the negative of the value in reg B into reg A */
void neg_twos_comp(Umsections_T asm, unsigned A, unsigned B, int tmp)
{
        /* put bit_comp of B into A */
        Umsections_emit_word(asm, instr_word(NAND, A, B, B));

        if (A == B) { /* need a temporary register */
                check_tmp(asm, tmp);
                B = tmp;
        }

        /*  -x % 2^32 = ~x + 1  */
        Umsections_emit_word(asm, load_word(LV, B, 1));
        Umsections_emit_word(asm, instr_word(ADD, A, A, B));
}

void sub(Umsections_T asm, unsigned A, unsigned B, unsigned C, int tmp)
{
        if (B == C) {
                /*  0 = x - x  */
                Umsections_emit_word(asm, load_word(LV, A, 0));
                return;
        }
        
        /*  (x - y) % 2^32 = (x + ~x + 1) % 2^32  */
        if (A == B) {
                check_tmp(asm, tmp); /* need a temporary register */

                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, load_word(LV, tmp, 1));
                Umsections_emit_word(asm, instr_word(ADD, C, tmp, C));
                Umsections_emit_word(asm, instr_word(ADD, A, B, C));

                /* back to intial state */
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
        } else if (A == C) {
                check_tmp(asm, tmp); /* need a temporary register */

                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, load_word(LV, tmp, 1));
                Umsections_emit_word(asm, instr_word(ADD, C, tmp, C));
                Umsections_emit_word(asm, instr_word(ADD, A, B, C));
        } else {
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, load_word(LV, A, 1));
                Umsections_emit_word(asm, instr_word(ADD, C, A, C));
                Umsections_emit_word(asm, instr_word(ADD, A, B, C));

                /* back to intial state */
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
        }
}

/*  stores the bitwise and of the values in regs B and C in reg A */
void bitwise_and(Umsections_T asm, unsigned A, unsigned B, unsigned C)
{
        /*  x ^ y = ~(~(x ^ y) ^ ~(x ^ y))  */
        Umsections_emit_word(asm, instr_word(NAND, B, B, C));
        Umsections_emit_word(asm, instr_word(NAND, A, B, B));
}

void bitwise_or(Umsections_T asm, unsigned A, unsigned B, unsigned C)
{
        /*  x = x or x  */
        if (A == B && B == C) return;

        /*  x or y = ~(~x ^ ~y)  */
        if (B == C) {
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, instr_word(NAND, A, C, C));

                /* back to inital state */
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
        } else if (A == B) {
                Umsections_emit_word(asm, instr_word(NAND, B, B, B));
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, instr_word(NAND, A, B, C));

                /* back to initial state */
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
        } else if (A == C) {
                Umsections_emit_word(asm, instr_word(NAND, B, B, B));
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, instr_word(NAND, A, B, C));

                /* back to initial state */
                Umsections_emit_word(asm, instr_word(NAND, B, B, B));
        } else {
                Umsections_emit_word(asm, instr_word(NAND, B, B, B));
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
                Umsections_emit_word(asm, instr_word(NAND, A, B, C));

                /* back to initial state */
                Umsections_emit_word(asm, instr_word(NAND, B, B, B));
                Umsections_emit_word(asm, instr_word(NAND, C, C, C));
        }
}

/*-----===========================-----*/
/*-----=== ASSEMBLER FUNCTIONS ===-----*/
/*-----===========================-----*/

/* calls the correct macro based on the opcode */
void Ummacros_op(Umsections_T asm, Ummacros_Op operator, int temporary,
                 Ummacros_Reg A, Ummacros_Reg B, Ummacros_Reg C)
{
        switch(operator) 
        {
        case 14:
                move(asm, A, B);
                break;
        case 15:
                bitwise_compliment(asm, A, B);
                break;
        case 16:
                neg_twos_comp(asm, A, B, temporary);
                break;
        case 17:
                sub(asm, A, B, C, temporary);
                break;
        case 18:
                bitwise_and(asm, A, B, C);
                break;
        case 19:
                bitwise_or(asm, A, B, C);
                break;
        default:
                break;
        }
}

/* loads value k into register A */
void Ummacros_load_literal(Umsections_T asm, int tmp, 
                           Ummacros_Reg A, uint32_t k)
{
        if (k < 33554433) {
                Umsections_emit_word(asm, load_word(LV, A, k));
        } else {
                uint32_t comp = ~k;

                if (comp < 33554433) {                        
                        Umsections_emit_word(asm, load_word(LV, A, comp));
                        Umsections_emit_word(asm, instr_word(NAND, A, A, A));
                } else {
                        check_tmp(asm, tmp);

                        fprintf(stderr, "YAY\n");

                        uint32_t high_end = Bitpack_getu(k, 25, 7);
                        uint32_t low_end = Bitpack_getu(k, 7, 0);
                        
                        Umsections_emit_word(asm, load_word(LV, tmp, 1 << 7));
                        Umsections_emit_word(asm, load_word(LV, A, high_end));
                        Umsections_emit_word(asm, instr_word(MUL, A, 
                                                             A, tmp));
                        Umsections_emit_word(asm, 
                                             load_word(LV, tmp, low_end));
                        Umsections_emit_word(asm, instr_word(NAND, A, A, A));
                        Umsections_emit_word(asm, 
                                             instr_word(NAND, tmp, tmp, tmp));
                        Umsections_emit_word(asm, instr_word(NAND, A, A, tmp));
                        return;
                }
        }       
}


