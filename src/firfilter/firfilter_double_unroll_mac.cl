/** @file firfilter.cl */

#define SERIAL_MAC_SIZE 16

typedef double FLOATING_POINT;
typedef unsigned int uint;

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
        
        #pragma unroll MEM_BLOCK_SIZE
        for (uint j=TAP_SIZE; j<TAP_SIZE+BLOCK_SIZE; j++)
            pr_samples[j-1] = samples[(i+j)-TAP_SIZE];         

        FLOATING_POINT pr_results[BLOCK_SIZE];
        #pragma unroll BLOCK_SIZE
        for (uint j=0; j<BLOCK_SIZE; j++)
            pr_results[j] = 0.0;

        #pragma unroll ((BLOCK_SIZE*TAP_SIZE)/SERIAL_MAC_SIZE)
        for (uint j=0; j<BLOCK_SIZE*TAP_SIZE; j+=SERIAL_MAC_SIZE)
        {
            for (uint k=j; k<j+SERIAL_MAC_SIZE; k++)
            {
                uint blockIndex = k/TAP_SIZE;
                uint sampleIndex = blockIndex+(k%TAP_SIZE);

                pr_results[blockIndex] += 
                    (pr_samples[sampleIndex] * pr_coeffs[sampleIndex-blockIndex]);
            }
        }
            
        #pragma unroll MEM_BLOCK_SIZE
        for (uint j=0; j<BLOCK_SIZE; j++)
            results[i+j] = pr_results[j];
    }
}
