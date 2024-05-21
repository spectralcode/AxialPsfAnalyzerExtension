#ifndef GAUSSFIT_H
#define GAUSSFIT_H

#include "gaussfunction.h"
#include "optimizationfunctor.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <iostream>

class GaussFit {
public:
	GaussFit(const Eigen::VectorXd &xDataInit, const Eigen::VectorXd &yDataInit);
	void fit();
	void setInitialGuessForA(double a);
	void setInitialGuessForM(double m);
	Eigen::VectorXd getParams() const { return this->params; }
	GaussFunction getGaussianFunction() const { return this->gaussFunction; }

private:
	Eigen::VectorXd xData;   // Data points x
	Eigen::VectorXd yData;   // Observed values y
	Eigen::VectorXd params;  // Parameters k, m, s
	GaussFunction gaussFunction;
};

#endif // GAUSSFIT_H
