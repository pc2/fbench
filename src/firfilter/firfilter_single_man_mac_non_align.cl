/** @file firfilter.cl */

#define MANUAL_MAC_L1_LEN 8
#define MANUAL_MAC_L2_LEN 32
#define MANUAL_MAC_L3_LEN 128
#define MANUAL_MAC_L4_LEN 256

#define X_DIM_COEFF_SIZE BLOCK_SIZE
#define Y_DIM_COEFF_SIZE TAP_SIZE

#define X_DIM_SMP_SIZE BLOCK_SIZE 
#define Y_DIM_SMP_SIZE TAP_SIZE       

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
mac_32x(uint sampleIndex, uint coeffIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    return
        mac_8x(sampleIndex+0*MANUAL_MAC_L1_LEN,
            coeffIndex + 0*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+1*MANUAL_MAC_L1_LEN,
           coeffIndex + 1*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+2*MANUAL_MAC_L1_LEN,
            coeffIndex + 2*MANUAL_MAC_L1_LEN, samples, coeffs)+
        mac_8x(sampleIndex+3*MANUAL_MAC_L1_LEN,
            coeffIndex + 3*MANUAL_MAC_L1_LEN, samples, coeffs);
}

FLOATING_POINT 
mac_128x(uint sampleIndex, uint coeffIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{
    return
        mac_32x(sampleIndex+0*MANUAL_MAC_L2_LEN,
            coeffIndex + 0*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(sampleIndex+1*MANUAL_MAC_L2_LEN,
            coeffIndex + 1*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(sampleIndex+2*MANUAL_MAC_L2_LEN,
            coeffIndex + 2*MANUAL_MAC_L2_LEN, samples, coeffs)+
        mac_32x(sampleIndex+3*MANUAL_MAC_L2_LEN,
            coeffIndex + 3*MANUAL_MAC_L2_LEN, samples, coeffs);
}

FLOATING_POINT 
vec_mac_manual_256x(uint sampleIndex, 
    FLOATING_POINT samples[], FLOATING_POINT coeffs[])
{   
    return
        mac_128x(sampleIndex+0*MANUAL_MAC_L3_LEN, 0*MANUAL_MAC_L3_LEN, samples, coeffs)+
        mac_128x(sampleIndex+1*MANUAL_MAC_L3_LEN, 1*MANUAL_MAC_L3_LEN, samples, coeffs);
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
    FLOATING_POINT //__attribute__((register))  
        pr_coeffs[TAP_SIZE];
    #pragma unroll MEM_BLOCK_SIZE
    for (uint j=0, k=TAP_SIZE-1; 
            j<Y_DIM_COEFF_SIZE; j++,k--)
        pr_coeffs[j] = coefficients[j];

    FLOATING_POINT //__attribute__((register))  
        pr_samples[TAP_SIZE+BLOCK_MEM_LCM];
    #pragma unroll TAP_SIZE+BLOCK_MEM_LCM
    for (uint j=0; j<TAP_SIZE+BLOCK_MEM_LCM; j++)
            pr_samples[j] = 0;

    FLOATING_POINT //__attribute__((register))  
        tmp_results[TAP_SIZE+BLOCK_SIZE+BLOCK_MEM_LCM];
    #pragma unroll TAP_SIZE+BLOCK_SIZE+BLOCK_MEM_LCM
    for (uint j=0; j<TAP_SIZE+BLOCK_SIZE+BLOCK_MEM_LCM; j++)
        tmp_results[j] = 0;

    int i;

    for (i=0; i<numSamples; i+=BLOCK_SIZE)
    {   
        bool read = i%BLOCK_MEM_LCM == 0;
        #pragma unroll BLOCK_MEM_LCM
        for (uint j=TAP_SIZE; j<TAP_SIZE+BLOCK_MEM_LCM; j++)
            if (read)
                pr_samples[j-1] = samples[i+(j-TAP_SIZE)];
            
        // Shift in BLOCK_SIZE
        #pragma unroll (TAP_SIZE+BLOCK_MEM_LCM)
        for (uint k=0; k<TAP_SIZE+BLOCK_MEM_LCM; k++)
            tmp_results[k] = tmp_results[k+BLOCK_SIZE];
        
        FLOATING_POINT pr_results[BLOCK_SIZE];
        #pragma unroll BLOCK_SIZE
        for (uint k=0; k<BLOCK_SIZE; k++)
            pr_results[k] = vec_mac_manual_256x(k, pr_samples,
                pr_coeffs); 

        #pragma unroll BLOCK_SIZE
        for (uint k=0; k<BLOCK_SIZE; k++)
            tmp_results[TAP_SIZE+BLOCK_MEM_LCM+k] = pr_results[k];
        
        // Shift in BLOCK_SIZE
        #pragma unroll TAP_SIZE+BLOCK_MEM_LCM 
        for (uint k=0; k<TAP_SIZE+BLOCK_MEM_LCM; k++)
            pr_samples[k] = pr_samples[k+BLOCK_SIZE];
    
        bool write = i != 0 && i%BLOCK_MEM_LCM == 0;
        
        #pragma unroll BLOCK_MEM_LCM
        for (int k=TAP_SIZE,l=0; k<TAP_SIZE+BLOCK_MEM_LCM; k++,l++)
            if (write)
                results[i-BLOCK_MEM_LCM+l] = tmp_results[k];
    }

     // Shift in BLOCK_SIZE
    #pragma unroll (TAP_SIZE+BLOCK_MEM_LCM)
    for (uint k=0; k<TAP_SIZE+BLOCK_MEM_LCM; k++)
        tmp_results[k] = tmp_results[k+BLOCK_SIZE];

    #pragma unroll BLOCK_MEM_LCM
    for (int k=TAP_SIZE; k<TAP_SIZE+BLOCK_MEM_LCM; k++)
        results[((i-BLOCK_MEM_LCM)+(k-(TAP_SIZE)))] = tmp_results[k];

}
