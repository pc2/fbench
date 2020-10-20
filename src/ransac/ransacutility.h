#ifndef RANSACUTILITY_H
#define RANSACUTILITY_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

typedef struct {
    int x;  // tail
    int y;  // tail
    int vx; // head
    int vy; // head
} flowvector;

typedef struct {
    float x;  
    float y;  
} point;

int readInputSize(string fileName);
void readInputData(flowvector *v, string fileName);
void readInputData(point *p, string fileName);
void genRandNumbers(int *r, int maxIter, int n);
void verify(flowvector *flow_vector_array, int size_flow_vector_array, int *random_numbers, int max_iter,
    int error_threshold, float convergence_threshold, int candidates, int b_outliers);
void verify(point *point_array, int size_point_array, int *random_numbers, int max_iter,
    int error_threshold, float convergence_threshold, int candidates, int b_outliers);

#endif // RANSACUTILITY_H
