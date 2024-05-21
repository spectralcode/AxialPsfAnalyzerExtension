#include "gaussfit.h"

GaussFit::GaussFit(const Eigen::VectorXd &xData, const Eigen::VectorXd &yData)
	: m_xData(xData), m_yData(yData), m_params(4), gaussFunction(1.0, 1.0, 1.0, 1.0) {
	// Initial parameters for k, m, s
	m_params << 10.0, 10.0, 10.0, 10.0;
}

void GaussFit::fit() {
	GaussFunctor functor(m_xData, m_yData);
	Eigen::NumericalDiff<GaussFunctor> numDiff(functor);
	Eigen::LevenbergMarquardt<Eigen::NumericalDiff<GaussFunctor>, double> lm(numDiff);
	lm.parameters.maxfev = 10000;   // Maximum number of function evaluations
	lm.parameters.xtol = 1e-6;    // Tolerance for the parameter change
	lm.parameters.ftol = 1e-6;    // Tolerance for the cost function change
	lm.parameters.gtol = 1e-6;    // Tolerance for the gradient

	int info = lm.minimize(m_params);

	std::cout << "Minimization info: " << info << std::endl;
	std::cout << "Fitted parameters: " << m_params.transpose() << std::endl;

	gaussFunction.setA(m_params[0]);
	gaussFunction.setK(m_params[1]);
	gaussFunction.setM(m_params[2]);
	gaussFunction.setS(m_params[3]);
}

void GaussFit::setInitialGuessForM(double m) {
	m_params[2] = m;
}
