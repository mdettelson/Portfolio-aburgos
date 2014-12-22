/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                                 um                                *
 *                                                                   *
 *                File: um.c                                         *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: Initializes the UM and interacts with        *
 *                      various modules to execute                   *
 *                      UM instructions as well as interact with     *
 *                      the segmented memory.                        *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "uarray.h"
#include "bitpack.h"

/*   U M   M O D U L E S   */
#include "managemem.h"
#include "alu.h"
#include "io.h"
#include "decoder.h"


/* * * * * * * * * * * * * * * * * * * * * * * * *
 *   F U N C T I O N   D E C L A R A T I O N S   *
 * * * * * * * * * * * * * * * * * * * * * * * * */

static void initialize_registers (UArray_T registers);

static void read_file            (int argc, char *argv[], 
                                    UArray_T registers, Memory mem);

static void execute_instruction  (instruction decoded, UArray_T registers, 
                                  Memory mem, uint32_t *program_counter);

static void free_um_memory       (UArray_T registers, Memory mem);


/* * * * * * * * * * * * * * * * * * 
 *   I M P L E M E N T A T I O N   *
 * * * * * * * * * * * * * * * * * */

int main(int argc, char *argv[]) 
{
        /* initialize segmented memory */
        Memory mem = initialize_memory();        
        
        /* create registers */
        UArray_T registers = UArray_new(8, sizeof(uint32_t));  
       
        /* initialize program counter */
        uint32_t pc_value = 0;
        uint32_t *program_counter = &pc_value;
        
        read_file(argc, argv, registers, mem);

        initialize_registers(registers);

        /* load in 32-bit instructions */
        Um_instruction codeword = (fetch_instruction(program_counter, mem));  
        instruction decoded = malloc(sizeof(*decoded));
        decode(codeword, decoded);
        
               


        while ( decoded->opcode != HALT && (signed)decoded->opcode != -1 ) {
          

                execute_instruction(decoded, registers, mem, program_counter);
                codeword = (fetch_instruction(program_counter, mem));
              
              
                decode(codeword, decoded);

        }   
        free(decoded);
        free_um_memory(registers, mem);
   
        
  
}

static void initialize_registers(UArray_T registers) 
{
        int i;
        for (i = 0; i < UArray_length(registers); i++) {
                int *value = UArray_at(registers, i);
                *value = 0; 
        }
}


static void read_file(int argc, char *argv[], UArray_T registers, Memory mem) 
{
        if (argc != 2) {
                fprintf(stderr, "Error: please specify one file\n");
                exit(EXIT_FAILURE);
                free_um_memory(registers, mem);
        }


        FILE *file_ptr = fopen(argv[1], "r");

        struct stat file_stats;

        if(stat(argv[1], &file_stats) == -1) {
                fprintf(stderr, "Error within file\n");
                exit(EXIT_FAILURE);
                fclose(file_ptr);
                free_um_memory(registers, mem);
        }
        
        if (file_stats.st_size % 4 != 0) {
                fprintf(stderr, "Error: File does not contain");
                fprintf(stderr, "correctly formatted instruction\n");
                exit(EXIT_FAILURE);
                fclose(file_ptr);
                free_um_memory(registers, mem);
                
        }
        load_value(0, (file_stats.st_size / 4), registers);
        map_segment(1, 0, registers, mem);

        uint32_t instruct = 0;
        uint32_t instruct_byte;
        int instruction_count = 0;
        unsigned lsb = 32;

        while((instruct_byte = getc(file_ptr)) != (unsigned)EOF) {
           
                lsb = lsb - 8;
                instruct = Bitpack_newu(instruct, 8, lsb, instruct_byte);
                
                if (lsb == 0) {
                        load_value(0, instruct, registers);
                        int *value2 = UArray_at(registers, 2);
                        *value2 = instruction_count++;
                        segmented_store(1, 2, 0, registers, mem);    
                        lsb = 32;
                }
        }
        fclose(file_ptr);
}


static void free_um_memory(UArray_T registers, Memory mem) 
{
        UArray_free(&registers);
        free_memory(mem);
}

/* Executes UM instruction based off decoded opcode */
static void execute_instruction(instruction decoded, UArray_T registers, 
                                Memory mem, uint32_t *program_counter) 
{
        switch ( decoded->opcode ) {
                case 0:
                        cond_move(decoded->ra, decoded->rb, decoded->rc, 
                                  registers);
                        break;
                case 1:
                        segmented_load(decoded->ra, decoded->rb, decoded->rc,
                                       registers, mem);
                        break;
                case 2: 
                        segmented_store(decoded->ra, decoded->rb, decoded->rc,
                                        registers, mem);
                        break;
                case 3:
                        addition(decoded->ra, decoded->rb, decoded->rc, 
                                 registers);
                        break;
                case 4:
                        multiply(decoded->ra, decoded->rb, decoded->rc, 
                                 registers);
                        break;
                case 5: 
                        division(decoded->ra, decoded->rb, decoded->rc,
                                 registers);
                        break;
                case 6: 
                        nand(decoded->ra, decoded->rb, decoded->rc, registers);
                        break;
                case 7: 
                        return;
                case 8: 
                        map_segment(decoded->rb, decoded->rc, registers, mem);
                        break;
                case 9: 
                        unmap_segment(decoded->rc, registers, mem);
                        break;
                case 10: 
                        output(decoded->rc, registers);
                        break;
                case 11: 
                        input(decoded->rc, registers);
                        break;
                case 12: 
                        load_program(decoded->rb, decoded->rc, registers, 
                                     mem, program_counter); 
                        break;
                case 13: 
                        load_value(decoded->ra, decoded->value, registers);
                        break;
        }
        
        if (decoded->opcode != 12) {
                *program_counter = *program_counter + 1;
        
        }
}



