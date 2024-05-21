#ifndef GAUSSFUNCTION_H
#define GAUSSFUNCTION_H

#include <QtMath>

class GaussFunction
{
public:
	GaussFunction(double a, double k, double m, double s);

	double operator()(double x) const; // Function call operator to compute Gaussian value at x
	double getA() const;
	double getK() const;
	double getM() const;
	double getS() const;
	void setA(double newA);
	void setK(double newK);
	void setM(double newM);
	void setS(double newS);
	double getFWHM();

private:
	double a;
	double k; // vertical offset
	double m; // mean value (horizontal offset)
	double s; // standard deviation
};

#endif //GAUSSFUNCTION_H
