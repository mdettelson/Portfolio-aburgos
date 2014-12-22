/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
 *                           managemem                               *
 *                                                                   *
 *                File: managemem.c                                  *
 *             Authors: Andrew Burgos and Lucy Qin                   *
 *       Date Modified: November 13, 2014                            *
 *             Purpose: maintains the                                *
 *                      segmented memory as well as handling UM      *
 *                      executions and other functions               *
 *                      that access the memory                       *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "managemem.h"



/* * * * * * * * * * * * * * * * * * * * * * * * *
 *   S T R U C T U R E   D E C L A R A T I O N   *
 * * * * * * * * * * * * * * * * * * * * * * * * */

struct Memory {
        Seq_T segments;
        Seq_T unused_ids;
};




const unsigned INITAL_SEQUENCE_SIZE = 100;

/* * * * * * * * * * * * * * * * * * * * * * * *
 *   F U N C T I O N   D E C L A R A T I O N   *
 * * * * * * * * * * * * * * * * * * * * * * * */

static void initialize_segment(UArray_T segment); 
static UArray_T copy_segment(UArray_T copied_segment, UArray_T segment_zero);
static void addSequenceIndices(Memory mem, Um_segmentID nextID);


/* * * * * * * * * * * * * * * * * * 
 *   I M P L E M E N T A T I O N   *
 * * * * * * * * * * * * * * * * * */

extern Memory initialize_memory() {
        
        Memory mem = malloc(sizeof(*mem)); 
        mem->segments = Seq_new(INITAL_SEQUENCE_SIZE);
        assert(mem->segments);
        
        mem->unused_ids = Seq_new(INITAL_SEQUENCE_SIZE);
        assert(mem->unused_ids);
     
        addSequenceIndices(mem, 0);
        
        return mem;
        
}

/* Adds additional sequences once unused_id and segment sequences 
 * have been filled 
 */
void addSequenceIndices(Memory mem, Um_segmentID nextID) {

        unsigned i;     
        for ( i = 0; i < INITAL_SEQUENCE_SIZE; i++ ) {
                Seq_addhi(mem->segments, NULL);
        }
        
        for ( i = 0; i < INITAL_SEQUENCE_SIZE; i++ ) {       
                Seq_addhi(mem->unused_ids, (void *)(uintptr_t)nextID);
                nextID++;
        }
}

/* Retrieves 32 bit codeword from segment zero */
extern Um_instruction fetch_instruction(uint32_t *program_counter, Memory mem) 
{
        UArray_T segment_zero = Seq_get(mem->segments, 0);
        assert(*program_counter < (unsigned)UArray_length(segment_zero));
           
        Um_instruction instruct = *((Um_instruction*)UArray_at(segment_zero,
                                                            *program_counter));

                                                            
        return instruct;
}

/* Access segmented memory at segment b offset c and loads into register a*/
extern void segmented_load(unsigned ra, unsigned rb, unsigned rc, 
                           UArray_T registers, Memory mem) 
{
        word register_a = UArray_at(registers, ra); // value
        word register_b = UArray_at(registers, rb); // seg ID
        word register_c = UArray_at(registers, rc); // offset
        
        assert(*register_b < (unsigned)Seq_length(mem->segments));
        
        UArray_T segment = Seq_get(mem->segments, *register_b);
        
        assert(segment);        

        word value = UArray_at(segment, *register_c);
        
        *register_a = *value;    
}

/* Stores value at register c into segmented memory */
extern void segmented_store(unsigned ra, unsigned rb, unsigned rc, 
                            UArray_T registers, Memory mem)
{
        word register_a = UArray_at(registers, ra); // segment ID
        word register_b = UArray_at(registers, rb); // offset 
        word register_c = UArray_at(registers, rc); // value     
              
        assert(*register_a < (unsigned)Seq_length(mem->segments)); 
               
        UArray_T segment = Seq_get(mem->segments, *register_a);
        
        assert(segment);
        assert(*register_b < (unsigned)UArray_length(segment));
        word value = UArray_at(segment, *register_b);
        
        *value = *register_c;  
}

/* Creates a new segment with a number of words equal to the value in register 
 * a
 */
extern void map_segment(unsigned rb, unsigned rc, 
                        UArray_T registers, Memory mem)
{
        uint32_t *seg_length = UArray_at(registers, rc);
        Um_segmentID curr_ID;
    
        UArray_T new_segment = UArray_new(*seg_length, sizeof(*seg_length));
        assert(new_segment);        

        if( Seq_length(mem->unused_ids) == 1 ) {
                Um_segmentID nextID = Seq_length(mem->segments);
                addSequenceIndices(mem, nextID);
        }       
        curr_ID = (Um_segmentID)(uintptr_t)Seq_remlo(mem->unused_ids);
        
        initialize_segment(new_segment);
     
        Seq_put(mem->segments, curr_ID, new_segment);
      
               
        word rb_register = UArray_at(registers, rb);
        *rb_register = curr_ID;
        
       
}
/* initializes segment to hold words containing 0 */
static void initialize_segment(UArray_T segment) 
{
        uint32_t i;
        for ( i = 0; i < (uint32_t)UArray_length(segment); i++ ) {
                word value = UArray_at(segment, i);
                *value = 0;      
        }
}

/* Frees segmented memory associated with segID at register ra, then stores 
 * segID for later use 
 */
extern void unmap_segment(unsigned rc, UArray_T registers, Memory mem)
{
        
        
        Um_segmentID segID = *((Um_segmentID *)(UArray_at(registers, rc)));
  
        assert((segID < (unsigned)Seq_length(mem->segments)) && segID != 0 );
        
        UArray_T removed_segment = (UArray_T)Seq_get(mem->segments, segID);
        assert(removed_segment);

        UArray_free(&removed_segment);
        
        Seq_put(mem->segments, segID, NULL);

        Seq_addlo(mem->unused_ids, (void *)(uintptr_t)segID);        
}

/* Segmented memory associated with segID at register ra is duplicated and
 * stored into segment zero 
 */
extern void load_program(unsigned rb, unsigned rc, UArray_T registers, 
                        Memory mem, uint32_t *program_counter)
{                
        uint32_t *value_c = UArray_at(registers, rc);
        *program_counter = *value_c;
        
        Um_segmentID segID = *((Um_segmentID *)UArray_at(registers, rb));   
        
        if ( segID == 0 ) {
                return;
        }
        
        UArray_T copied_segment = (Seq_get(mem->segments, segID));
        assert(copied_segment);
        
        UArray_T segment_zero = (Seq_get(mem->segments, 0));
    
        if ( segment_zero != NULL ) {
                UArray_free(&segment_zero);
        }
        
        segment_zero = copy_segment(copied_segment, segment_zero);
        
}

/* Copies the value of one segment into segment zero */
static UArray_T copy_segment(UArray_T copied_segment, UArray_T segment_zero)
{
        int i;
        int segment_length = UArray_length(copied_segment);
        
        segment_zero = UArray_new(segment_length, UArray_size(copied_segment));
        
        for ( i = 0; i < segment_length; i++ ) {
                word value = *((word*)UArray_at(copied_segment, i));
               
                word *duplicate_value = (word *)UArray_at(segment_zero, i);
                *duplicate_value = value;
        }
        return segment_zero;
}

/* If you love it, set it free */
extern void free_memory(Memory mem) {
        

        int i;
        
        for (i = 0; i < Seq_length(mem->segments); i++) {
                if (Seq_get(mem->segments, i) != NULL) {
                        UArray_T segment = Seq_get(mem->segments, i); 
                        UArray_free(&segment);
                }
        }
        

        
        Seq_free(&(mem->segments));
        Seq_free(&(mem->unused_ids));
        
        free(mem);
}

