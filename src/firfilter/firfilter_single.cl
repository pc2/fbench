/** @file firfilter.cl */

typedef float FLOATING_POINT;
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
    for (uint i=0; i<numSamples; i++)
    {
        FLOATING_POINT sum = 0.0;

        int k=i;
        uint j=0;
        
        for (; j<TAP_SIZE && k >= 0; j++,k--)
            sum += samples[k] * coefficients[j];

        results[i] = sum;
    }    
}
