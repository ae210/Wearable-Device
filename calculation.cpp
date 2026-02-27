#include <math.h>
#include "config.h"
#include "calculation.h"

double a0, a1, a2, a3;
int MO = 0;

void calculation() {
    x[MO] = MO + 2;
    y[MO] = ave;
    a0 = 0; a1 = 0; a2 = 0; a3 = 0;
    sai(x, y);

    for (int i = 0; i < 41; i++) {
        x_val[i] = (i / 10.0) + 2.0;
        double val = x_val[i];
        adc_val[i] = a0 * (val * val * val) + a1 * (val * val) + a2 * val + a3;
    }
}

void sai(double x[CALC_S], double y[CALC_S]) {
    int i, j, k;
    double A[MY_S_VALUE][MY_S_VALUE + 1], coef[CALC_N];

    for (i = 0; i < MY_S_VALUE; i++)
        for (j = 0; j < MY_S_VALUE + 1; j++)
            A[i][j] = 0.0;

    for (i = 0; i < MY_S_VALUE; i++)
        for (j = 0; j < MY_S_VALUE; j++)
            for (k = 0; k < CALC_S; k++)
                A[i][j] += pow(x[k], i + j);

    for (i = 0; i < MY_S_VALUE; i++)
        for (k = 0; k < CALC_S; k++)
            A[i][MY_S_VALUE] += pow(x[k], i) * y[k];

    gauss(A, coef, &a0, &a1, &a2, &a3);
}

void gauss(double a[CALC_N][CALC_N + 1], double coef[CALC_N], double *a0, double *a1, double *a2, double *a3) {
    int i, j, k, l, pivot;
    double p, q, m, b[1][CALC_N + 1];

    for (i = 0; i < CALC_N; i++) {
        m = 0; pivot = i;
        for (l = i; l < CALC_N; l++) {
            if (fabs(a[l][i]) > m) {
                m = fabs(a[l][i]); pivot = l;
            }
        }
        if (pivot != i) {
            for (j = 0; j < CALC_N + 1; j++) {
                b[0][j] = a[i][j];
                a[i][j] = a[pivot][j];
                a[pivot][j] = b[0][j];
            }
        }
    }

    for (k = 0; k < CALC_N; k++) {
        p = a[k][k];
        a[k][k] = 1;
        for (j = k + 1; j < CALC_N + 1; j++) a[k][j] /= p;
        for (i = k + 1; i < CALC_N; i++) {
            q = a[i][k];
            for (j = k + 1; j < CALC_N + 1; j++) a[i][j] -= q * a[k][j];
            a[i][k] = 0;
        }
    }

    for (i = CALC_N - 1; i >= 0; i--) {
        coef[i] = a[i][CALC_N];
        for (j = CALC_N - 1; j > i; j--) coef[i] -= a[i][j] * coef[j];
    }

    *a0 = coef[3]; *a1 = coef[2]; *a2 = coef[1]; *a3 = coef[0];
}