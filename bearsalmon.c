//
//  bearsalmon.c
//  Bearsalmon
//  Referenced: "Designing Audio Objects for Max/MSP and Pd" by Eric Lyon
//
//  Created by Aren Akian on 3/9/18.
//  Copyright © 2018 Aren Akian. All rights reserved.
//


#include "m_pd.h"
#include <math.h>
#include <string.h>
#include "stdlib.h"

static t_class *bearsalmon_class;


typedef struct _bearsalmon{
    t_object    x_obj;
    
    
    t_symbol    *a_name; // name of the first buffer
    t_garray    *a_buffer;
    long        a_valid; // is the buffer valid? 1 or 0
    long        a_frames; // the number of elements in the buffer
    float       *a_samples; // the array of samples
    float       a_sr; // sampling rate

    
    t_symbol    *b_name; // name of the second buffer
    t_garray    *buffer;
    long        b_valid;
    long        b_frames;
    float       *b_samples;
    float       b_sr; // sampling rate

    
} t_bearsalmon;


void *bearsalmon_new(t_symbol *sA, t_symbol *sB) {
    t_bearsalmon *x = (t_bearsalmon *) pd_new(bearsalmon_class);
    
    x->a_name = sA;
    x->b_name = sB;
    
    post( "bearsalmon • Object was created");
    
    return x;
    
}

void bearsalmon_free(t_bearsalmon *x)
{
    /* Print message to Max window */
    post("bearsalmon • Object was deleted");
}


int bearsalmon_attach_buffer(t_bearsalmon *x)
{
    
    t_symbol *a_name;
    float *a_samples;
    int a_frames;
    
    t_symbol *b_name;
    float *b_samples;
    int b_frames;
   
    a_name = x->a_name;
    b_name = x->b_name;

    x->b_valid = 0;
    x->a_valid = 0;

    /* Check validity of the arrays */
    if (!(x->a_buffer = (t_garray *)pd_findbyclass(a_name, garray_class))) { // A
        if (a_name->s_name) {
            post("checking ArrayA... • \"%s\" is not a valid buffer", x->a_name->s_name);
        }
        return (int)x->b_valid;
    }
    if (!(x->buffer = (t_garray *)pd_findbyclass(b_name, garray_class))) { //B
        if (b_name->s_name) {
            post("checking ArrayB... • \"%s\" is not a valid buffer", x->b_name->s_name);
        }
        return (int)x->b_valid;
    }
    
    
    /* ******* */
    if (!garray_getfloatwords(x->a_buffer, &a_frames, &a_samples)) {
        post("Attaching ArrayA... • \"%s\" is not a valid buffer", x->a_name->s_name);
        return (int)x->a_valid;
    } else {
        x->a_valid = 1;
        x->a_frames = (long)a_frames;
        x->a_samples = a_samples;
        x->a_sr = sys_getsr();
        
        if (x->a_sr <= 0) {
            x->a_sr = 44100.0;
        }
    }
    
    if (!garray_getfloatwords(x->buffer, &b_frames, &b_samples)) {
        post("Attaching ArrayB... • \"%s\" is not a valid buffer", x->b_name->s_name);
        return (int)x->b_valid;
    } else {
        x->b_valid = 1;
        x->b_frames = (long)b_frames;
        x->b_samples = b_samples;
        x->b_sr = sys_getsr();
        
        if (x->b_sr <= 0) {
            x->b_sr = 44100.0;
        }
    }

    return (int)(x->b_valid && x->a_valid);
}


int bearsalmon_attach_any_buffer(t_garray **destbuf, t_symbol *b_name)
{
    if (!(*destbuf = (t_garray *)pd_findbyclass(b_name, garray_class))) {
        if (b_name->s_name) {
            post("bearsalmon • \"%s\" is not a valid buffer", b_name->s_name);
        }
        return 0;
    }
    return 1;
}


// arguments
// arraySel - the array to perform the cut on
//          - since bearsalmon stores TWO arrays, arraySel = 0 performs the cut on arrayA
//                                                arraySel = 1 performs the cut on ArrayB
void bearsalmon_cut(t_bearsalmon *x, t_floatarg arraySel, t_floatarg start, t_floatarg end)
{
    if (!bearsalmon_attach_buffer(x)) {
        error("attach_buffer() failed");
        return;
    }
    
    if ((!x->b_valid) ||(!x->a_valid)) {
        post("bearsalmon • Not a valid buffer!");
        return;
    }
    
    long startframe = start;
    long endframe = end;
    long cutframes = endframe - startframe;
    
    
    
    long chunksize = 2*cutframes * sizeof(float);

    if (arraySel > 0) {

        if (startframe < 0 || endframe > x->b_frames || startframe > endframe) {
            post("bearsalmon • %.0f and %.0f are not valid cut times", start, end);
            return;
        }
        memset(x->b_samples + (2*startframe), 0, chunksize);
    }
    if (arraySel == 0){

        if (startframe < 0 || endframe > x->a_frames || startframe > endframe) {
            post("bearsalmon • %.0f and %.0f are not valid cut times", start, end);
            return;
        }
        memset(x->a_samples + (2*startframe), 0, chunksize);

    }
    
    
    garray_redraw(x->buffer);
    garray_redraw(x->a_buffer);

}


// swap the range of samples from start to end in the two arrays
void bearsalmon_swap(t_bearsalmon *x, t_floatarg start, t_floatarg end)
{
    if (!bearsalmon_attach_buffer(x)) {
        error("attach_buffer() failed");
        return;
    }
    
    if ((!x->b_valid) ||(!x->a_valid)) {
        post("bearsalmon • Not a valid buffer!");
        return;
    }
    
    if (start < 0 || end  > x->b_frames || end > x->a_frames || start > end ) {
        post("bearsalmon • %.0f and %.0f are not valid cut times", start, end);
        return;
    }
    
    long startframe = start;
    long endframe = end;
    long swapframes = endframe - startframe; // same as bufferlength
                                             // the number of frames to swap
   
    
    long chunksize = 2*swapframes * sizeof(float);
    
    
    float *local_buffer = getbytes(chunksize);
    if (local_buffer == NULL) {
        error("bearsalmon • Cannot allocate memory for swap");
        return;
    } else {
        startframe = startframe*2;
        memcpy(local_buffer, x->b_samples+ startframe, chunksize);
        memcpy(x->b_samples + startframe, x->a_samples + startframe, chunksize);
        memcpy(x->a_samples + startframe, local_buffer, chunksize);

    }
    
    freebytes(local_buffer, chunksize);

    garray_redraw(x->buffer);
    garray_redraw(x->a_buffer);
    
}

// like swap(), but removed the gui redraw() function
// for use in the shuffle() function
void bearsalmon_quickswap(t_bearsalmon *x, t_floatarg start, t_floatarg end)
{
    // if you have problems remove this
    if (!bearsalmon_attach_buffer(x)) {
        error("attach_buffer() failed");
        return;
    }
    
    if ((!x->b_valid) ||(!x->a_valid)) {
        post("bearsalmon • Not a valid buffer!");
        return;
    }
    
    if (start < 0 || end  > x->b_frames || end > x->a_frames || start > end ) {
        post("bearsalmon • %.0f and %.0f are not valid cut times", start, end);
        return;
    }
    
    long startframe = start;
    long endframe = end;
    long swapframes = endframe - startframe;
   
    
    long chunksize = 2*swapframes * sizeof(float);
    
    
    float *local_buffer = getbytes(chunksize);
    if (local_buffer == NULL) {
        error("bearsalmon • Cannot allocate memory for swap");
        return;
    } else {
        startframe = startframe*2;
        memcpy(local_buffer, x->b_samples+ startframe, chunksize);
        memcpy(x->b_samples + startframe, x->a_samples + startframe, chunksize);
        memcpy(x->a_samples + startframe, local_buffer, chunksize);
        
    }
    
    freebytes(local_buffer, chunksize);
    
    
}

void bearsalmon_shuffle(t_bearsalmon *x, t_floatarg width){
    if (!bearsalmon_attach_buffer(x)) {
        error("attach_buffer() failed");
        return;
    }
    
    if ((!x->b_valid) ||(!x->a_valid)) {
        post("bearsalmon • Not a valid buffer!");
        return;
    }
    
    long a_length = x->a_frames;
    long b_length = x->b_frames;
    long length = fminl(a_length, b_length);
    
    for( int i = 0; (i < length) && (i+width < length); i = i+width){
        bearsalmon_quickswap(x, i, i + width);
        i = i+width; // so that every other will be swapped
    }
    
    garray_redraw(x->buffer);
    garray_redraw(x->a_buffer);
    
}

void bearsalmon_info( t_bearsalmon *x){
    if ( !(bearsalmon_attach_buffer(x)) ) {
        return;
    }
    
    post("bearsalmon • Information:");
    post("Array A:");
    post("     buffer name: %s", x->a_name->s_name);
    post("     frame count: %d", x->a_frames);
    post("     validity %d", x->a_valid);
    
    post("Array B:");
    post("bearsalmon • Information:");
    post("     buffer name: %s", x->b_name->s_name);
    post("     frame count: %d", x->b_frames);
    post("     validity %d", x->b_valid);
}

void bearsalmon_bufname(t_bearsalmon *x, t_symbol *name)
{
    x->b_name = name;
}

void bearsalmon_setup(void) {
    
    bearsalmon_class = class_new(gensym("bearsalmon"),
                                 (t_newmethod) bearsalmon_new,
                                 (t_method) bearsalmon_free,
                                 sizeof(t_bearsalmon),
                                 0, A_SYMBOL, A_SYMBOL, 0);
    
    class_addbang(bearsalmon_class, (t_method) bearsalmon_info);
    class_addmethod(bearsalmon_class, (t_method)bearsalmon_bufname, gensym("name"), A_SYMBOL, 0);
    class_addmethod(bearsalmon_class, (t_method) bearsalmon_cut, gensym("cut"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(bearsalmon_class, (t_method) bearsalmon_swap, gensym("swap"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(bearsalmon_class, (t_method) bearsalmon_shuffle, gensym("shuffle"), A_FLOAT, 0);

    
    post("bearsalmon • External was loaded");
    
}