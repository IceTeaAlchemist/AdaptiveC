//
//  Academic License - for use in teaching, academic research, and meeting
//  course requirements at degree granting institutions only.  Not for
//  government, commercial, or other organizational use.
//
//  GaussNewtonSine.cpp
//
//  Code generation for function 'GaussNewtonSine'
//


// Include files
#include "GaussNewtonSine.h"
#include "coder_array.h"
#include <cmath>

// Function Definitions
void GaussNewtonSine(const coder::array<double, 1U> &x, const coder::array<
                     double, 1U> &y, const double beta0[4], const double lb[4],
                     const double ub[4], double coeff[4], double *iter)
{
  static const double b_y[16] = { 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0001, 0.0, 0.0,
    0.0, 0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0001 };

  coder::array<double, 2U> d;
  coder::array<double, 2U> j;
  double C[16];
  double da[4];
  int b_iter;
  int coffset;
  int i;
  int jy;
  signed char ipiv[4];
  boolean_T exitg1;

  //  Set function tolerance, this is the maximum change for convergence
  //  condition.
  //  Set the maximum number of iterations the program will take.
  //  Set up the initial guess.
  coeff[0] = beta0[0];
  coeff[1] = beta0[1];
  coeff[2] = beta0[2];
  coeff[3] = beta0[3];
  j.set_size(x.size(0), 4);
  jy = x.size(0) << 2;
  for (i = 0; i < jy; i++) {
    j[i] = 0.0;
  }

  d.set_size(1, x.size(0));
  jy = x.size(0);
  for (i = 0; i < jy; i++) {
    d[i] = 0.0;
  }

  //  Begin looping.
  *iter = 1.0;
  b_iter = 0;
  exitg1 = false;
  while ((!exitg1) && (b_iter < 26)) {
    double a;
    double b;
    double e;
    int b_j;
    int boffset;
    int k;
    *iter = static_cast<double>(b_iter) + 1.0;

    //  Set coefficient guesses.
    a = coeff[0];
    b = coeff[1];
    e = coeff[3];

    //  Create the Jacobian.
    i = x.size(0);
    for (jy = 0; jy < i; jy++) {
      double b_j_tmp;
      double j_tmp;
      j_tmp = b * x[jy] + e;
      b_j_tmp = std::sin(j_tmp);
      j[jy] = b_j_tmp;
      j_tmp = std::cos(j_tmp);
      j[jy + j.size(0)] = a * x[jy] * j_tmp;
      j[jy + j.size(0) * 2] = 1.0;
      j[jy + j.size(0) * 3] = a * j_tmp;
      d[jy] = y[jy] - (a * b_j_tmp + coeff[2]);
    }

    //  Damp the Jacobian to avoid poor conditioning and add it.
    jy = j.size(0);
    da[0] = 0.0;
    da[1] = 0.0;
    da[2] = 0.0;
    da[3] = 0.0;
    for (k = 0; k < jy; k++) {
      da[0] += j[k] * d[k];
      da[1] += j[j.size(0) + k] * d[k];
      da[2] += j[(j.size(0) << 1) + k] * d[k];
      da[3] += j[3 * j.size(0) + k] * d[k];
    }

    jy = j.size(0);
    for (b_j = 0; b_j < 4; b_j++) {
      coffset = b_j << 2;
      boffset = b_j * j.size(0);
      C[coffset] = 0.0;
      C[coffset + 1] = 0.0;
      C[coffset + 2] = 0.0;
      C[coffset + 3] = 0.0;
      for (k = 0; k < jy; k++) {
        a = j[boffset + k];
        C[coffset] += j[k] * a;
        C[coffset + 1] += j[j.size(0) + k] * a;
        C[coffset + 2] += j[(j.size(0) << 1) + k] * a;
        C[coffset + 3] += j[3 * j.size(0) + k] * a;
      }
    }

    for (i = 0; i < 16; i++) {
      C[i] += b_y[i];
    }

    ipiv[0] = 1;
    ipiv[1] = 2;
    ipiv[2] = 3;
    for (b_j = 0; b_j < 3; b_j++) {
      int b_tmp;
      int ix;
      int mmj_tmp;
      mmj_tmp = 2 - b_j;
      b_tmp = b_j * 5;
      boffset = b_tmp + 2;
      jy = 4 - b_j;
      coffset = 0;
      ix = b_tmp;
      a = std::abs(C[b_tmp]);
      for (k = 2; k <= jy; k++) {
        ix++;
        b = std::abs(C[ix]);
        if (b > a) {
          coffset = k - 1;
          a = b;
        }
      }

      if (C[b_tmp + coffset] != 0.0) {
        if (coffset != 0) {
          jy = b_j + coffset;
          ipiv[b_j] = static_cast<signed char>(jy + 1);
          a = C[b_j];
          C[b_j] = C[jy];
          C[jy] = a;
          a = C[b_j + 4];
          C[b_j + 4] = C[jy + 4];
          C[jy + 4] = a;
          a = C[b_j + 8];
          C[b_j + 8] = C[jy + 8];
          C[jy + 8] = a;
          a = C[b_j + 12];
          C[b_j + 12] = C[jy + 12];
          C[jy + 12] = a;
        }

        i = (b_tmp - b_j) + 4;
        for (jy = boffset; jy <= i; jy++) {
          C[jy - 1] /= C[b_tmp];
        }
      }

      jy = b_tmp + 4;
      coffset = b_tmp;
      for (k = 0; k <= mmj_tmp; k++) {
        a = C[jy];
        if (C[jy] != 0.0) {
          ix = b_tmp + 1;
          i = coffset + 6;
          boffset = (coffset - b_j) + 8;
          for (int ijA = i; ijA <= boffset; ijA++) {
            C[ijA - 1] += C[ix] * -a;
            ix++;
          }
        }

        jy += 4;
        coffset += 4;
      }
    }

    if (ipiv[0] != 1) {
      a = da[0];
      da[0] = da[ipiv[0] - 1];
      da[ipiv[0] - 1] = a;
    }

    if (ipiv[1] != 2) {
      a = da[1];
      da[1] = da[ipiv[1] - 1];
      da[ipiv[1] - 1] = a;
    }

    if (ipiv[2] != 3) {
      a = da[2];
      da[2] = da[ipiv[2] - 1];
      da[ipiv[2] - 1] = a;
    }

    if (da[0] != 0.0) {
      for (jy = 2; jy < 5; jy++) {
        da[jy - 1] -= da[0] * C[jy - 1];
      }
    }

    if (da[1] != 0.0) {
      for (jy = 3; jy < 5; jy++) {
        da[jy - 1] -= da[1] * C[jy + 3];
      }
    }

    if (da[2] != 0.0) {
      for (jy = 4; jy < 5; jy++) {
        da[3] -= da[2] * C[11];
      }
    }

    if (da[3] != 0.0) {
      da[3] /= C[15];
      for (jy = 0; jy < 3; jy++) {
        da[jy] -= da[3] * C[jy + 12];
      }
    }

    if (da[2] != 0.0) {
      da[2] /= C[10];
      for (jy = 0; jy < 2; jy++) {
        da[jy] -= da[2] * C[jy + 8];
      }
    }

    if (da[1] != 0.0) {
      da[1] /= C[5];
      for (jy = 0; jy < 1; jy++) {
        da[0] -= da[1] * C[4];
      }
    }

    if (da[0] != 0.0) {
      da[0] /= C[0];
    }

    //  Iterate.
    coeff[0] += da[0];
    coeff[1] += da[1];
    coeff[2] += da[2];
    coeff[3] += da[3];

    //  Invoke boundary conditions.
    if (coeff[0] < lb[0]) {
      coeff[0] = lb[0];
    }

    if (coeff[1] < lb[1]) {
      coeff[1] = lb[1];
    }

    if (coeff[2] < lb[2]) {
      coeff[2] = lb[2];
    }

    if (coeff[3] < lb[3]) {
      coeff[3] = lb[3];
    }

    if (coeff[0] > ub[0]) {
      coeff[0] = ub[0];
    }

    if (coeff[1] > ub[1]) {
      coeff[1] = ub[1];
    }

    if (coeff[2] > ub[2]) {
      coeff[2] = ub[2];
    }

    if (coeff[3] > ub[3]) {
      coeff[3] = ub[3];
    }

    //  Break on convergence.
    if ((std::abs(da[0]) < 0.001) && (std::abs(da[1]) < 0.001) && (std::abs(da[2])
         < 0.001) && (std::abs(da[3]) < 0.001)) {
      //         disp(out);
      //        disp('Method has converged.');
      exitg1 = true;
    } else {
      b_iter++;
    }
  }
}

// End of code generation (GaussNewtonSine.cpp)
