/*

Copyright (c) 2011, UT-Battelle, LLC
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
  the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/** @file md5.cl
*/
// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

#define F(x,y,z) ((x & y) | ((~x) & z))
#define G(x,y,z) ((x & z) | ((~z) & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | (~z)))

/// This version of the round shifts the interpretation of a,b,c,d by one
/// and must be called with v/x/y/z in a matching shuffle pattern.
/// Every four Rounds, a,b,c,d are back to their original interpretation,
/// thogh, so it all works out in the end (we have 64 rounds per block).
#define ROUND_INPLACE_VIA_SHIFT(w, r, k, v, x, y, z, func)       \
{                                                                \
    v += func(x,y,z) + w + k;                                    \
    v = x + LEFTROTATE(v, r);                                    \
}

/// This version ignores the mapping of a/b/c/d to v/x/y/z and simply
/// uses a temporary variable to keep the interpretation of a/b/c/d
/// consistent.  Whether this one or the previous one performs better
/// probably depends on the compiler....
#define ROUND_USING_TEMP_VARS(w, r, k, v, x, y, z, func)         \
{                                                                \
    a = a + func(b,c,d) + k + w;                                 \
    unsigned int temp = d;                                       \
    d = c;                                                       \
    c = b;                                                       \
    b = b + LEFTROTATE(a, r);                                    \
    a = temp;                                                    \
}

// Here, we pick which style of ROUND we use.
#define ROUND ROUND_USING_TEMP_VARS
//#define ROUND ROUND_INPLACE_VIA_SHIFT

/// NOTE: this really only allows a length up to 7 bytes, not 8, because
/// we need to start the padding in the first byte following the message,
/// and we only have two words to work with here....
/// It also assumes words[] has all zero bits except the chars of interest.
inline bool md5_2words_compare(unsigned int *words, unsigned int len,
                                int *digest)
{
    // For any block but the first one, these should be passed in, not
    // initialized, but we are assuming we only operate on a single block.
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xefcdab89;
    unsigned int h2 = 0x98badcfe;
    unsigned int h3 = 0x10325476;

    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;

    unsigned int WL = len * 8;
    unsigned int W0 = words[0];
    unsigned int W1 = words[1];

    switch (len)
    {
      case 0: W0 |= 0x00000080; break;
      case 1: W0 |= 0x00008000; break;
      case 2: W0 |= 0x00800000; break;
      case 3: W0 |= 0x80000000; break;
      case 4: W1 |= 0x00000080; break;
      case 5: W1 |= 0x00008000; break;
      case 6: W1 |= 0x00800000; break;
      case 7: W1 |= 0x80000000; break;
      default: break;
    }

    // args: word data, per-round shift amt, constant, 4 vars, function macro
    ROUND(W0,   7, 0xd76aa478, a, b, c, d, F);
    ROUND(W1,  12, 0xe8c7b756, d, a, b, c, F);
    ROUND(0,   17, 0x242070db, c, d, a, b, F);
    ROUND(0,   22, 0xc1bdceee, b, c, d, a, F);
    ROUND(0,    7, 0xf57c0faf, a, b, c, d, F);
    ROUND(0,   12, 0x4787c62a, d, a, b, c, F);
    ROUND(0,   17, 0xa8304613, c, d, a, b, F);
    ROUND(0,   22, 0xfd469501, b, c, d, a, F);
    ROUND(0,    7, 0x698098d8, a, b, c, d, F);
    ROUND(0,   12, 0x8b44f7af, d, a, b, c, F);
    ROUND(0,   17, 0xffff5bb1, c, d, a, b, F);
    ROUND(0,   22, 0x895cd7be, b, c, d, a, F);
    ROUND(0,    7, 0x6b901122, a, b, c, d, F);
    ROUND(0,   12, 0xfd987193, d, a, b, c, F);
    ROUND(WL,  17, 0xa679438e, c, d, a, b, F);
    ROUND(0,   22, 0x49b40821, b, c, d, a, F);

    ROUND(W1,   5, 0xf61e2562, a, b, c, d, G);
    ROUND(0,    9, 0xc040b340, d, a, b, c, G);
    ROUND(0,   14, 0x265e5a51, c, d, a, b, G);
    ROUND(W0,  20, 0xe9b6c7aa, b, c, d, a, G);
    ROUND(0,    5, 0xd62f105d, a, b, c, d, G);
    ROUND(0,    9, 0x02441453, d, a, b, c, G);
    ROUND(0,   14, 0xd8a1e681, c, d, a, b, G);
    ROUND(0,   20, 0xe7d3fbc8, b, c, d, a, G);
    ROUND(0,    5, 0x21e1cde6, a, b, c, d, G);
    ROUND(WL,   9, 0xc33707d6, d, a, b, c, G);
    ROUND(0,   14, 0xf4d50d87, c, d, a, b, G);
    ROUND(0,   20, 0x455a14ed, b, c, d, a, G);
    ROUND(0,    5, 0xa9e3e905, a, b, c, d, G);
    ROUND(0,    9, 0xfcefa3f8, d, a, b, c, G);
    ROUND(0,   14, 0x676f02d9, c, d, a, b, G);
    ROUND(0,   20, 0x8d2a4c8a, b, c, d, a, G);

    ROUND(0,    4, 0xfffa3942, a, b, c, d, H);
    ROUND(0,   11, 0x8771f681, d, a, b, c, H);
    ROUND(0,   16, 0x6d9d6122, c, d, a, b, H);
    ROUND(WL,  23, 0xfde5380c, b, c, d, a, H);
    ROUND(W1,   4, 0xa4beea44, a, b, c, d, H);
    ROUND(0,   11, 0x4bdecfa9, d, a, b, c, H);
    ROUND(0,   16, 0xf6bb4b60, c, d, a, b, H);
    ROUND(0,   23, 0xbebfbc70, b, c, d, a, H);
    ROUND(0,    4, 0x289b7ec6, a, b, c, d, H);
    ROUND(W0,  11, 0xeaa127fa, d, a, b, c, H);
    ROUND(0,   16, 0xd4ef3085, c, d, a, b, H);
    ROUND(0,   23, 0x04881d05, b, c, d, a, H);
    ROUND(0,    4, 0xd9d4d039, a, b, c, d, H);
    ROUND(0,   11, 0xe6db99e5, d, a, b, c, H);
    ROUND(0,   16, 0x1fa27cf8, c, d, a, b, H);
    ROUND(0,   23, 0xc4ac5665, b, c, d, a, H);

    ROUND(W0,   6, 0xf4292244, a, b, c, d, I);
    ROUND(0,   10, 0x432aff97, d, a, b, c, I);
    ROUND(WL,  15, 0xab9423a7, c, d, a, b, I);
    ROUND(0,   21, 0xfc93a039, b, c, d, a, I);
    ROUND(0,    6, 0x655b59c3, a, b, c, d, I);
    ROUND(0,   10, 0x8f0ccc92, d, a, b, c, I);
    ROUND(0,   15, 0xffeff47d, c, d, a, b, I);
    ROUND(W1,  21, 0x85845dd1, b, c, d, a, I);
    ROUND(0,    6, 0x6fa87e4f, a, b, c, d, I);
    ROUND(0,   10, 0xfe2ce6e0, d, a, b, c, I);
    ROUND(0,   15, 0xa3014314, c, d, a, b, I);
    ROUND(0,   21, 0x4e0811a1, b, c, d, a, I);
    ROUND(0,    6, 0xf7537e82, a, b, c, d, I);
    ROUND(0,   10, 0xbd3af235, d, a, b, c, I);
    ROUND(0,   15, 0x2ad7d2bb, c, d, a, b, I);
    ROUND(0,   21, 0xeb86d391, b, c, d, a, I);

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;

    return digest[0] == h0 && digest[1] == h1 
        && digest[2] == h2 && digest[3] == h3;
}

/****************************************************************************
* <b>Function:</b> IndexToKey()
*
* <b>Purpose:</b> For a given index in the keyspace, find the actual key string
* which is at that index.
*
* @param index index in key space
* @param byteLength number of bytes in a key
* @param valsPerByte number of values each byte can take on
* @param vals output key string
*
* @returns Void
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* <b>Modifications:</b>
****************************************************************************/
inline void IndexToKey(unsigned int index, int byteLength, int valsPerByte,
                       unsigned char vals[8])
{
    vals[0] = index % valsPerByte;
    index /= valsPerByte;

    vals[1] = index % valsPerByte;
    index /= valsPerByte;

    vals[2] = index % valsPerByte;
    index /= valsPerByte;

    vals[3] = index % valsPerByte;
    index /= valsPerByte;

    vals[4] = index % valsPerByte;
    index /= valsPerByte;

    vals[5] = index % valsPerByte;
    index /= valsPerByte;

    vals[6] = index % valsPerByte;
    index /= valsPerByte;

    vals[7] = index % valsPerByte;
    index /= valsPerByte;
}

/****************************************************************************
* <b>Function:</b> FindKeyWithDigest_Kernel()
*
* <b>Purpose:</b> Within the FPGA, search the key space
* to find a key with the given digest.
*
* @param searchDigest the digest to search for
* @param keyspace the size of the key space to search
* @param byteLength number of bytes in a key
* @param valsPerByte number of values each byte can take on
* @param foundIndex output - the index of the found key (if found)
* @param foundKey output - the string of the found key (if found)
* @param foundDigest output - the digest of the found key (if found)
*
* @returns Void
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* <b>Modifiers:</b> Abdul Rehman  
*      
* <b>Modifications:</b>
*
*  + Changed GPU code to Single Work Item FPGA Code.
*  + Optimized the code for the FPGA.
*   
****************************************************************************/
__attribute__((uses_global_work_offset(0)))
__kernel void
FindKeyWithDigest_Kernel(unsigned int searchDigest0,
                         unsigned int searchDigest1,
                         unsigned int searchDigest2,
                         unsigned int searchDigest3,
                         int keyspace,
                         int byteLength, 
                         int valsPerByte,
                         global int* restrict foundIndex,
                         global unsigned char* restrict foundKey,
                         global unsigned int* restrict foundDigest)
{
    int locFoundIndex = -1;

    int locSearchDigest[4] = 
        { searchDigest0, searchDigest1, searchDigest2, searchDigest3};

    for (int h=0; h<keyspace; h+=TERMINAL_LOOP_SIZE)
    {
        #pragma unroll TERMINAL_LOOP_SIZE
        for (unsigned int i=0; i<TERMINAL_LOOP_SIZE; i++)
        {
            unsigned char key[8] = {0,0,0,0, 0,0,0,0};
            IndexToKey(h+i, byteLength, valsPerByte, key);

            if (md5_2words_compare((unsigned int*)key, byteLength,
                                    locSearchDigest))
            {
                locFoundIndex = h+i; 
            }
        }
    }

    unsigned char key[8] = {0,0,0,0, 0,0,0,0};
    IndexToKey(locFoundIndex, byteLength, valsPerByte, key);

    *foundIndex = locFoundIndex;

    foundKey[0] = key[0];
    foundKey[1] = key[1];
    foundKey[2] = key[2];
    foundKey[3] = key[3];
    foundKey[4] = key[4];
    foundKey[5] = key[5];
    foundKey[6] = key[6];
    foundKey[7] = key[7];

    foundDigest[0] = locSearchDigest[0];
    foundDigest[1] = locSearchDigest[1];
    foundDigest[2] = locSearchDigest[2];
    foundDigest[3] = locSearchDigest[3];

}
