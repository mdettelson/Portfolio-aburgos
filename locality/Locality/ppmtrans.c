/* Authors: Chris Penny, Andrew Burgos
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "stack.h"
#include "list.h"

/* Main Functions */
Pnm_ppm read_file(int argc, char * argv[], A2Methods_T, FILE*);
void transform_image(int argc, char * argv[], Pnm_ppm image, A2Methods_T m);

/* Transformation Functions */
void rotate90(Pnm_ppm image, A2Methods_mapfun *map, A2Methods_T methods);
void map90(int i, int j, A2Methods_UArray2 a, void *el, void *cl);
void rotate180(Pnm_ppm image, A2Methods_mapfun *map, A2Methods_T methods);
void map180(int i, int j, A2Methods_UArray2 a, void *el, void *cl);
void rotate270(Pnm_ppm image, A2Methods_mapfun *map, A2Methods_T methods);
void map270(int i, int j, A2Methods_UArray2 a, void *el, void *cl);

/* Debugging Functions */
void print_array(int i, int j, A2Methods_UArray2 a, void *elem, void *cl);
void print_stack(Stack_T stack);

/* Functions related to alternate implementation */
void alternate_rotate90(Pnm_ppm, A2Methods_UArray2, A2Methods_T);
Stack_T reverse_stack(Stack_T stack);
void write_to_array(void *elem, void *cl);
void push_to_stack(void *elem, void *cl);

/* Default Closure Type */
typedef struct closure {
        A2Methods_T methods;
        Pnm_ppm image;
} *Closure_T;

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
        "[-{row,col,block}-major] [filename]\n",
                progname);
        exit(1);
}

int main(int argc, char *argv[]) 
{
        
        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        FILE *fp = stdin;

        Pnm_ppm image = read_file(argc, argv, methods, fp);
        transform_image(argc, argv, image, methods);     

        Pnm_ppmfree(&image);
        return 0;
}

/*  Called in transform_image.  It is slightly less efficient
 *  than reading it in through the loop below, but it allows
 *  for the file to be located anywhere in the arguments.
 *  We could not find any specific places in the specification that said
 *  the file must be the last argument, so we figured this was the best
 *  option.
 *
 *  Returns the Pnm_ppm image from ppmread() from the relevant file
 */
Pnm_ppm read_file(int argc, 
                  char * argv[],  
                  A2Methods_T methods,
                  FILE *fp)
{
        int i;
        Pnm_ppm image;

/* Access may be responsible for memory leaks
 *
 *
 */
        for (i = 1; i < argc; i++) {
                if( access(argv[i], R_OK) == 0 ) {
                        fp = fopen(argv[i], "rb");
                        break;
                }
        }
        
        image = Pnm_ppmread(fp, methods);
        assert(image);
        fclose(fp);
        return image;
        
}

/* File must be at end of arguments if not using stdin */
void transform_image(int argc, char * argv[], Pnm_ppm image, 
                     A2Methods_T methods)
{
        int i;
        int rotation = 0;

        bool flip = false;
        char *fliptype;
        bool rotate = false;
        bool transpose = false;

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

#define SET_METHODS(METHODS, MAP, WHAT) do {                            \
                methods = (METHODS);                                    \
                assert(methods != NULL);                                \
                map = methods->MAP;                                     \
                if (map == NULL) {                                      \
                        fprintf(stderr, "%s does not support "          \
                                        WHAT "mapping\n",               \
                                argv[0]);                               \
                        exit(1);                                        \
                }                                                       \
        } while (0)

        for ( i = 1; i < argc; i++ ) {
                if ( !strcmp(argv[i], "-row-major") ) {
                        SET_METHODS(uarray2_methods_plain, map_row_major,
                                    "row-major");
                        

                } else if ( !strcmp(argv[i], "-col-major") ) {
                        SET_METHODS(uarray2_methods_plain, map_col_major,
                                    "column-major");
                        

                } else if ( !strcmp(argv[i], "-block-major") ) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                       

                } else if ( !strcmp(argv[i], "-rotate") ) {
                        if (!(i + 1 < argc)) { //If last arg    
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90
                              || rotation == 180 || rotation == 270)) {
                                fprintf(stderr, "Rotation must be "
                                        "0, 90 180 or 270\n");
                                usage(argv[0]);
                        }

                        if (!(*endptr == '\0')) {   
                                usage(argv[0]);
                        }
                        rotate = true;

                } else if ( !strcmp(argv[i], "-flip") ) {
                        if (!(i + 1 < argc)) {   //If last arg
                                usage(argv[0]);
                        }
                        fliptype = argv[++i];
                        if( strcmp(fliptype, "horizontal") != 0 &&
                            strcmp(fliptype, "vertical") != 0 ) {
                                fprintf(stderr, "Flip type must be "
                                    "\"horizontal\" or \"vertical\"");
                                usage(argv[0]);
                        }

                        flip = true;
                        
                } else if ( !strcmp(argv[i], "-transpose") ) {
                        transpose = true;

                }else if ( *argv[i] == '-' ) {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                } else if ( argc - i > 1 ) { //at the end
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        //Do Nothing
                        /* Normally you would read in the file here, but
                           We decided to implement this differently. */
                }
        }
        
        /* Apply appropriate rotation */
        if ( rotate == true ) {
                if ( rotation == 90 ) {
                        rotate90(image, map, methods);
                } else if ( rotation == 180 ) {
                        rotate180(image, map, methods);
                } else if ( rotation == 270 ) {
                        rotate270(image, map, methods);
                } else if ( rotation == 0 ) {
                        Pnm_ppmwrite(stdout, image);
                }
        }

        /* We decided not to implement these optional components */
        if ( flip == true ) {
                fprintf(stderr, "Unsupported Transformation\n");
        }
        if (transpose == true) {
                fprintf(stderr, "Unsupported Transformation\n");
        }

}

/* Rotates the image by 90 degrees using the map90 function.
 * Prints the transformed image.
 */
void rotate90(Pnm_ppm image, A2Methods_mapfun *map, A2Methods_T methods)
{
        /* Swap dimensions */
        A2Methods_UArray2 new_pixels =  methods->new(image->height, 
                                                     image->width,
                                       image->methods->size(image->pixels));
                                       
        /* Pass the methods and the original image to the apply function */
        Closure_T closure;
        closure = malloc(sizeof(*closure));
        closure->methods = methods;
        closure->image = image;
 
        map(new_pixels, map90, closure);
        /* Print the transformed image */
        Pnm_ppm transform;
        transform = malloc(sizeof(*transform));
        transform->pixels      = new_pixels;
        transform->width       = methods->width(new_pixels);
        transform->height      = methods->height(new_pixels);
        transform->denominator = image->denominator;
        transform->methods     = methods;
 
        Pnm_ppmwrite(stdout, transform);
        Pnm_ppmfree(&transform);
        free(closure);
}

/*  Maps the pixels from the final transformed image to the original image.
 */
void map90(int i_f, int j_f, A2Methods_UArray2 new_array, 
           void *el, void *cl)
{
        Closure_T closure = (Closure_T)cl; 
        A2Methods_T methods = closure->methods;       
        Pnm_ppm image = closure->image;
        (void)methods;
        (void)new_array;
        
        int i_init = j_f;
        int j_init = image->methods->height(image->pixels) - 1 - i_f;

        Pnm_rgb old_pixel = image->methods->at(image->pixels, i_init, j_init);

        Pnm_rgb new_pixel = (Pnm_rgb)el;
        
        *new_pixel = *old_pixel;    
}

/* Rotates the image by 270 degrees using the map270 function.
 * Prints the transformed image.
 */
void rotate270(Pnm_ppm image, A2Methods_mapfun *map, 
               A2Methods_T methods)
{
        A2Methods_UArray2 new_pixels 
                =  methods->new(image->height, image->width,
                                image->methods->size(image->pixels));
        
        Closure_T closure;
        closure = malloc(sizeof(*closure));
        closure->methods = methods;
        closure->image = image;
        
        map(new_pixels, map270, closure);

        Pnm_ppm transform;
        transform = malloc(sizeof(*transform));
        transform->pixels      = new_pixels;
        transform->width       = methods->width(new_pixels);
        transform->height      = methods->height(new_pixels);
        transform->denominator = image->denominator;
        transform->methods     = methods;

        Pnm_ppmwrite(stdout, transform);
        Pnm_ppmfree(&transform);
        free(closure);
}

/*  Maps the pixels from the final transformed image to the original image.
 */
void map270(int i_f, int j_f, A2Methods_UArray2 new_array, 
            void *el, void *cl)
{
        Closure_T closure = (Closure_T)cl;
        Pnm_ppm image = closure->image;
        A2Methods_T methods = closure->methods;

        int j_init = i_f;
        int i_init = methods->height(new_array) - 1 - j_f;
 
        Pnm_rgb old_pixel = image->methods->at(image->pixels, i_init, j_init);
        Pnm_rgb new_pixel = (Pnm_rgb)el;
        
        *new_pixel = *old_pixel;
}

/* Rotates the image by 180 degrees using the map180 function.
 * Prints the transformed image.
 */
void rotate180(Pnm_ppm image, A2Methods_mapfun *map, 
               A2Methods_T methods)
{
        A2Methods_UArray2 new_pixels = methods->new(image->width,
                                                    image->height,
                                      image->methods->size(image->pixels));
        Closure_T closure;
        closure = malloc(sizeof(*closure));
        closure->methods = methods;
        closure->image = image;
        
        map(new_pixels, map180, closure);
        
        Pnm_ppm transform;
        transform = malloc(sizeof(*transform));
        transform->pixels      = new_pixels;
        transform->width       = methods->width(new_pixels);
        transform->height      = methods->height(new_pixels);
        transform->denominator = image->denominator;
        transform->methods     = methods;

        Pnm_ppmwrite(stdout, transform);
        Pnm_ppmfree(&transform);
        free(closure);
}

/*  Maps the pixels from the final transformed image to the original image.
 */
void map180(int i_f, int j_f, A2Methods_UArray2 new_array, 
            void *el, void *cl)
{
        Closure_T closure = (Closure_T)cl;
        Pnm_ppm image = closure->image;
        A2Methods_T methods = closure->methods;
 

        int i_init = methods->width(new_array) - i_f - 1;
        int j_init = methods->height(new_array) - j_f - 1;

        Pnm_rgb old_pixel = image->methods->at(image->pixels, i_init, j_init);
        Pnm_rgb new_pixel = (Pnm_rgb)el;
        
        *new_pixel = *old_pixel;
}

/* We thought of a clever way of manipulating the image rotation using a stack.
 * We would push images onto a stack using map col major and through a series 
 * of manipulations set up a stack where if you pop all the elements off it, 
 * the order would be a correctly rotated image.There were some drawbacks and 
 * we could not call map_block using the stack so we scrapped the idea. Heres 
 * the code as it stood. You can disregard this if it doesnt interest you
 *

void alternate_rotate90(Pnm_ppm original, 
              A2Methods_UArray2 new, 
              A2Methods_T methods)
{
        //int size = original->methods->size(original->pixels);
        int i;
        int new_width = methods->width(new);
 
        Stack_T initial = Stack_new();
        Stack_T final   = Stack_new();
        Stack_T temp    = Stack_new();
        


        //Push elements in col-major order to the stack
        methods->small_map_col_major(original->pixels, push_to_stack, initial);
 ;
        initial = reverse_stack(initial);
        
        while (Stack_empty(initial) != 1){      

                // Push each row onto stack, revering the order 
                for (i = 0; i < new_width; i++) {
                        Stack_push(temp, Stack_pop(initial));
                }
        
                // add reversed row to the final queue 
                for (i = 0; i < new_width; i++) {
                        Stack_push(final, Stack_pop(temp));
                }
                
        }

        final = reverse_stack(final);

        methods->small_map_row_major(new, write_to_array, final);
        Stack_free(&initial);
        Stack_free(&temp);
        Stack_free(&final);
}

void push_to_stack(void *elem, void *cl)
{
        Stack_T stack = (Stack_T)cl;
        Stack_push(stack, elem);
}

void write_to_array(void *elem, void *cl)
{
        Stack_T stack = (Stack_T)cl;
        Pnm_rgb pixel = (Pnm_rgb)elem;
        void *stack_elem = Stack_pop(stack);
        *pixel = *(Pnm_rgb)stack_elem;
}

Stack_T reverse_stack(Stack_T stack)
{
        Stack_T temp = Stack_new();
        while(Stack_empty(stack) == 0) {
                Stack_push(temp, Stack_pop(stack));
        }
        Stack_free(&stack);
        return temp;
}

void print_stack(Stack_T stack)
{
        Stack_T temp = Stack_new();
        int i = 0;
        while(Stack_empty(stack) == 0) {
                if(i % 132 == 0) printf("\n");
                void *s = Stack_pop(stack);
                printf("%d ", *(int*)s);
                Stack_push(temp, s);//Stack_pop(stack));
                i++;
        }
        while(Stack_empty(temp) == 0) {
                Stack_push(stack, Stack_pop(temp));
        }

}

void print_array(int i, int j, A2Methods_UArray2 a, void *elem, void *cl)
{       
        (void)j;
        (void)a;
        (void)cl;
        int current = *(int*)elem;
        if(i == 0) {
                printf("\n");
        }
        printf("%d ", current);
}
*/
