/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Version$   $Date$
|*
|*  IN PACKAGE:         FFT
|*
|*  COPYRIGHT:          Copyright (c) 2006, Altium
|*
|*  DESCRIPTION:        FFT calculations
 */

#define FPU
#include "fft.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/*
 * THIS software is derived from KISSFFT
 * only parts that are necessary are includeed and optimised for this example
 */

/*
original copyright notice:

------------------------------------------


Copyright (c) 2003-2004 Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


------------------------------------------

*/



static complex_t *scratchbuf = NULL;
static size_t nscratchbuf = 0;
static complex_t * tmpbuf = NULL;
static size_t ntmpbuf = 0;


inline size_t checkbuf(complex_t *buf, size_t nbuf, size_t n)
{
    if (nbuf < n)
    {
        free(buf);
        buf = (complex_t*)malloc(sizeof(complex_t) * n);
        nbuf = n;
    }
    return nbuf;
}


/**********************************************************************
|*
|*  FUNCTION    : butterfly2
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to FFT data
|*                fstride = stride through FFT data
|*                m = number of butterfly-operations to perform
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Perform butterfly operation
 */

static void butterfly2(const fft_cfg st, complex_t *fout, const size_t fstride, size_t m)
{
    complex_t *fout2;
    complex_t *tw1 = st->twiddles;
    complex_t t;

    fout2 = fout + m;
    do
    {
        /* t = *fout2 * *tw1 */
        t.r = ((*fout2).r * tw1->r) - (fout2->i * tw1->i);
        t.i = (fout2->r * tw1->i) + (fout2->i * tw1->r);

        tw1 += fstride;

        /* *fout2 = *fout - t */
        fout2->r = fout->r - t.r;
        fout2->i = fout->i - t.i;

        /* *fout += t */
        fout->r = fout->r + t.r;
        fout->i = fout->i + t.i;

        ++fout2;
        ++fout;
    } while (--m);
}


/**********************************************************************
|*
|*  FUNCTION    : butterfly3
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to FFT data
|*                fstride = stride through FFT data
|*                m = number of butterfly-operations to perform
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Perform butterfly operation
 */

static void butterfly3(const fft_cfg st, complex_t *fout, const size_t fstride, size_t m)
{
    size_t k = m;
    const size_t m2 = 2 * m;
    complex_t *tw1, *tw2;
    complex_t scratch[5];
    complex_t epi3;
    epi3 = st->twiddles[fstride * m];

    tw1 = tw2 = st->twiddles;

    do
    {
        /* scratch[1] = fout[m] * *tw1 */
        scratch[1].r = (fout[m].r * tw1->r) - (fout[m].i * tw1->i);
        scratch[1].i = (fout[m].r * tw1->i) + (fout[m].i * tw1->r);

        /* scratch[1] = fout[m] * *tw1 */
        scratch[2].r = (fout[m2].r * tw2->r) - (fout[m2].i * tw2->i);
        scratch[2].i = (fout[m2].r * tw2->i) + (fout[m2].i * tw2->r);

        /* scratch[3] = scratch[0] + scratch[2] */
        scratch[3].r = scratch[1].r + scratch[2].r;
        scratch[3].i = scratch[1].i + scratch[2].i;

        /* scratch[4] = scratch[0] - scratch[2] */
        scratch[0].r = scratch[1].r - scratch[2].r;
        scratch[0].i = scratch[1].i - scratch[2].i;

        tw1 += fstride;
        tw2 += fstride * 2;

        fout[m].r = fout->r - (0.5 * scratch[3].r);
        fout[m].i = fout->i - (0.5 * scratch[3].i);

        scratch[0].r = scratch[0].r * epi3.i;
        scratch[0].i = scratch[0].i * epi3.i;

        fout->r = fout->r + scratch[3].r;
        fout->i = fout->i + scratch[3].i;

        fout[m2].r = fout[m].r + scratch[0].i;
        fout[m2].i = fout[m].i - scratch[0].r;

        fout[m].r = fout[m].r - scratch[0].i;
        fout[m].i = fout[m].i + scratch[0].r;

        ++fout;
    }
    while (--k);
}


/**********************************************************************
|*
|*  FUNCTION    : butterfly4
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to FFT data
|*                fstride = stride through FFT data
|*                m = number of butterfly-operations to perform
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Perform butterfly operation
 */

static void butterfly4(const fft_cfg st, complex_t *fout, const size_t fstride, const size_t m)
{
    complex_t *tw1, *tw2, *tw3;
    complex_t scratch[6];
    size_t k = m;
    const size_t m2 = 2 * m;
    const size_t m3 = 3 * m;

    tw3 = tw2 = tw1 = st->twiddles;

    do
    {
        /* scratch[0] = fout[m] * *tw1 */
        scratch[0].r = (fout[m].r * tw1->r) - (fout[m].i * tw1->i);
        scratch[0].i = (fout[m].r * tw1->i) + (fout[m].i * tw1->r);

        /* scratch[1] = fout[m2] * *tw2 */
        scratch[1].r = (fout[m2].r * tw2->r) - (fout[m2].i * tw2->i);
        scratch[1].i = (fout[m2].r * tw2->i) + (fout[m2].i * tw2->r);

        /* scratch[2] = fout[m3] * *tw3 */
        scratch[2].r = (fout[m3].r * tw3->r) - (fout[m3].i * tw3->i);
        scratch[2].i = (fout[m3].r * tw3->i) + (fout[m3].i * tw3->r);

        /* scratch[5] = *fout - scratch[1] */
        scratch[5].r = fout->r - scratch[1].r;
        scratch[5].i = fout->i - scratch[1].i;

        /* *fout += scratch[1] */
        fout->r = fout->r + scratch[1].r;
        fout->i = fout->i + scratch[1].i;

        /* scratch[3] = scratch[0] + scratch[2] */
        scratch[3].r = scratch[0].r + scratch[2].r;
        scratch[3].i = scratch[0].i + scratch[2].i;

        /* scratch[4] = scratch[0] - scratch[2] */
        scratch[4].r = scratch[0].r - scratch[2].r;
        scratch[4].i = scratch[0].i - scratch[2].i;

        /* fout[m2] = *fout - scratch[3] */
        fout[m2].r = fout->r - scratch[3].r;
        fout[m2].i = fout->i - scratch[3].i;

        tw1 += fstride;
        tw2 += fstride * 2;
        tw3 += fstride * 3;

        /* *fout += scratch[3] */
        fout->r = fout->r + scratch[3].r;
        fout->i = fout->i + scratch[3].i;

        fout[m].r = scratch[5].r + scratch[4].i;
        fout[m].i = scratch[5].i - scratch[4].r;
        fout[m3].r = scratch[5].r - scratch[4].i;
        fout[m3].i = scratch[5].i + scratch[4].r;

        ++fout;
    }
    while (--k);
}


/**********************************************************************
|*
|*  FUNCTION    : butterfly5
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to FFT data
|*                fstride = stride through FFT data
|*                m = number of butterfly-operations to perform
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Perform butterfly operation
 */

static void butterfly5(const fft_cfg st, complex_t *fout, const size_t fstride, int m)
{
    complex_t *fout0, *fout1, *fout2, *fout3, *fout4;
    int u;
    complex_t scratch[13];
    complex_t *twiddles = st->twiddles;
    complex_t *tw;
    complex_t ya, yb;
    ya = twiddles[fstride * m];
    yb = twiddles[fstride * 2 * m];

    fout0 = fout;
    fout1 = fout0 + m;
    fout2 = fout0 + 2 * m;
    fout3 = fout0 + 3 * m;
    fout4 = fout0 + 4 * m;

    tw = st->twiddles;
    for (u = 0; u < m; ++u)
    {
        scratch[0] = *fout0;

        /* scratch[1] = *fout1 * tw[u * fstride] */
        scratch[1].r = (fout1->r * tw[u * fstride].r) - (fout1->i * tw[u * fstride].i);
        scratch[1].i = (fout1->r * tw[u * fstride].i) + (fout1->i * tw[u * fstride].r);

        /* scratch[2] = *fout2 * tw[2 * u * fstride] */
        scratch[2].r = (fout2->r * tw[2 * u * fstride].r) - (fout2->i * tw[2 * u * fstride].i);
        scratch[2].i = (fout2->r * tw[2 * u * fstride].i) + (fout2->i * tw[2 * u * fstride].r);

        /* scratch[3] = *fout3 * tw[3 * u * fstride] */
        scratch[3].r = (fout3->r * tw[3 * u * fstride].r) - (fout3->i * tw[3 * u * fstride].i);
        scratch[3].i = (fout3->r * tw[3 * u * fstride].i) + (fout3->i * tw[3 * u * fstride].r);

        /* scratch[4] = *fout4 * tw[4 * u * fstride] */
        scratch[4].r = (fout4->r * tw[4 * u * fstride].r) - (fout4->i * tw[4 * u * fstride].i);
        scratch[4].i = (fout4->r * tw[4 * u * fstride].i) + (fout4->i * tw[4 * u * fstride].r);

        /* scratch[7] = scratch[1] + scratch[4] */
        scratch[7].r = scratch[1].r + scratch[4].r;
        scratch[7].i = scratch[1].i + scratch[4].i;

        /* scratch[10] = scratch[1] - scratch[4] */
        scratch[10].r = scratch[1].r - scratch[4].r;
        scratch[10].i = scratch[1].i - scratch[4].i;

        /* scratch[8] = scratch[2] + scratch[3] */
        scratch[8].r = scratch[2].r + scratch[3].r;
        scratch[8].i = scratch[2].i + scratch[3].i;

        /* scratch[9] = scratch[2] - scratch[3] */
        scratch[9].r = scratch[2].r - scratch[3].r;
        scratch[9].i = scratch[2].i - scratch[3].i;

        fout0->r = fout0->r + scratch[7].r + scratch[8].r;
        fout0->i = fout0->i + scratch[7].i + scratch[8].i;

        scratch[5].r = scratch[0].r + (scratch[7].r * ya.r) + (scratch[8].r * yb.r);
        scratch[5].i = scratch[0].i + (scratch[7].i * ya.r) + (scratch[8].i * yb.r);

        scratch[6].r =   (scratch[10].i * ya.i) + (scratch[9].i * yb.i);
        scratch[6].i = - (scratch[10].r * ya.i) + (scratch[9].r * yb.i);

        /* *fout1 = scratch[5] - scratch[6] */
        fout1->r = scratch[5].r - scratch[6].r;
        fout1->i = scratch[5].i - scratch[6].i;
        /* *fout4 = scratch[5] + scratch[6] */
        fout4->r = scratch[5].r + scratch[6].r;
        fout4->i = scratch[5].i + scratch[6].i;

        scratch[11].r = scratch[0].r + (scratch[7].r * yb.r) + (scratch[8].r * ya.r);
        scratch[11].i = scratch[0].i + (scratch[7].i * yb.r) + (scratch[8].i * ya.r);

        scratch[12].r = (scratch[9].i * ya.i) - (scratch[10].i * yb.i);
        scratch[12].i = (scratch[10].r * yb.i) - (scratch[9].r * ya.i);

        /* *fout2 = scratch[11] + scratch[12] */
        fout2->r = scratch[11].r + scratch[12].r;
        fout2->i = scratch[11].i + scratch[12].i;
        /* *fout3 = scratch[11] - scratch[12] */
        fout3->r = scratch[11].r - scratch[12].r;
        fout3->i = scratch[11].i - scratch[12].i;

        ++fout0;
        ++fout1;
        ++fout2;
        ++fout3;
        ++fout4;
    }
}



/**********************************************************************
|*
|*  FUNCTION    : butterfly_generic
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to result buffer
|*                fstride = stride through the FFT data
|*                m = number of butterfly operation to perform
|*                p = butterfly points
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : perform the butterfly for one stage of a mixed radix FFT
|*
 */

static void butterfly_generic(const fft_cfg st, complex_t *fout, const size_t fstride, int m, int p)
{
    int u, k, q1, q;
    complex_t * twiddles = st->twiddles;
    complex_t t;
    int Norig = st->nfft;

    nscratchbuf = checkbuf(scratchbuf, nscratchbuf, p);

    for (u = 0; u < m; ++u)
    {
        k = u;
        for (q1 = 0; q1 < p; ++q1)
        {
            scratchbuf[q1] = fout[k];
            k += m;
        }

        k = u;
        for (q1 = 0; q1 < p; ++q1)
        {
            int twidx = 0;
            fout[k] = scratchbuf[0];
            for (q = 1; q < p; ++q)
            {
                twidx += fstride * k;
                if (twidx >= Norig)
                    twidx -= Norig;
                /* t = scratchbuf[q] * twiddles[twidx] */
                t.r = (scratchbuf[q].r * twiddles[twidx].r) - (scratchbuf[q].i * twiddles[twidx].i);
                t.i = (scratchbuf[q].r * twiddles[twidx].i) + (scratchbuf[q].i * twiddles[twidx].r);
                /* fout[f] += t */
                fout[k].r = fout[k].r + t.r;
                fout[k].i = fout[k].i + t.i;
            }
            k += m;
        }
    }
}


/**********************************************************************
|*
|*  FUNCTION    : fft_work
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fout = pointer to result buffer
|*                f = pointer to FFT input data
|*                fstride = stride through the FFT data
|*                factors = pointer to FFT input data
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Split FFT, call recursive till butterfly is possible, then perform butterfly
|*
 */

static void fft_work(const fft_cfg st, complex_t *fout, const complex_t *f, const size_t fstride, int *factors)
{
    complex_t *fout_beg = fout;
    const int p = *factors++; /* the radix  */
    const int m = *factors++; /* stage's fft length/p */
    const complex_t *fout_end = fout + p * m;

    if (m == 1)
    {
        do
        {
            *fout = *f;
            f += fstride;
        }
        while (++fout != fout_end);
    }
    else
    {
        do
        {
            fft_work(st,  fout, f, fstride * p, factors);
            f += fstride;
        }
        while ((fout += m) != fout_end);
    }

    fout = fout_beg;

    switch (p)
    {
        case 2 :
            butterfly2(st, fout, fstride, m);
            break;
        case 3 :
            butterfly3(st, fout, fstride, m);
            break;
        case 4 :
            butterfly4(st, fout, fstride, m);
            break;
        case 5 :
            butterfly5(st, fout, fstride, m);
            break;
        default :
            butterfly_generic(st, fout, fstride, m, p);
            break;
    }
}


/**********************************************************************
|*
|*  FUNCTION    : fft_splitfactor
|*
|*  PARAMETERS  : n = number to be split
|*                factorbuf = pointer to buffer with factors
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Split n in factors
|*
 */

static void fft_splitfactor(int n, int *factorbuf)
{
/*
 * facbuf is populated by p1,m1,p2,m2, ...
 * where
 * p[i] * m[i] = m[i-1]
 * m0 = n
 */
    while ((n % 4) == 0)
    {
        n /= 4;
        *factorbuf++ = 4;
        *factorbuf++ = n;
    }
    while ((n % 2) == 0)
    {
        n /= 2;
        *factorbuf++ = 2;
        *factorbuf++ = n;
    }
    while ((n % 3) == 0)
    {
        n /= 3;
        *factorbuf++ = 3;
        *factorbuf++ = n;
    }
    while ((n % 5) == 0)
    {
        n /= 5;
        *factorbuf++ = 5;
        *factorbuf++ = n;
    }
    if (n != 1)
    {
        *factorbuf++ = n;
        *factorbuf++ = 1;
    }
}


/**********************************************************************
|*
|*  FUNCTION    : fft_alloc
|*
|*  PARAMETERS  : nfft = number of FFT points
|*                inverse_fft = perform inverse fft, normally 0
|*                mem = pointer to memory, used by FFT
|*                lenmem = size of memory
|*
|*  RETURNS     : FFT configuration data structure
|*
|*  DESCRIPTION : Initialize FFT, alloc memory if needed
|*                User-callable function to allocate all necessary storage space for the fft.
|*                The return value is a contiguous block of memory, allocated with malloc.  As such,
|*                It can be freed with free(), rather than a kiss_fft-specific function.
|*
 */

fft_cfg fft_alloc(int nfft, int inverse_fft, void * mem, size_t * lenmem)
{
    fft_cfg st = NULL;
    size_t memneeded = sizeof(struct fft_state) + sizeof(complex_t) *(nfft - 1); /* twiddle factors*/

    if (lenmem == NULL)
    {
        st = (fft_cfg)malloc(memneeded);
    }
    else
    {
        if (* lenmem >= memneeded)
            st = (fft_cfg)mem;
        * lenmem = memneeded;
    }
    if (st)
    {
        int i;
        st->nfft = nfft;
        st->inverse = inverse_fft;

        for (i = 0; i < nfft; ++i)
        {
            const double pi = 3.14159265358979323846264338327;
            double phase = (- 2 * pi / nfft) * i;
            if (st->inverse)
            {
                phase *= - 1;
            }
            (st->twiddles + i)->r = cos(phase);
            (st->twiddles + i)->i = sin(phase);

        }

        fft_splitfactor(nfft, st->factors);
    }
    return st;
}


/**********************************************************************
|*
|*  FUNCTION    : fft_stride
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fin = pointer to input data in complex notation
|*                fout = pointer to FFT-results in complex notation
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Calculate fft, called by fft()
|*
 */

static void fft_stride(fft_cfg st, const complex_t * fin, complex_t * fout)
{
    if (fin == fout)
    {
        ntmpbuf = checkbuf(tmpbuf, ntmpbuf, st->nfft);
        fft_work(st, tmpbuf, fin, 1, st->factors);
        memcpy(fout, tmpbuf, sizeof(complex_t) * st->nfft);
    }
    else
    {
        fft_work(st, fout, fin, 1, st->factors);
    }
}


/**********************************************************************
|*
|*  FUNCTION    : fft
|*
|*  PARAMETERS  : fft_cfg = FFT configuration data
|*                fin = pointer to input data in complex notation
|*                fout = pointer to FFT-results in complex notation
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Calculate fft
|*
 */

void fft(fft_cfg cfg, const complex_t * fin, complex_t * fout)
{
    fft_stride(cfg, fin, fout);
}

