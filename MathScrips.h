#pragma once
#ifndef __MATHSCRIPTS_H
#define __MATHSCRIPTS_H
#include <math.h>
inline double Sqr(double v) { return v*v; }
inline double Divide(double n, double d) { return d ? n / d : 0.0;  }
inline double CalcStd(double * data, int Start, int End, double * AVG = 0) {
	double Sum = 0., Sum2 = 0.;
	int N = End - Start + 1;
	if (N <= 0) return 0.;
	for (int i = Start; i <= End; i++) {
		Sum += data[i];
		Sum2 += Sqr(data[i]);
	}
	if (AVG) { *AVG = Sum / N;  }
	return sqrt(Divide(Sum2 - Sqr(Sum) / N, N - 1));
}
inline double Regress(double * X, double *Y, int Begin, int End, double * Yintcp = 0) {
	double sx = 0., sy = 0., st2 = 0., slp = 0.;
	int Npts = End - Begin + 1;
	for (int i = Begin; i <= End; i++) {
		sx += X[i];
		sy += Y[i];
	}
	double xmean = sx / Npts;
	for (int i = Begin; i <= End; i++) {
		double t = X[i] - xmean;
		st2 += Sqr(t);
		slp += t*Y[i];
	}
	if (!st2) {
		if (Yintcp) *Yintcp = 0.;
		return 0;
	}
	if (Yintcp) {
		*Yintcp = (sy - sx*slp) / Npts;
	}
	return slp;
}
#endif