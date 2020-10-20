/** @file ransac_p.cl
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
 * Modifications:   + Changed model from F-o-F to simple linear function
 *                  + Changed Code to single work-item kernels
 *                  + Split RANSAC stages into multiple kernels
 *                  + Optimized the code for FPGA
 *                  + Added tunable parameters for scalability
 *
 * Date:            2020/06/21
 *
 */

#define BR  4       // Burst Read

typedef struct {
    float x;  
    float y;  
} point;

typedef struct {
    point p[4];
} point4;

typedef struct {
    point p[PO];
} pointN;

typedef struct {
    float m;
    float b;
} m_parameters;

typedef struct {
    uint eT;    // error Threshold
    uint nF;    // num points
    uint nI;    // num Iterations
} constants_in;

typedef struct {
    int vld;
    int ol;     // outliers
} constants_out;

#pragma OPENCL EXTENSION cl_intel_channels : enable

channel point4 ch_points_init __attribute__((depth(512)));
channel pointN ch_points[CU] __attribute__((depth(512)));
channel m_parameters ch_model_params[CU] __attribute__((depth(512)));
channel constants_in ch_constants_in[CU] __attribute__((depth(0)));
channel constants_out ch_constants_out[CU] __attribute__((depth(512)));

inline int gen_model_param(float x1, float y1, float x2, float y2, m_parameters* model_param) {
    int   ret = 1;
    float m, b;
    float tmp = x2 - x1;
    if(tmp == 0){
        tmp = 1;
        ret = 0;
    }

    m = (y2-y1)/tmp;
    b = y1 - (x1*m);

    model_param->m = m;
    model_param->b = b;

    return ret;
}

/****************************************************************************
* <b>Function:</b> ransac_data_handler()
*
* <b>Purpose:</b> Store all input data locally and provide it to the CUs
*
* @param points all points
* @param errorThreshold threshold for data points to be inliers
* @param n_points number of total points
* @param n_iterations number of iterations (random samples to take)
*
* @returns Void
*
* @author Jennifer Faj 
*   
****************************************************************************/

__kernel void ransac_data_handler(global point* restrict points, const uint errorThreshold, const uint n_points, const uint n_iterations)
{
    local point local_points[N/PO][PO];

    constants_in c_in;
    c_in.eT = errorThreshold;
    c_in.nF = n_points;
    c_in.nI = n_iterations;

    write_channel_intel(ch_constants_in[0], c_in);

    for(int i = 0; i < n_points/PO; i++){
        for(int j = 0; j < PO; j+=BR){
            point4 p4;
            #pragma unroll
            for(int k = 0; k < BR; k++){
                local_points[i][j + k] = points[i*PO + j + k];
                p4.p[k] = local_points[i][j + k];
            }
            write_channel_intel(ch_points_init, p4);
        }
    }

    for(int k = 0; k < n_iterations/CU; k++){
        for(int i = 0; i < n_points/PO; i++){
            pointN pN;
            #pragma unroll
            for(int j = 0; j < PO; j++){
                pN.p[j] = local_points[i][j];
            }
            write_channel_intel(ch_points[0], pN);
        }
    }
}

/****************************************************************************
* <b>Function:</b> ransac_model_gen()
*
* <b>Purpose:</b> Take two random samples and generate model parameters 
* using a simple linear function
*
* @param randomNumbers array of indices for random samples
* @param n_points number of total points
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
*  + Model to compute: simple linear function
*  + Store data locally
*  + Send model parameters via channels to outlier count CUs
*   
****************************************************************************/

__kernel void ransac_model_gen(global int* restrict randomNumbers, const uint n_points, const uint n_iterations)
{
    local point local_points[N];

    for(int i = 0; i < n_points; i += BR){
        point4 p4;
        p4 = read_channel_intel(ch_points_init);
        #pragma unroll
        for(int j = 0; j < BR; j++){
            local_points[i + j] = p4.p[j];
        }
    }

    for(int i = 0; i < n_iterations; i++){
        // select two random data points
        int randNum1 = randomNumbers[i * 2 + 0];
        int randNum2 = randomNumbers[i * 2 + 1];

        point p[2];
        p[0] = local_points[randNum1];
        p[1] = local_points[randNum2];

        int ret = 0;

        m_parameters modelParams;
        // Function to generate model parameters
        ret = gen_model_param(p[0].x, p[0].y, p[1].x, p[1].y, &modelParams);

        if(ret == 0){
            modelParams.m = -1;
            modelParams.b = -1;
        }

        write_channel_intel(ch_model_params[i % CU], modelParams);
    }
}

/****************************************************************************
* <b>Function:</b> ransac_outlier_count()
*
* <b>Purpose:</b> Take model parameters and check the whole data set for outliers 
* using a simple linear function
*
* @returns Void
*
* @author IMPACT Research Group
*
* <b>Modifiers:</b> Jennifer Faj 
*      
* <b>Modifications:</b>
*
*  + Model to compute: simple linear function
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
    uint errorThresholdSquared = c_in.eT*c_in.eT;
    uint n_points = c_in.nF;
    uint n_iterations = c_in.nI;
    if(cu_id < (CU-1)){
        write_channel_intel(ch_constants_in[cu_id + 1], c_in);
    }

    for(int i = 0; i < n_iterations/CU; i++){
        m_parameters modelParams;
        modelParams = read_channel_intel(ch_model_params[cu_id]);

        int vld = 1;
        if((modelParams.m == -1) && (modelParams.b == -1)){
            vld = 0;
        }

        float m2 = (-1)/modelParams.m;

        int outlierCount[PO];
        #pragma unroll PO
        for(int j = 0; j < PO; j++){
            outlierCount[j] = 0;
        }

        // Compute number of outliers
        for(int j = 0; j < n_points; j+= PO) {
            pointN pN = read_channel_intel(ch_points[cu_id]);

            #pragma unroll
            for(int k = 0; k < PO; k++){
                float qx, qy, b2, distErrorSquared;
                b2 = pN.p[k].y - (m2*pN.p[k].x);
                qx = (modelParams.b-b2)/(m2-modelParams.m);
                qy = (modelParams.m*qx) + modelParams.b;

                distErrorSquared = ((qy-pN.p[k].y) * (qy-pN.p[k].y)) + ((qx-pN.p[k].x) * (qx-pN.p[k].x));

                if((vld != 0) && (fabs(distErrorSquared) >= errorThresholdSquared)) {
                    outlierCount[k]++;
                } 
            }

            if(cu_id < (CU-1)){
                write_channel_intel(ch_points[cu_id + 1], pN);
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
* @param n_points number of total points
* @param n_iterations number of iterations (random samples to take)
* @param n_bestModelParams output - number of models that are above convergence threshold
* @param n_bestOutliers output - outlier count for best model
*
* @returns Void
*
* @author  Jennifer Faj 
*   
****************************************************************************/

__kernel void ransac_model_count(const uint n_points, const uint n_iterations, const float convergenceThreshold, global int* restrict n_bestModelParams, global int* restrict n_bestOutliers){
    
    int recvd = 0;

    int localBestOutliers = n_points;
    int suitableModelCount = 0;
    
    while(recvd < n_iterations){
        for(int i = 0; i < CU; i++){
            bool vld_read;
            constants_out c_out;
            c_out = read_channel_nb_intel(ch_constants_out[i], &vld_read);

            int vld = c_out.vld;
            int outliers = c_out.ol;

            if(vld_read){            
                if((vld != 0) && (outliers < n_points * convergenceThreshold)){
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