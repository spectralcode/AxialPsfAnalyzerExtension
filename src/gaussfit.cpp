#include "gaussfit.h"

GaussFit::GaussFit(const Eigen::VectorXd &xDataInit, const Eigen::VectorXd &yDataInit)
	: xData(xDataInit),
	yData(yDataInit),
	params(4),
	gaussFunction(1.0, 1.0, 1.0, 1.0)
{
	// Initial parameters for a, k, m, s
	params << 10.0, 10.0, 10.0, 10.0;
}

void GaussFit::fit() {
	GaussFunctor functor(this->xData, this->yData);
	Eigen::NumericalDiff<GaussFunctor> numDiff(functor);
	Eigen::LevenbergMarquardt<Eigen::NumericalDiff<GaussFunctor>, double> lm(numDiff);
	lm.parameters.maxfev = 10000;   // Maximum number of function evaluations
	lm.parameters.xtol = 1e-6;    // Tolerance for the parameter change
	lm.parameters.ftol = 1e-6;    // Tolerance for the cost function change
	lm.parameters.gtol = 1e-6;    // Tolerance for the gradient

	lm.minimize(this->params);

	this->gaussFunction.setA(this->params[0]);
	this->gaussFunction.setK(this->params[1]);
	this->gaussFunction.setM(this->params[2]);
	this->gaussFunction.setS(this->params[3]);
}

void GaussFit::setInitialGuessForA(double a) {
	this->params[0] = a;
}

void GaussFit::setInitialGuessForM(double m) {
	this->params[2] = m;
}
