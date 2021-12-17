//
//  Academic License - for use in teaching, academic research, and meeting
//  course requirements at degree granting institutions only.  Not for
//  government, commercial, or other organizational use.
//
//  GaussNewtonSine.h
//
//  Code generation for function 'GaussNewtonSine'
//


#ifndef GAUSSNEWTONSINE_H
#define GAUSSNEWTONSINE_H

// Include files
#include "rtwtypes.h"
#include "coder_array.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
extern void GaussNewtonSine(const coder::array<double, 1U> &x, const coder::
  array<double, 1U> &y, const double beta0[4], const double lb[4], const double
  ub[4], double coeff[4], double *iter);

#endif

// End of code generation (GaussNewtonSine.h)
