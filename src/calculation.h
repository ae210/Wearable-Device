#ifndef __CALCULATION_H
#define __CALCULATION_H
#include "config.h"

#define MY_S_VALUE 4
#define CALC_S 5 
#define CALC_N 4

extern double a0, a1, a2, a3;
extern int MO;
extern int val[10]; 
extern double x[CALC_S], y[CALC_S];
extern double x_val[41];
extern double adc_val[41];

void calculation();
void sai(double x[CALC_S], double y[CALC_S]);
void gauss(double a[MY_S_VALUE][MY_S_VALUE + 1], double coef[MY_S_VALUE], double *a0, double *a1, double *a2, double *a3);

#endif