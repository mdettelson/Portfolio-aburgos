/* umsections.c
 *
 * James McCants and Andrew Burgos
 *
 * Handles the creation and manipulation of sections that the assembler emits
 * words to.
 */

#include <stdlib.h>

#include "umsections.h"
#include "atom.h"
#include "table.h"
#include "seq.h"
#include "assert.h"
#include "list.h"
#include "bitpack.h"

/* Umsections_T struct. It contains a Table with each key representing a unique
 * section in the assembler and each value being a sequence holding the words
 * associated with that section.
 */
struct Umsections_T {
        Table_T table;
        Seq_T order; 
        const char *section; /* current section */
        int (*err_func)(void *errstate, const char *message);
        void *errstate;
};

/* Umsections_new makes a new 'assembler' in the form of a Umsections_T. It
 * creates a new table and makes the first key value pair with the given
 * section. It also sets the assembler to emit to that section.
 */
Umsections_T Umsections_new(const char *section, 
                 int (*error)(void *errstate, const char *message),
                 void *errstate)
{
        Umsections_T assembler = malloc(sizeof(*assembler));
        assert(assembler);

        Table_T table = Table_new(100, NULL, NULL);
        assert(table);

        Seq_T order = Seq_new(100);
        assert(order);
        Seq_addhi(order, (void *)section);

        Seq_T instructions = Seq_new(100);
        assert(instructions);

        Table_put(table, Atom_string(section), instructions);

        /* intializes the struct */
        assembler->table = table;
        assembler->section = section;
        assembler->order = order;
        assembler->err_func = error;
        assembler->errstate = errstate;

        return assembler;
}

/* A Table_map apply function used to free the seqs stored in the table */
void apply_free(const void *key, void **value, void *cl)
{
        (void)key;
        (void)cl;

        Seq_T tmp = *value;
        Seq_free(&tmp);
}

/* Frees a given Umsections_T */
void Umsections_free(Umsections_T *asmp)
{
        Table_map((*asmp)->table, apply_free, NULL);

        Table_free(&((*asmp)->table));
        Seq_free(&((*asmp)->order));

        free(*asmp);
}

/* calls the error function with the given message */
int Umsections_error(Umsections_T asm, const char *msg)
{        
        return asm->err_func(asm->errstate, msg);
}

/* starts emitting to the given section, creates it if necessary */
void Umsections_section(Umsections_T asm, const char *section)
{
        if (Table_get(asm->table, section) == NULL) {
                Seq_T seq = Seq_new(100);
                Table_put(asm->table, Atom_string(section), seq);
                Seq_addhi(asm->order, (void *)section);
                asm->section = section;
        } else {
                asm->section = section;
        }
}

/* appends a word to the section currently being emitted to */
void Umsections_emit_word(Umsections_T asm, Umsections_word data)
{
        Seq_T current = Table_get(asm->table, asm->section);
        assert(current);

        Seq_addhi(current, (void *)(uintptr_t)data);
}

/* calls the given apply function each section name and passes the given cl */
void Umsections_map(Umsections_T asm, void apply(const char *name, void *cl), 
                    void *cl)
{
        int len = Seq_length(asm->order);

        for (int i = 0; i < len; i++) {                
                apply((const char *)Seq_get(asm->order, i), cl);
        }
}

/* helper function to check if a section exists */
inline void check_name(Umsections_T asm, Seq_T current)
{
        if (current == NULL) {
                const char *msg = "No such segment.\n";
                asm->err_func(asm->errstate, msg);
        };
}

/* helper function to check if an index is valid within a section */
inline void check_index(Umsections_T asm, Seq_T current, int i)
{      
        int len = Seq_length(current);
        if (i >= len || i < 0) {
                const char *msg = "No word at that index.\n";
                asm->err_func(asm->errstate, msg);
        }
}

/* returns the length of a given section */
int Umsections_length(Umsections_T asm, const char *name)
{
        Seq_T current = Table_get(asm->table, Atom_string(name));
        check_name(asm, current);

        return Seq_length(current);
}

/* returns a word from a section at a given index */
Umsections_word Umsections_getword(Umsections_T asm, const char *name, int i)
{      
        Seq_T current = Table_get(asm->table, Atom_string(name));
        check_name(asm, current);
        check_index(asm, current, i);
        
        return (Umsections_word)(uintptr_t)Seq_get(current, i);
}

/* puts a word into a section at a given index  */
void Umsections_putword(Umsections_T asm, const char *name, 
                        int i, Umsections_word w)
{
        Seq_T current = Table_get(asm->table, Atom_string(name));
        check_name(asm, current);
        check_index(asm, current, i);

        Seq_put(current, i, (void *)(uintptr_t)w);
}

/* goes through each section in order, writing each word to the given file */
void Umsections_write(Umsections_T asm, FILE *output)
{        
        int len = Seq_length(asm->order);
   
        for (int i = 0; i < len; i++) {
                Seq_T tmp = Table_get(asm->table, 
                                      (const char *)Seq_get(asm->order, i));
                int tmp_len = Seq_length(tmp);
               
                for (int j = 0; j < tmp_len; j++) {
                        Umsections_word word = 0;
                        word = (Umsections_word)(uintptr_t)Seq_get(tmp, j);
                  
                        for (int p = 3; p >= 0; p--) {
                                uint32_t c = Bitpack_getu(word, 8, 8 * p);
                                putc(c, output);
                        }
                }
        }
}

