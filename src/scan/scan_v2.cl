#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_intel_channels : enable

#define II_CYCLES 6

__kernel void scan(__global const float* restrict in,
			__global float* restrict out,
			const uint n)
{
	float shift_reg[II_CYCLES + 1];

	#pragma unroll
	for (int i = 0 ; i < II_CYCLES + 1 ; i++)
	{
		shift_reg[i] = 0;
	}

	float last = 0;

	for (uint b=0 ; b<n ; b+=BLOCKSIZE)
	{
		float inBlock[BLOCKSIZE];
		float outBlock[BLOCKSIZE];

		//loading a block of elements to local
		#pragma unroll
		for(uint i = 0 ; i < BLOCKSIZE ; i++)
			inBlock[i] = in[i+b];


                //calculating the partial scan on current block
                #pragma unroll
                for (uint i = 0 ; i < BLOCKSIZE ; i++)
                {
                        float partial_sum = 0;
                        #pragma unroll
                        for (uint j = 0 ; j <= i ; j++)
                        {
                                partial_sum += inBlock[j];
                        }
                        outBlock[i] = partial_sum + last;
                }

                //saving a block of elements to global
                #pragma unroll
                for(uint i = 0 ; i < BLOCKSIZE ; i++)
                        out[i+b] = outBlock[i];

		float block_sum = 0;

		//calculating the new last result
		#pragma unroll
		for(int i = 0 ; i < BLOCKSIZE ; i++)
		{
			block_sum += inBlock[i];
		}

                shift_reg[II_CYCLES] = shift_reg[0] + block_sum;

		#pragma unroll
                for(int i = 0 ; i < II_CYCLES + 1 ; ++i)
                {
                        shift_reg[i] = shift_reg[i + 1];
                }

                float temp_sum = 0;

		#pragma unroll
                for(int i = 0 ; i < II_CYCLES ; ++i)
                {
                        temp_sum += shift_reg[i];
                }

                last = temp_sum;
	}
}
