#ifndef SUPPORT_H
#define SUPPORT_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/opencl.h>

/****************************************************************************
* @file support.h
*
* <b>Purpose:<b> Various OpenCL-related support routines.
*
* @author Jeremy Meredith
* @date June 12, 2009
****************************************************************************/

inline const char *CLErrorString(cl_int err)
{
    switch (err)
    {
      case CL_SUCCESS:                         return "CL_SUCCESS";                         // break;
      case CL_DEVICE_NOT_FOUND:                return "CL_DEVICE_NOT_FOUND";                // break;
      case CL_DEVICE_NOT_AVAILABLE:            return "CL_DEVICE_NOT_AVAILABLE";            // break;
      case CL_COMPILER_NOT_AVAILABLE:          return "CL_COMPILER_NOT_AVAILABLE";          // break;
      case CL_MEM_OBJECT_ALLOCATION_FAILURE:   return "CL_MEM_OBJECT_ALLOCATION_FAILURE";   // break;
      case CL_OUT_OF_RESOURCES:                return "CL_OUT_OF_RESOURCES";                // break;
      case CL_OUT_OF_HOST_MEMORY:              return "CL_OUT_OF_HOST_MEMORY";              // break;
      case CL_PROFILING_INFO_NOT_AVAILABLE:    return "CL_PROFILING_INFO_NOT_AVAILABLE";    // break;
      case CL_MEM_COPY_OVERLAP:                return "CL_MEM_COPY_OVERLAP";                // break;
      case CL_IMAGE_FORMAT_MISMATCH:           return "CL_IMAGE_FORMAT_MISMATCH";           // break;
      case CL_IMAGE_FORMAT_NOT_SUPPORTED:      return "CL_IMAGE_FORMAT_NOT_SUPPORTED";      // break;
      case CL_BUILD_PROGRAM_FAILURE:           return "CL_BUILD_PROGRAM_FAILURE";           // break;
      case CL_MAP_FAILURE:                     return "CL_MAP_FAILURE";                     // break;
      case CL_INVALID_VALUE:                   return "CL_INVALID_VALUE";                   // break;
      case CL_INVALID_DEVICE_TYPE:             return "CL_INVALID_DEVICE_TYPE";             // break;
      case CL_INVALID_PLATFORM:                return "CL_INVALID_PLATFORM";                // break;
      case CL_INVALID_DEVICE:                  return "CL_INVALID_DEVICE";                  // break;
      case CL_INVALID_CONTEXT:                 return "CL_INVALID_CONTEXT";                 // break;
      case CL_INVALID_QUEUE_PROPERTIES:        return "CL_INVALID_QUEUE_PROPERTIES";        // break;
      case CL_INVALID_COMMAND_QUEUE:           return "CL_INVALID_COMMAND_QUEUE";           // break;
      case CL_INVALID_HOST_PTR:                return "CL_INVALID_HOST_PTR";                // break;
      case CL_INVALID_MEM_OBJECT:              return "CL_INVALID_MEM_OBJECT";              // break;
      case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; // break;
      case CL_INVALID_IMAGE_SIZE:              return "CL_INVALID_IMAGE_SIZE";              // break;
      case CL_INVALID_SAMPLER:                 return "CL_INVALID_SAMPLER";                 // break;
      case CL_INVALID_BINARY:                  return "CL_INVALID_BINARY";                  // break;
      case CL_INVALID_BUILD_OPTIONS:           return "CL_INVALID_BUILD_OPTIONS";           // break;
      case CL_INVALID_PROGRAM:                 return "CL_INVALID_PROGRAM";                 // break;
      case CL_INVALID_PROGRAM_EXECUTABLE:      return "CL_INVALID_PROGRAM_EXECUTABLE";      // break;
      case CL_INVALID_KERNEL_NAME:             return "CL_INVALID_KERNEL_NAME";             // break;
      case CL_INVALID_KERNEL_DEFINITION:       return "CL_INVALID_KERNEL_DEFINITION";       // break;
      case CL_INVALID_KERNEL:                  return "CL_INVALID_KERNEL";                  // break;
      case CL_INVALID_ARG_INDEX:               return "CL_INVALID_ARG_INDEX";               // break;
      case CL_INVALID_ARG_VALUE:               return "CL_INVALID_ARG_VALUE";               // break;
      case CL_INVALID_ARG_SIZE:                return "CL_INVALID_ARG_SIZE";                // break;
      case CL_INVALID_KERNEL_ARGS:             return "CL_INVALID_KERNEL_ARGS";             // break;
      case CL_INVALID_WORK_DIMENSION:          return "CL_INVALID_WORK_DIMENSION";          // break;
      case CL_INVALID_WORK_GROUP_SIZE:         return "CL_INVALID_WORK_GROUP_SIZE";         // break;
      case CL_INVALID_WORK_ITEM_SIZE:          return "CL_INVALID_WORK_ITEM_SIZE";          // break;
      case CL_INVALID_GLOBAL_OFFSET:           return "CL_INVALID_GLOBAL_OFFSET";           // break;
      case CL_INVALID_EVENT_WAIT_LIST:         return "CL_INVALID_EVENT_WAIT_LIST";         // break;
      case CL_INVALID_EVENT:                   return "CL_INVALID_EVENT";                   // break;
      case CL_INVALID_OPERATION:               return "CL_INVALID_OPERATION";               // break;
      case CL_INVALID_GL_OBJECT:               return "CL_INVALID_GL_OBJECT";               // break;
      case CL_INVALID_BUFFER_SIZE:             return "CL_INVALID_BUFFER_SIZE";             // break;
      case CL_INVALID_MIP_LEVEL:               return "CL_INVALID_MIP_LEVEL";               // break;
      case CL_INVALID_GLOBAL_WORK_SIZE:        return "CL_INVALID_GLOBAL_WORK_SIZE";        // break;
      case CL_INVALID_PROPERTY:                return "CL_INVALID_PROPERTY";                // break;
      default:                                 return "UNKNOWN";                            // break;
  }
}

#define CL_CHECK_ERROR(err) \
{                       \
	if (err != CL_SUCCESS)                  \
	{ \
	    std::cerr << __FILE__ << ':' << __LINE__ << ": " << CLErrorString(err) << std::endl; \
	    exit(1); \
	} \
}

#endif