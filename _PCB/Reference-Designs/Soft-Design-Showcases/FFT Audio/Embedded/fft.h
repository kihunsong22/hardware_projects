/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Version$   $Date$
|*
|*  IN PACKAGE:         FFT
|*
|*  COPYRIGHT:          Copyright (c) 2006, Altium
|*
|*  DESCRIPTION:        Interface to FFT
 */

#ifndef _FFT_H
#define _FFT_H

#include <limits.h>

#define MAXFACTORS 32
/* e.g. an fft of length 128 has 4 factors
 as far as kissfft is concerned
 4*4*4*2
 */

#include <malloc.h>


typedef struct
{
    float r;
    float i;
}complex_t;

typedef struct fft_state *fft_cfg;

struct fft_state
{
    int nfft;
    int inverse;
    int factors[2*MAXFACTORS];
    complex_t twiddles[];
};

fft_cfg fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem);
void fft(fft_cfg cfg,const complex_t *fin,complex_t *fout);

#define fftr_free free

#endif // _FFT_H
