# FBench

The FBench (FPGA benchmarking) suite consists of a number of benchmark applications to test various performance characteristics of an FPGA accelerator using OpenCL. The initial efforts of the development are to target Intel's FPGA accelerators only. It can be extended to target device from multiple vendors, e.g. Intel, Xilinx and others.

OpenCL is an open standard for programming various types of computing devices. The OpenCL specification describes a language for programming kernels to run on an OpenCL-capable device, and an Application Programming Interface (API) for transferring data to such devices and executing kernels on them.

## Benchmark programs
- **md5hash**: computation of many small MD5 digests, heavily dependent on bitwise operations.
- **scan**: scan (also known as parallel prefix sum) on an array of single or double precision floating point values.
- **firfilter**: convolution of input samples of a signal by a number of coefficients resulting as applying a fir filter on it, heavily dependent on multiplication and addition operations on double precision floating point values.
- **ransac**: random sample consensus used for model estimation (here: ego-motion estimation using first order flow (F-o-F) model, simple linear function estimation)
- **mm**: Compute modulo of arbitary percision large integer for cryposystems using Montgomery Multiplication
- **nw**: Needleman-Wunsch sequence allignement
- **mergesort**: k-way merge sort (also known as multiway merge sort) merges a number of input (sorted) arrays into a single resultant output array in sorted order; operates on integer values and heavily dependent on bitwise compare operations.

## Folder structure

<pre>
fbench  
 --- src  
     --- common             // Common source files.
     --- md5                // Md5 benchmark related folder.
     --- scan               // Scan benchmark related folder.
     --- firfilter          // FIR-Filter benchmark related folder.
     --- ransac             // Ransac benchmark related folder.
     --- mm                 // Montgomery Multiplication benchmark related folder.
     --- nw                 // Needleman-Wunsch benchmark related folder.
	 --- mergesort         	// Merge Sort benchmark related folder.
     --- mainhost.cpp       // Main host logic.
     --- ...  
 --- test                   // Unit test related source files.
     --- ...  
</pre>


## Platforms
- Linux

## Dependencies
- GCC 
- CMake (Version 3.15)
- Boost 
- Intel FPGA SDK for OpenCL 
- GMP (For calculating arbitrary percision large integer on CPU for Montgomery Multiplication)

### Optional
- Google Test (C++)
- Doxygen

## Building and setting up FBench
- Create a `build` directory from which you run the following commands:

        cmake ../                       # Generate Makefiles
        make                            # Compile main host (also compiles maintest)
        make <kernel_name>_synthesis    # AOC synthesis
        
        # alternatively use make all_kernels_<emulate/report/synthesis>
        # to compile all existing kernels of the benchmark suite

- Preprocessor directives:
    - These can be set by adding any of the following options to the cmake command via the -D argument (e.g. `-DMD5_LEVEL_OF_PARALLELISM=5`)
    - **md5**: 
        - MD5_LEVEL_OF_PARALLELISM: This sets the parameter TERMINAL_LOOP_SIZE, determining the number of hashes being calculated in parallel
    - **scan**:
        - SCANBSIZE: This changes the block size of data processed in parallel (Default: 16)
    - **firfilter**:
        - TAP_SIZE: Defines the order (delay taps) of lter design. For the provided input data workload it should be 256 .
        - BLOCK_SIZE: Defines the size of block for the number of samples to be filtered at a time.
        - MEM_BLOCK_SIZE: Defines the size of block for the global memory access. It should be equal to or lesser than the BLOCK_SIZE
        - BLOCK_MEM_LCM: For the kernels which allow non-aligned BLOCK_SIZE and MEM_BLOCK_SIZE values, this should have their least common multiple (LCM) in its value.
    - **ransac**:
        - RANSAC_CU: This sets the parameter CU, determining the number of compute units and thus the number of model parameters to generate in parallel (expects: any number that is a divisor of the ransac iterations)
        - RANSAC_PO: This sets the parameter PO, determining the number of outlier checks done in parallel within one CU (expects: multiples of 4, that are divisors of the data set size)
        - RANSAC_N: This sets the data set size N 
    - **nw**:
        - NWBSIZE: This defines the block size (Default: 16)
        - NWPAR: This defines the parallel factor (Default: 4)

- Host executable and .aocx files will be placed under `fbench/build/bin` after compilation

## Running FBench

`$INSTALL_DIR$/fbench [<Args>]`   

**Note:** *For running fir filter benchmark, for now you will need to copy the `.../src/firfilter/data` directory to the directory where the benchmark suite executable is situated.*

### Cmd arguments

#### Syntax

`<Args>  :=  [--kerneldir <kernel-dir-path>]`  
`            [--benchmark/-b <benchmarks-to-run-names>]`  
`            [--config/-c <configuration-file-name>]`  
`            [--dumpxml/-x]`  
`            [--dumpjson/-j]`  
`            [--platform/-p <integer-platform-id>]`  
`            [--device/-d <integer-platform-id>]`  
`            [--verbose/-v]`  
`            [--quite/-q]`  
`            [--kernel/-k <kernel-file-names>]`  
`            [--md5kernel/-y <md5-kernel-file-names>]`  
`            [--scankernel/-z <scan-kernel-file-names>]`  
`            [--mmkernel/-z <Montgomery-Mutliplication-kernel-file-names>]`  
`            [--nwkernel/-z <Needleman-Wunsch-kernel-file-names>]`  
`            [--size/-s <integer-problem-size>]`  
`            [--passes/-n <integer-test-passes>]`  
`            [--iterations/-i <integer-test-iterations>]`  
`            [--inputdir <firfilter-input-files-directory>]`    
`            [--group <firfilter-input-files-group>]`   

#### Arguments' definitions

##### General

 `kerneldir  `      : The directory in which the sythesized bitstreams for the kernels are residing.    
 `benchmark  `      : Comma separated names of the benchmarks to run or 'all' (default: all). E.g.: md5, scan or firfilter.  
 `config     `      : The name of the configuration file (default: config.json)  
 `dumpxml    `      : Specify for dumping results in a xml file (default: not specified).  
 `dumpjson   `      : Specify for dumping results in a json file (default: not specified).  
 `platform   `      : Specify the OpenCL platform to use (default: -1).  
 `device     `      : Specify the device to run the benchmarks on (default: -1).    
 `verbose    `      : Specify to enable verbose output (default: not specified).    
 `quite      `      : Specify to enable quiet output (default: not specified).  
*`kernel     `      : The name of the kernel bitstream file (default: value of --benchmark + .aocx). To be used in JSON config file only.  
 `md5kernel  `      : The name of the md5 kernel bitstream file (default: value of --benchmark + .aocx).  
 `scankernel `      : The name of the scan kernel bitstream file (default: value of --benchmark + .aocx).  
 `firfilterkernel ` : The name of the fir filter kernel bitstream file (default: value of --benchmark + .aocx).  
 `ransackernel `    : The name of the ransac kernel bitstream file (default: value of --benchmark + .aocx).  
 `mmkernel `        : The name of the Montgomery Mutliplication kernel bitstream file (default: value of --benchmark + .aocx).     
 `nwkernel `        : The name of the Needleman-Wunsch kernel bitstream file (default: value of --benchmark + .aocx).    
 `mergesortkernel ` : The name of the Merge Sort kernel bitstream file (default: value of --benchmark + .aocx).  
 `size       `      : The problem size (default: 1).    
 `passes     `      : The number of passes of each benchmark specified (default: 10).     
 `iterations `      : The number of iterations for specific benchmarks (default: 256).     
 `inputdir `        : The directory name where input files are place for firfilter (default: "data/").   
 `group `           : The group number of input files for firfilter (default: 1).     
 `model`            : The model on which the ransac algorithm shall be performed (fv: flowvectors in local memory, fvg: flowvectors in global memory, p: linear function).     
 `ifile`            : The input file containing the data set for ransac

When the benchmark suite is ran without any specified arguments, it will look for config.json file in the installation directory and try to read the settings/configurations for the benchmarks from there, if it could not locate it there then the application will check if the necessary arguments are specified, if not the program will terminate. Specification of any aforementioned argumnet will be overriding the values specified in the file if it is there. For instance if `--passes 4` is specified in the command line argument(s), the application will assume 4 passes for all the benchmarks it is going to run. 

Examples of running the suite with command line arguments:

*  `./mainhost -n 1 -p 0 -d 0 -v -b scan,md5 --md5kernel md5_emulate.aocx --scankernel scan_emulate.aocx`
*  `./mainhost -n 1 -p 0 -d 0 -v -b all --md5kernel md5_emulate.aocx --scankernel scan_emulate.aocx`
*  `./mainhost -n 1 -p 0 -d 0 -v -b md5 --md5kernel md5_emulate.aocx`

#### Configuration file 

The configuration JSON file will look something like the following:

```
{
    "host": {
                "kerneldir": "...",
                "dumpxml": "...",
                "dumpjson": "...",
                "platform": "...",
                "device": "...",
                "verbose": "...",
                "quite": "..."
            },
    "scan": {
                "kernel": "...",
                "size": "...",
                "passes": "...",
                "iteration": "...",
                ...
            },
    "md5": {
                "kernel": "...",
                "size": "...",
                "passes": "..."
                ...
            }
    "firfilter": {
            "kernel": "...",
            "passes": "..."
            ...
        }
    ...
}  
```

When using configuration file the benchmark suite can be provided with different values per benchmark application, unlike as in cmd arguments where the argument(s) apply on all the benchmark application which will be run.

## Benchmark results
### Formats

Based on the settings of the benchmark application, it can either give results of the benchmarks on the console or in file(s).

- Console: Will be simple formatted text, with the benchmark specific details and units (An example would be nice).
- File (JSON/XML/etc.): Will be marked up per benchmark specific units (An example would be nice). 

### Unit definition

- md5:          Giga hashes per second (GHash/Sec)
- scan:         Giga binary bytes per second (GiB/Sec)
- firfilter:    Giga samples per second (GSample/Sec)
- ransac:       Iterations per second (GB/Sec)
- mm:           Operations per second (Op/Sec)
- nw:           Giga element per second (GigaElement/Sec)
- mergesort:	Elements per second (elements/s)
