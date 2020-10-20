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

 
Modifiers:       Jennifer Faj
Modifications:   - Change to single work-item kernel (get rid of barriers etc)
                 - FPGA specific optimizations
Date:            2020/05/10

*/


#define VEC_SIZE        16


__kernel void scan(__global const float* restrict in,
                        __global float* restrict out,
                        const uint n)
{   

    float s_seed = 0.0f;

    // process a block of 16x4=64 elements
    for(unsigned int ii = 0; ii < n; ii += (NUM_VECTORS*VEC_SIZE))
    {
        float lmem_reduce[NUM_VECTORS][VEC_SIZE];
        float lmem_bottom[NUM_VECTORS][VEC_SIZE];
        float blocksums[NUM_VECTORS];

        // read input data in chunks of 16 32-bit floating point elements
        for(unsigned int i = 0; i < NUM_VECTORS; i++)
        {
            #pragma unroll
            for(unsigned int j = 0; j < VEC_SIZE; j++)
            {
                float in_buff = in[ii + i*VEC_SIZE + j];
                lmem_reduce[i][j] = in_buff;
                lmem_bottom[i][j] = in_buff;
            }
        }

        // reduce 16 elements 4 times in parallel
        #pragma unroll
        for(int i = 0; i < NUM_VECTORS; i++)
        {

            float tmp_lmem_reduce[VEC_SIZE];
            #pragma unroll
            for(unsigned int j = 0; j < VEC_SIZE; j++)
            {
                tmp_lmem_reduce[j] = lmem_reduce[i][j];
            }

            // --- Reduction ---
            // purpose: sum up a block of 16 elements to later perform top-level scan
            // pragma: no pipelining due to data dependencies between each tree level
            // #pragma disable_loop_pipelining
            #pragma disable_loop_pipelining
            for(unsigned int s = VEC_SIZE/2; s > 0; s >>= 1)
            {   
                float r[VEC_SIZE];

                #pragma unroll
                for (unsigned int t = 0; t < VEC_SIZE; t++)
                {
                    if(t < s){
                        r[t] = tmp_lmem_reduce[t + s];
                    } else {
                        r[t] = 0.0f;
                    }
                }

                #pragma unroll
                for (unsigned int t = 0; t < VEC_SIZE; t++)
                {
                    tmp_lmem_reduce[t] += r[t];
                }
            }

            blocksums[i] = tmp_lmem_reduce[0];
        }

        // add last scan value of previous block to first new block
        blocksums[0] += s_seed;

        // --- Top-Scan ---
        // algorithm: Kogge-Stone-Scan (structure similar to reduction tree)
        // purpose: perform scan on blocksums to later add them to the bottom scans that are performed in parallel
        // pragma: no pipelining due to data dependencies between each tree level
        // #pragma disable_loop_pipelining
        #pragma disable_loop_pipelining
        for(int i = 1; i < NUM_VECTORS; i*=2)
        {
            float t[NUM_VECTORS];

            #pragma unroll
            for (int idx = 0; idx < NUM_VECTORS; idx++)
            {   
                if((idx - i) >= 0){
                    t[idx] = blocksums[idx - i];
                } else {
                    t[idx] = 0.0f;
                }
            }

            #pragma unroll
            for(int idx = 0; idx < NUM_VECTORS; idx++)
            {
                blocksums[idx] += t[idx];
            }
        }

        float __attribute__((register)) lmem_out[NUM_VECTORS][VEC_SIZE];

        // perform scan on 16 elements 4 times in parallel
        #pragma unroll
        for(int i = 0; i < NUM_VECTORS; i++)
        {

            float tmp_lmem_bottom[VEC_SIZE];
            #pragma unroll
            for(unsigned int j = 0; j < VEC_SIZE; j++)
            {
                tmp_lmem_bottom[j] = lmem_bottom[i][j];
            }

            // --- Bottom-Scan ---
            // algorithm: Kogge-Stone-Scan (structure similar to reduction tree)
            // purpose: perform scan on the elements within each of the 4 input subblocks
            // pragma: no pipelining due to data dependencies between each tree level
            // #pragma disable_loop_pipelining
            #pragma disable_loop_pipelining
            for(int j = 1; j < VEC_SIZE; j*=2)
            {
                float t[VEC_SIZE];

                #pragma unroll
                for (int idx = 0; idx < VEC_SIZE; idx++)
                {   
                    if((idx - j) >= 0){
                        t[idx] = tmp_lmem_bottom[idx - j];
                    } else {
                        t[idx] = 0.0f;
                    }
                }

                #pragma unroll
                for(int idx = 0; idx < VEC_SIZE; idx++)
                {
                    tmp_lmem_bottom[idx] += t[idx];
                }
            }

            #pragma unroll
            for(unsigned int j = 0; j < VEC_SIZE; j++)
            {
                lmem_out[i][j] = tmp_lmem_bottom[j];
            }

        }

        // --- Write ouput data ---
        // for the first subblock don't add blocksums
        #pragma unroll
        for(unsigned int j = 0; j < VEC_SIZE; j++)
        {
            out[ii + j] = lmem_out[0][j] + s_seed;
        }

        // for the following subblocks, add blocksum of preceding subblock
        for(unsigned int i = 1; i < NUM_VECTORS; i++)
        {
            #pragma unroll
            for(unsigned int j = 0; j < VEC_SIZE; j++)
            {
                out[ii + i*VEC_SIZE + j] = lmem_out[i][j] + blocksums[i-1];
            }
        }

        s_seed = blocksums[NUM_VECTORS-1];
    }      
}


