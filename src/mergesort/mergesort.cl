/** @file mergesort.cl
*
* Implementation of k-way merge sort algorithm
*
* Source reference: 
* https://coldfunction.com/mgen/p/3z?fbclid=IwAR3nCzg7-3lLcZUPcnjc2zXXO70HvhEpjaT5wytIHGHC1AnHfud35H_LeOw 
*
*/

#define NUM_OF_SUBARR 8

#define SIZE_OF_SUBARR 8


//Min function
inline uint minimum(uint x, uint y)
{
	if(x < y)
		return x;
		
	else
		return y;
	
}

//Merges two arrays into another array with given range
inline void mergeTwoArrays(int* output_arr, int* input_arr, uint start, uint mid, uint end) 
{
    uint i = start, j = mid, k = start;
	
	// Compare two elements from two input (sorted) sub-arrays and 
	// put the smaller one in the resultant array.
	// i,j are indices for input arrays, and k for output array.	
	for(k = start; k < end; k++)
    {
		int val_i = input_arr[i];
		int val_j = input_arr[j];
		
		bool should_update_i = false;
		
        if(i < mid && (j >= end || val_i < val_j))
		{
			
			should_update_i = true;
			i++;
		}
		
		else
		{
			should_update_i = false;
			j++;
		}
		
		output_arr[k] = (should_update_i) ? val_i : val_j;
		
	}
	
	//Transfer the elements from the output array to the input array
    for (i = start; i < end; i++) 
	{
        input_arr[i] = output_arr[i];
    }
}

/*********************************************************************
* Kernel: mergesort
*
* Purpose:
*	Sorts pre-sorted arrays into a single array.
* 
* @param TOTAL_SIZE length of the input and output arrays
* @param NUM_OF_SUBARR number of input (sorted) sub-arrays
* @param SIZE_OF_SUBARR length of each input sub-array
*********************************************************************/


__kernel 
void mergesort(global int* restrict input_buffer,
	global int* restrict output_buffer,
	uint n) 
{
	const uint TOTAL_SIZE = NUM_OF_SUBARR * SIZE_OF_SUBARR;
	
	int input_arr[TOTAL_SIZE];
	int output_arr[TOTAL_SIZE];
	
	//Copy data from global storage to local storage
	for(uint i = 0; i < TOTAL_SIZE; i++)
	{
		input_arr[i] = input_buffer[i];
	}
	

	//Initialize output_arr
	for(uint t = 0; t < TOTAL_SIZE; t++)
	{
		output_arr[t] = input_arr[t];
	}
	
	//Call mergeTwoArrays function
	for(uint len = SIZE_OF_SUBARR; len <= TOTAL_SIZE; len *= 2)
    {
        for(uint i = 0; (i+len) < TOTAL_SIZE; i += (2*len))
        {
            mergeTwoArrays(output_arr, input_arr, i, i + len, minimum(i + 2 * len, TOTAL_SIZE));
        }
    }

	//Sending back the result from local memory to global memory
	for(uint i = 0; i < TOTAL_SIZE; i++)
	{
		output_buffer[i] = output_arr[i];
	}
		
}
