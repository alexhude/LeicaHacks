#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "md5.h"

typedef union uwb {
    unsigned w;
    unsigned char b[4];
} WBunion;

typedef unsigned Digest[4];

unsigned md5_f0( unsigned abcd[] ){
    return ( abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);}

unsigned md5_f1( unsigned abcd[] ){
    return ( abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);}

unsigned md5_f2( unsigned abcd[] ){
    return  abcd[1] ^ abcd[2] ^ abcd[3];}

unsigned md5_f3( unsigned abcd[] ){
    return abcd[2] ^ (abcd[1] |~ abcd[3]);}

typedef unsigned (*DgstFctn)(unsigned a[]);

unsigned *md5_calcKs( unsigned *k)
{
    double s, pwr;
    int i;
	
    pwr = pow( 2, 32);
    for (i=0; i<64; i++) {
        s = fabs(sin(1+i));
        k[i] = (unsigned)( s * pwr );
    }
    return k;
}

// ROtate v Left by amt bits
unsigned md5_rol( unsigned v, short amt )
{
    unsigned  msk1 = (1<<amt) -1;
    return ((v>>(32-amt)) & msk1) | ((v<<amt) & ~msk1);
}

unsigned *md5_calc( uint8_t *msg, int mlen)
{
    static Digest h0 = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
//  static Digest h0 = { 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210 };
    static DgstFctn ff[] = { &md5_f0, &md5_f1, &md5_f2, &md5_f3 };
    static short M[] = { 1, 5, 3, 7 };
    static short O[] = { 0, 1, 5, 0 };
    static short rot0[] = { 7,12,17,22};
    static short rot1[] = { 5, 9,14,20};
    static short rot2[] = { 4,11,16,23};
    static short rot3[] = { 6,10,15,21};
    static short *rots[] = {rot0, rot1, rot2, rot3 };
    static unsigned kspace[64];
    static unsigned *k;
	
    static Digest h;
    Digest abcd;
    DgstFctn fctn;
    short m, o, g;
    unsigned f;
    short *rotn;
    union {
        unsigned w[16];
        char     b[64];
    }mm;
    int os = 0;
    int grp, grps, q, p;
    unsigned char *msg2;
	
    if (k==NULL) k= md5_calcKs(kspace);
	
    for (q=0; q<4; q++) h[q] = h0[q];   // initialize
	
    {
        grps  = 1 + (mlen+8)/64;
        msg2 = (unsigned char *)malloc( 64*grps);
        memcpy( msg2, msg, mlen);
        msg2[mlen] = (unsigned char)0x80;
        q = mlen + 1;
        while (q < 64*grps){ msg2[q] = 0; q++ ; }
        {
//          unsigned char t;
            WBunion u;
            u.w = 8*mlen;
//          t = u.b[0]; u.b[0] = u.b[3]; u.b[3] = t;
//          t = u.b[1]; u.b[1] = u.b[2]; u.b[2] = t;
            q -= 8;
            memcpy(msg2+q, &u.w, 4 );
        }
    }
	
    for (grp=0; grp<grps; grp++)
    {
        memcpy( mm.b, msg2+os, 64);
        for(q=0;q<4;q++) abcd[q] = h[q];
        for (p = 0; p<4; p++) {
            fctn = ff[p];
            rotn = rots[p];
            m = M[p]; o= O[p];
            for (q=0; q<16; q++) {
                g = (m*q + o) % 16;
                f = abcd[1] + md5_rol( abcd[0]+ fctn(abcd) + k[q+16*p] + mm.w[g], rotn[q%4]);
				
                abcd[0] = abcd[3];
                abcd[3] = abcd[2];
                abcd[2] = abcd[1];
                abcd[1] = f;
            }
        }
        for (p=0; p<4; p++)
            h[p] += abcd[p];
        os += 64;
    }
    return h;
}

void get_md5(uint8_t* in_buf, uint32_t in_size, uint8_t* md5)
{
	memcpy(md5, md5_calc(in_buf, in_size), 16);
}

bool check_md5(uint8_t* in_buf, uint32_t in_size, uint8_t* md5)
{
	return (memcmp(md5_calc(in_buf, in_size), md5, 16) == 0);
}
