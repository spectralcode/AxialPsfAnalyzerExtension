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
	GaussFit(const Eigen::VectorXd &xData, const Eigen::VectorXd &yData);
	void fit();
	void setInitialGuessForM(double m);
	Eigen::VectorXd getParams() const { return m_params; }

private:
	Eigen::VectorXd m_params;  // Parameters k, m, s
	Eigen::VectorXd m_xData;   // Data points x
	Eigen::VectorXd m_yData;   // Observed values y
	GaussFunction gaussFunction;
};

#endif // GAUSSFIT_H
