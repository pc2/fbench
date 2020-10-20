/****************************************************************************
* @author MEHMET UFUK BÜYÜK ¸SAHIN
****************************************************************************/
#define SIZE 32

ulong mult(uint A, uint B)
{
    ulong _A, _B, _C;
    _A = A, _B = B, _C = 0;

    #pragma unroll
    for(int i=0; i<32; i++)
    {
        if ((_B & 0x1) == 0x1)
        {
            _C ^= _A;
        }
        _A <<= 1;
        _B >>= 1;
    }
    return _C;
}

__kernel
__attribute__((reqd_work_group_size(SIZE, 1, 1)))
//__attribute__((num_simd_work_items(2)))
//__attribute__((num_compute_units(2)))
void Montgomery(
    __global const uint * restrict A,
    __global const uint * restrict B,
    __global const uint * restrict N,
    __global const uint * restrict N_,
    __global uint * restrict tl,
    __global uint * restrict m,
    __global uint * restrict C)
{
    const int sizeR = SIZE+1;

    size_t i = get_global_id(0);
    size_t j = 0;

    for(int ii=0; ii<SIZE; ii++){
        C[ii] = 0;
    }

    for(int jj=0; jj<SIZE*2; jj++){
        tl[jj] = 0;
        m[jj] = 0;
    }

    ulong tmp=0;

    // printf("A: %d\n",A[i]);
    // printf("B: %d\n",B[i]);
    // printf("N: %d\n",N[i]);
    // printf("N': %d\n",N_[i]);
    

    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    // *************** [ t = A * B ] *************** //
    for(j=0; j<SIZE; j++)
    {
        tmp = mult(A[i], B[j]);
        //printf("tmp: %lu\n",tmp);
        tl[i+j] ^= (tmp & 0xFFFFFFFF);
        //printf("tl i+j: %d\n",tl[i+j]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
        tl[i+j+1] ^= (tmp >> 32);
        //printf("tl i+j+1: %d\n",tl[i+j+1]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    }

    // *************** [ m = tN_ ] *************** //
    for(j=0; j<SIZE*2; j++)
    {
        tmp = mult(tl[j], N_[i]);
        m[i+j] ^= (tmp & 0xFFFFFFFF);
        //printf("m i+j: %d\n",m[i+j]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
        m[i+j+1] ^= (tmp >> 32);
        //printf("m i+j+1: %d\n",m[i+j+1]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    }

    // *************** [ m = tN_ mod R ] *************** //
    // if(i +sizeR < 2 * SIZE) m[i+sizeR] = 0;
    // barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

    // *************** [ u = t ^ mN ] *************** //
    for(j=0; j<sizeR; j++)
    {
        tmp = mult(m[j], N[i]);
        tl[i+j] ^= (tmp & 0xFFFFFFFF);
        //printf("tl i+j: %d\n",tl[i+j]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
        tl[i+j+1] ^= (tmp >> 32);
        //printf("tl i+j+1: %d\n",tl[i+j+1]);
        barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    }
    
    C[i] = tl[i+sizeR];
    barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
    //printf("C: %d\n",C[i]);
    
}
