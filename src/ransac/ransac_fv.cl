/** @file ransac_fv.cl
*/

/*
 * Copyright (c) 2016 University of Cordoba and University of Illinois
 * All rights reserved.
 *
 * Developed by:    IMPACT Research Group
 *                  University of Cordoba and University of Illinois
 *                  http://impact.crhc.illinois.edu/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *      > Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimers.
 *      > Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimers in the
 *        documentation and/or other materials provided with the distribution.
 *      > Neither the names of IMPACT Research Group, University of Cordoba, 
 *        University of Illinois nor the names of its contributors may be used 
 *        to endorse or promote products derived from this Software without 
 *        specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
 * THE SOFTWARE.
 *
 * Modifiers:       Jennifer Faj
 * Modifications:   + Changed Code to single work-item kernels
 *                  + Split RANSAC stages into multiple kernels
 *                  + Optimized the code for FPGA
 *                  + Added tunable parameters for scalability
 *
 * Date:            2020/06/21
 *
 */

#define BR  4       // Burst Read

typedef struct {
    int x;
    int y;
    int vx;
    int vy;
} flowvector;

typedef struct {
    flowvector fv[4];
} flowvector4;

typedef struct {
    flowvector fv[PO];
} flowvectorN;

typedef struct {
    float xc;
    float yc;
    float D;
    float R;
} m_parameters;

typedef struct {
    uint eT;    // error Threshold
    uint nF;    // num Flowvectors
    uint nI;    // num Iterations
} constants_in;

typedef struct {
    int vld;
    int ol;     // outliers
} constants_out;

#pragma OPENCL EXTENSION cl_intel_channels : enable

channel flowvector4 ch_flowvectors_init __attribute__((depth(512)));
channel flowvectorN ch_flowvectors[CU] __attribute__((depth(512)));
channel m_parameters ch_model_params[CU] __attribute__((depth(1024)));
channel constants_in ch_constants_in[CU] __attribute__((depth(0)));
channel constants_out ch_constants_out[CU] __attribute__((depth(512)));

inline int gen_model_param(int x1, int y1, int vx1, int vy1, int x2, int y2, int vx2, int vy2, m_parameters* model_param) {
    float temp1;
    float temp2;
    int ret = 1;
    // xc -> model_param[0], yc -> model_param[1], D -> model_param[2], R -> model_param[3]
    temp1 = (float)((vx1 * (vx1 - (2 * vx2))) + (vx2 * vx2) + (vy1 * vy1) - (vy2 * ((2 * vy1) - vy2)));
    if(temp1 == 0) { // Check to prevent division by zero
        ret = 0;
        model_param->xc = (((vx1 * ((-vx2 * x1) + (vx1 * x2) - (vx2 * x2) + (vy2 * y1) - (vy2 * y2))) +
                            (vy1 * ((-vy2 * x1) + (vy1 * x2) - (vy2 * x2) - (vx2 * y1) + (vx2 * y2))) +
                            (x1 * ((vy2 * vy2) + (vx2 * vx2)))) /
                            1);
        model_param->yc = (((vx2 * ((vy1 * x1) - (vy1 * x2) - (vx1 * y1) + (vx2 * y1) - (vx1 * y2))) +
                            (vy2 * ((-vx1 * x1) + (vx1 * x2) - (vy1 * y1) + (vy2 * y1) - (vy1 * y2))) +
                            (y2 * ((vx1 * vx1) + (vy1 * vy1)))) /
                            1);
    } else {
        model_param->xc = (((vx1 * ((-vx2 * x1) + (vx1 * x2) - (vx2 * x2) + (vy2 * y1) - (vy2 * y2))) +
                            (vy1 * ((-vy2 * x1) + (vy1 * x2) - (vy2 * x2) - (vx2 * y1) + (vx2 * y2))) +
                            (x1 * ((vy2 * vy2) + (vx2 * vx2)))) /
                            temp1);
        model_param->yc = (((vx2 * ((vy1 * x1) - (vy1 * x2) - (vx1 * y1) + (vx2 * y1) - (vx1 * y2))) +
                            (vy2 * ((-vx1 * x1) + (vx1 * x2) - (vy1 * y1) + (vy2 * y1) - (vy1 * y2))) +
                            (y2 * ((vx1 * vx1) + (vy1 * vy1)))) /
                            temp1);
    }

    temp2 = (float)((x1 * (x1 - (2 * x2))) + (x2 * x2) + (y1 * (y1 - (2 * y2))) + (y2 * y2));
    if(temp2 == 0) { // Check to prevent division by zero
        ret = 0;
        model_param->D = ((((x1 - x2) * (vx1 - vx2)) + ((y1 - y2) * (vy1 - vy2))) / 1);
        model_param->R = ((((x1 - x2) * (vy1 - vy2)) + ((y2 - y1) * (vx1 - vx2))) / 1);
    } else {
        model_param->D = ((((x1 - x2) * (vx1 - vx2)) + ((y1 - y2) * (vy1 - vy2))) / temp2);
        model_param->R = ((((x1 - x2) * (vy1 - vy2)) + ((y2 - y1) * (vx1 - vx2))) / temp2);
    }
    return ret;
}

/****************************************************************************
* <b>Function:</b> ransac_data_handler()
*
* <b>Purpose:</b> Store all input data locally and provide it to the CUs
*
* @param flowvectors all flowvectors
* @param errorThreshold threshold for data flowvectors to be inliers
* @param n_flowvectors number of total flowvectors
* @param n_iterations number of iterations (random samples to take)
*
* @returns Void
*
* @author Jennifer Faj 
*   
****************************************************************************/

__kernel void ransac_data_handler(global flowvector* restrict flowvectors, const uint errorThreshold, const uint n_flowvectors, const uint n_iterations)
{
    local flowvector local_flowvectors[N/PO][PO];

    constants_in c_in;
    c_in.eT = errorThreshold;
    c_in.nF = n_flowvectors;
    c_in.nI = n_iterations;

    write_channel_intel(ch_constants_in[0], c_in);

    for(int i = 0; i < n_flowvectors/PO; i++){
        for(int j = 0; j < PO; j+=BR){
            flowvector4 fv4;
            #pragma unroll
            for(int k = 0; k < BR; k++){
                local_flowvectors[i][j + k] = flowvectors[i*PO + j + k];
                fv4.fv[k] = local_flowvectors[i][j + k];
            }
            write_channel_intel(ch_flowvectors_init, fv4);
        }
    }

    for(int k = 0; k < n_iterations/CU; k++){
        for(int i = 0; i < n_flowvectors/PO; i++){
            flowvectorN fvN;
            #pragma unroll
            for(int j = 0; j < PO; j++){
                fvN.fv[j] = local_flowvectors[i][j];
            }
            write_channel_intel(ch_flowvectors[0], fvN);
        }
    }
}

/****************************************************************************
* <b>Function:</b> ransac_model_gen()
*
* <b>Purpose:</b> Take two random samples and generate model parameters 
* using the First-order-Flow model
*
* @param randomNumbers array of indices for random samples
* @param n_flowvectors number of total flowvectors
* @param n_iterations number of iterations (random samples to take)
*
* @returns Void
*
* @author IMPACT Research Group
*
* <b>Modifiers:</b> Jennifer Faj 
*      
* <b>Modifications:</b>
*
*  + Store data locally
*  + Send model parameters via channels to outlier count CUs
*  + Adjustment of computation for "invalid" models, 
*    so that the amount of computations stays balanced for comparability
*   
****************************************************************************/

__kernel void ransac_model_gen(global int* restrict randomNumbers, const uint n_flowvectors, const uint n_iterations)
{
    local flowvector local_flowvectors[N];

    for(int i = 0; i < n_flowvectors; i += BR){
        flowvector4 fv4;
        fv4 = read_channel_intel(ch_flowvectors_init);
        #pragma unroll
        for(int j = 0; j < BR; j++){
            local_flowvectors[i + j] = fv4.fv[j];
        }
    }

    for(int i = 0; i < n_iterations; i++){
        // select two random flow vectors
        int randNum1 = randomNumbers[i * 2 + 0];
        int randNum2 = randomNumbers[i * 2 + 1];

        flowvector fv[2];
        fv[0] = local_flowvectors[randNum1];
        fv[1] = local_flowvectors[randNum2];

        int ret = 0;

        int vx1 = fv[0].vx - fv[0].x;
        int vy1 = fv[0].vy - fv[0].y;
        int vx2 = fv[1].vx - fv[1].x;
        int vy2 = fv[1].vy - fv[1].y;

        m_parameters modelParams;
        // Function to generate model parameters according to F-o-F (xc, yc, D and R)
        ret = gen_model_param(fv[0].x, fv[0].y, vx1, vy1, fv[1].x, fv[1].y, vx2, vy2, &modelParams);

        if(ret == 0){
            modelParams.xc = -1;
            modelParams.yc = -1;
            modelParams.D = -1;
            modelParams.R = -1;
        }

        write_channel_intel(ch_model_params[i % CU], modelParams);
    }
}

/****************************************************************************
* <b>Function:</b> ransac_outlier_count()
*
* <b>Purpose:</b> Take model parameters and check the whole data set for outliers 
* using the First-order-Flow model
*
* @returns Void
*
* @author IMPACT Research Group
*
* <b>Modifiers:</b> Jennifer Faj 
*      
* <b>Modifications:</b>
*
*  + Separate Autorun kernels (channels to read input data & model params and write outliers)
*  + Multiple compute units (scalable)
*  + Multiple outlier computations in parallel (scalable)
*   
****************************************************************************/

__attribute__((max_global_work_dim(0)))
__attribute__((autorun))
__attribute__((num_compute_units(CU)))
__kernel void ransac_outlier_count()
{
    constants_in c_in;

    int cu_id = get_compute_id(0);

    c_in = read_channel_intel(ch_constants_in[cu_id]);
    uint errorThreshold = c_in.eT;
    uint n_flowvectors = c_in.nF;
    uint n_iterations = c_in.nI;
    if(cu_id < (CU-1)){
        write_channel_intel(ch_constants_in[cu_id + 1], c_in);
    }

    for(int i = 0; i < n_iterations/CU; i++){
        m_parameters modelParams;
        modelParams = read_channel_intel(ch_model_params[cu_id]);

        int vld = 1;
        if((modelParams.xc == -1) && (modelParams.yc == -1) && (modelParams.D == -1) && (modelParams.R == -1)){
            vld = 0;
        }

        int outlierCount[PO];
        #pragma unroll PO
        for(int j = 0; j < PO; j++){
            outlierCount[j] = 0;
        }

        // Compute number of outliers
        for(int j = 0; j < n_flowvectors; j+= PO) {
            flowvectorN fvN = read_channel_intel(ch_flowvectors[cu_id]);

            #pragma unroll
            for(int k = 0; k < PO; k++){
                float vxError, vyError;
                vxError = fvN.fv[k].x + ((int)((fvN.fv[k].x - modelParams.xc) * modelParams.D) -
                        (int)((fvN.fv[k].y - modelParams.yc) * modelParams.R)) - fvN.fv[k].vx;
                vyError = fvN.fv[k].y + ((int)((fvN.fv[k].y - modelParams.yc) * modelParams.D) +
                        (int)((fvN.fv[k].x - modelParams.xc) * modelParams.R)) - fvN.fv[k].vy;

                if((vld != 0) && ((fabs(vxError) >= errorThreshold) || (fabs(vyError) >= errorThreshold))) {
                    outlierCount[k]++;
                }
            }

            if(cu_id < (CU-1)){
                write_channel_intel(ch_flowvectors[cu_id + 1], fvN);
            }
        }

        int outliers = 0;
        #pragma unroll PO
        for(int j = 0; j < PO; j++){
            outliers += outlierCount[j];
        }

        constants_out c_out;
        c_out.vld = vld;
        c_out.ol = outliers;

        write_channel_intel(ch_constants_out[cu_id], c_out);
    }
}

/****************************************************************************
* <b>Function:</b> ransac_model_count()
*
* <b>Purpose:</b> Check if a better model was found by comparing outlier counts and 
* count the number of suitable models
*
* @param convergenceThreshold threshold for a suitable model
* @param n_flowvectors number of total flowvectors
* @param n_iterations number of iterations (random samples to take)
* @param n_bestModelParams output - number of models that are above convergence threshold
* @param n_bestOutliers output - outlier count for best model
*
* @returns Void
*
* @author IMPACT Research Group
*
* <b>Modifiers:</b> Jennifer Faj 
*      
* <b>Modifications:</b>
*   + Separate kernel
*   + Read outlier counts from CUs via channels
*   
****************************************************************************/

__kernel void ransac_model_count(const uint n_flowvectors, const uint n_iterations, const float convergenceThreshold, global int* restrict n_bestModelParams, global int* restrict n_bestOutliers){
    
    int recvd = 0;

    int localBestOutliers = n_flowvectors;
    int suitableModelCount = 0;
    
    while(recvd < n_iterations){
        for(int i = 0; i < CU; i++){
            bool vld_read;
            constants_out c_out;
            c_out = read_channel_nb_intel(ch_constants_out[i], &vld_read);

            int vld = c_out.vld;
            int outliers = c_out.ol;

            if(vld_read){            
                if((vld != 0) && (outliers < n_flowvectors * convergenceThreshold)){
                    suitableModelCount++;
                    if(outliers < localBestOutliers){
                        localBestOutliers = outliers;
                    }
                }
                recvd++;
            }
        }
    }

    *n_bestModelParams = suitableModelCount;
    *n_bestOutliers = localBestOutliers;
}