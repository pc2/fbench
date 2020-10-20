/** @file firfilter.cl */

#define MEM_BLOCK_SIZE_EFFECT BLOCK_SIZE % MEM_BLOCK_SIZE == 0 ? MEM_BLOCK_SIZE : BLOCK_SIZE

#define MANUAL_MAC_L1_LEN 8
#define MANUAL_MAC_L2_LEN 32
#define MANUAL_MAC_L2_REPS 4
#define MANUAL_MACS_L2_SIZE (MANUAL_MAC_L2_LEN*MANUAL_MAC_L2_REPS)

typedef float FLOATING_POINT;
typedef unsigned int uint;

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
mac_32x(uint blockIndex, uint sampleIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    return
        mac_8x(sampleIndex+0*MANUAL_MAC_L1_LEN,
            (sampleIndex-blockIndex)+0*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+1*MANUAL_MAC_L1_LEN,
            (sampleIndex-blockIndex)+1*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+2*MANUAL_MAC_L1_LEN,
            (sampleIndex-blockIndex)+2*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+3*MANUAL_MAC_L1_LEN,
            (sampleIndex-blockIndex)+3*MANUAL_MAC_L1_LEN, samples, coeffs);
}

FLOATING_POINT 
mac_manual(uint blockIndex, uint sampleIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{   
    return
        mac_32x(blockIndex, sampleIndex+0*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(blockIndex, sampleIndex+1*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(blockIndex, sampleIndex+2*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(blockIndex, sampleIndex+3*MANUAL_MAC_L2_LEN, samples, coeffs);
}

/****************************************************************************
* <b>Function:</b> convolve()
* <b>Purpose:</b> Within the FPGA, convolve the input samples with the filter
* coefficients and write back the results.
* @param samples the input samples to convolve.
* @param coefficients the filter coefficients.
* @param result output -the resulted convolved samples.
* @param numSamples the number of input samples.
* @param numCoefficients the number of filter coefficients.
* @returns Void
* @author Abdul Rehman Tareen
* @date May 14, 2020 
****************************************************************************/
__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel void
convolve(global FLOATING_POINT * restrict samples,
                global FLOATING_POINT * restrict coefficients,
                global FLOATING_POINT * restrict results,
                uint numSamples,
                uint numCoefficients)
{      
    FLOATING_POINT pr_coeffs[TAP_SIZE];
    #pragma unroll MEM_BLOCK_SIZE
    for (uint i=0,j=TAP_SIZE-1; i<TAP_SIZE; i++,j--)
        pr_coeffs[i] = coefficients[j];

    FLOATING_POINT pr_samples[TAP_SIZE+BLOCK_SIZE];
    #pragma unroll TAP_SIZE+BLOCK_SIZE
    for (uint j=0; j<TAP_SIZE+BLOCK_SIZE; j++)
            pr_samples[j] = 0;

    for (uint i=0; i<numSamples; i+=BLOCK_SIZE)
    {   
        #pragma unroll TAP_SIZE
        for (uint j=0; j<TAP_SIZE; j++)
            pr_samples[j] = pr_samples[j+BLOCK_SIZE];
        
        #pragma unroll MEM_BLOCK_SIZE_EFFECT
        for (uint j=TAP_SIZE; j<TAP_SIZE+BLOCK_SIZE; j++)
            pr_samples[j-1] = samples[(i+j)-TAP_SIZE];         

        FLOATING_POINT pr_results[BLOCK_SIZE];
        #pragma unroll BLOCK_SIZE
        for (uint j=0; j<BLOCK_SIZE; j++)
            pr_results[j] = 0.0;

        #pragma unroll ((BLOCK_SIZE*TAP_SIZE)/MANUAL_MACS_L2_SIZE)
        for (uint j=0; j<BLOCK_SIZE*TAP_SIZE; j+=MANUAL_MACS_L2_SIZE)
        {
        	uint blockIndex = j/TAP_SIZE;
		uint sampleIndex = blockIndex+(j%TAP_SIZE);
		pr_results[blockIndex] += 
                	mac_manual(blockIndex, sampleIndex, pr_samples, pr_coeffs);
        }
            
        #pragma unroll MEM_BLOCK_SIZE_EFFECT
        for (uint j=0; j<BLOCK_SIZE; j++)
            results[i+j] = pr_results[j];
    }
}
