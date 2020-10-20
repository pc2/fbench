/** @file firfilter.cl */

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_intel_channels : enable

#define X_DIM_COEFF_SIZE BLOCK_SIZE
#define Y_DIM_COEFF_SIZE TAP_SIZE

#define X_DIM_SMP_SIZE BLOCK_SIZE 
#define Y_DIM_SMP_SIZE TAP_SIZE    

#define MANUAL_MAC_L1_LEN 8
#define MANUAL_MAC_L1_REPS 2

#define MANUAL_MAC_L2_LEN (MANUAL_MAC_L1_LEN * MANUAL_MAC_L1_REPS)
#define MANUAL_MAC_L2_REPS 2

#define MANUAL_MAC_L3_LEN (MANUAL_MAC_L2_LEN * MANUAL_MAC_L2_REPS)
#define MANUAL_MAC_L3_REPS 2

#define MANUAL_MAC_L4_LEN (MANUAL_MAC_L3_LEN * MANUAL_MAC_L3_REPS)
#define MANUAL_MAC_L4_REPS 4

#define MANUAL_MAC_TOTAL_LEN (MANUAL_MAC_L4_LEN * MANUAL_MAC_L4_REPS)

typedef float FLOATING_POINT;
typedef unsigned int uint;

typedef struct BLOCK_SAMPLES {
   FLOATING_POINT vector[BLOCK_SIZE+TAP_SIZE];
} BLOCK_SAMPLES;

typedef struct BLOCKS {
   BLOCK_SAMPLES blocks[BLOCK_MEM_LCM/BLOCK_SIZE];
} BLOCKS;

typedef struct BLOCK_RESULT {
   FLOATING_POINT vector[BLOCK_SIZE];
} BLOCK_RESULT;

__attribute__((depth(BLOCK_MEM_LCM/BLOCK_SIZE)))
channel BLOCKS samp_chan;

__attribute__((depth(BLOCK_MEM_LCM/BLOCK_SIZE))) 
channel BLOCK_RESULT res_chan;

FLOATING_POINT 
mac_8x(uint sampleIndex, uint coeffIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    return  samples[sampleIndex+0]*coeffs[coeffIndex+0] +
            samples[sampleIndex+1]*coeffs[coeffIndex+1] +
            samples[sampleIndex+2]*coeffs[coeffIndex+2] +
            samples[sampleIndex+3]*coeffs[coeffIndex+3] +
            samples[sampleIndex+4]*coeffs[coeffIndex+4] + 
            samples[sampleIndex+5]*coeffs[coeffIndex+5] +
            samples[sampleIndex+6]*coeffs[coeffIndex+6] + 
            samples[sampleIndex+7]*coeffs[coeffIndex+7] ;
}

FLOATING_POINT 
mac_16x(uint sampleIndex, uint coeffIndex,
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    uint l1_len = MANUAL_MAC_L1_LEN;
    return
        mac_8x(sampleIndex+0*l1_len, 
            coeffIndex+0*l1_len, samples, coeffs) +
        mac_8x(sampleIndex+1*l1_len, 
            coeffIndex+1*l1_len, samples, coeffs) ;
}

FLOATING_POINT 
mac_32x(uint sampleIndex, uint coeffIndex,
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    uint l2_len = MANUAL_MAC_L2_LEN;
    return
        mac_16x(sampleIndex+0*l2_len, 
            coeffIndex+0*l2_len, samples, coeffs) +
        mac_16x(sampleIndex+1*l2_len, 
            coeffIndex+1*l2_len, samples, coeffs) ;
}

FLOATING_POINT 
mac_64x(uint sampleIndex, uint coeffIndex,
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    uint l3_len = MANUAL_MAC_L3_LEN;
    return
        mac_32x(sampleIndex+0*l3_len,
                coeffIndex+0*l3_len, samples, coeffs) +
        mac_32x(sampleIndex+1*l3_len,
                coeffIndex+1*l3_len, samples, coeffs) ;
}

FLOATING_POINT 
vec_mac_manual_256x(FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{   
    uint l4_len = MANUAL_MAC_L4_LEN;
    return
        mac_64x(0*l4_len, 0*l4_len, samples, coeffs) +
        mac_64x(1*l4_len, 1*l4_len, samples, coeffs) +
        mac_64x(2*l4_len, 2*l4_len, samples, coeffs) +
        mac_64x(3*l4_len, 3*l4_len, samples, coeffs);
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel void
convolve_read(global FLOATING_POINT * restrict samples,
                unsigned int numSamples)
{
    FLOATING_POINT //__attribute__((register)) 
        tmp_samples[TAP_SIZE+BLOCK_MEM_LCM];
    #pragma unroll TAP_SIZE+BLOCK_MEM_LCM
    for (uint j=0; j<TAP_SIZE+BLOCK_MEM_LCM; j++)
        tmp_samples[j] = 0;

    for (int i=0; i<numSamples; i+=BLOCK_MEM_LCM)
    {
        #pragma unroll BLOCK_MEM_LCM
        for (int j=TAP_SIZE; j<TAP_SIZE+BLOCK_MEM_LCM; j++)
            tmp_samples[j-1] = samples[(j-TAP_SIZE)+i];        

        BLOCKS block;

        #pragma unroll (BLOCK_MEM_LCM/BLOCK_SIZE)
        for (int j=0; j<BLOCK_MEM_LCM/BLOCK_SIZE; j++)
            #pragma unroll TAP_SIZE+BLOCK_SIZE
            for (uint k=0; k<TAP_SIZE+BLOCK_SIZE; k++)
                block.blocks[j].vector[k] = tmp_samples[j*BLOCK_SIZE+k];

        write_channel_intel(samp_chan, block);

        #pragma unroll TAP_SIZE // Shift in BLOCK_MEM_LCM
        for (uint j=0; j<TAP_SIZE; j++)
            tmp_samples[j] = tmp_samples[j+BLOCK_MEM_LCM];
    }

}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel void
convolve_perform(global FLOATING_POINT * restrict coefficients,
                unsigned int numSamples)
{   
    FLOATING_POINT tmp_coeffs[TAP_SIZE];
    #pragma unroll MEM_BLOCK_SIZE
    for (uint j=0; j<Y_DIM_COEFF_SIZE; j++)
        tmp_coeffs[j] = coefficients[j];

    FLOATING_POINT //__attribute__((register)) 
        pr_coeffs[X_DIM_COEFF_SIZE][Y_DIM_COEFF_SIZE];

    #pragma unroll X_DIM_COEFF_SIZE
    for (uint j=0; j<X_DIM_COEFF_SIZE; j++)
        #pragma unroll Y_DIM_COEFF_SIZE
        for (uint l=0,k=TAP_SIZE-1; l<Y_DIM_COEFF_SIZE; l++,k--)
            pr_coeffs[j][l] = tmp_coeffs[k];

    for (int i=0; i<numSamples; i+=BLOCK_MEM_LCM)
    {   
        BLOCKS block = read_channel_intel(samp_chan);

        for (uint j=0; j<BLOCK_MEM_LCM; j+=BLOCK_SIZE)
        {
            FLOATING_POINT //__attribute__((register)) 
                pr_samples[X_DIM_SMP_SIZE][Y_DIM_SMP_SIZE];   

            #pragma unroll X_DIM_SMP_SIZE
            for (uint k=0; k<X_DIM_SMP_SIZE; k++)
                #pragma unroll Y_DIM_SMP_SIZE
                for (uint l=0; l<Y_DIM_SMP_SIZE; l++)
                    pr_samples[k][l] = block.blocks[j/BLOCK_SIZE].vector[k+l];
            
            BLOCK_RESULT block_result;
            #pragma unroll BLOCK_SIZE
            for (uint k=0; k<BLOCK_SIZE; k++)
                block_result.vector[k] = vec_mac_manual_256x(pr_samples[k],
                    pr_coeffs[k]);   

            write_channel_intel(res_chan, block_result);  
        }
    }     
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel void
convolve_write(global FLOATING_POINT * restrict results,
                unsigned int numSamples)
{
    for (int i=0; i<numSamples; i+=BLOCK_MEM_LCM)
    {
        BLOCK_RESULT //__attribute__((register))  
            block_results[BLOCK_MEM_LCM/BLOCK_SIZE];

        for (int j=0; j<BLOCK_MEM_LCM; j+=BLOCK_SIZE)
            block_results[j/BLOCK_SIZE] = 
                read_channel_intel(res_chan);

        #pragma unroll BLOCK_MEM_LCM
        for (int k=0; k<BLOCK_MEM_LCM; k++)
            results[i+k] = 
                block_results[k/BLOCK_SIZE].vector[k%BLOCK_SIZE];       
    }    
}
