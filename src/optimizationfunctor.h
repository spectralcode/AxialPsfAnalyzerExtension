#ifndef OPTIMIZATIONFUNCTOR_H
#define OPTIMIZATIONFUNCTOR_H

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include "gaussfunction.h"

/***********************************************************************************************/
//this code is adapted from a stackoverflow post by MattKelly.
//see original code here: https://stackoverflow.com/questions/18509228/how-to-use-the-eigen-unsupported-levenberg-marquardt-implementation/49078011#49078011

// Generic functor
// See http://eigen.tuxfamily.org/index.php?title=Functors
// C++ version of a function pointer that stores meta-data about the function
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor
{

	// Information that tells the caller the numeric type (eg. double) and size (input / output dim)
	typedef _Scalar Scalar;
	enum { // Required by numerical differentiation module
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};

	// Tell the caller the matrix sizes associated with the input, output, and jacobian
	typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

	// Local copy of the number of inputs
	int m_inputs, m_values;

	// Two constructors:
	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	// Get methods for users to determine function input and output dimensions
	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

};

/***********************************************************************************************/

// Functor for the Gauss function
struct GaussFunctor : Functor<double>
{
	// Constructor
	GaussFunctor(const Eigen::VectorXd& xData, const Eigen::VectorXd& yData)
		: Functor<double>(4, xData.size()), xData(xData), yData(yData) {}

	// Implementation of the objective function
	int operator()(const Eigen::VectorXd &params, Eigen::VectorXd &fvec) const {
		// Create an instance of GaussFunction with current parameters
		GaussFunction gaussFunction(params[0], params[1], params[2], params[3]);
		for (int i = 0; i < xData.size(); ++i) {
			fvec[i] = yData[i] - gaussFunction(xData[i]);
		}

		return 0;
	}

	const Eigen::VectorXd xData;
	const Eigen::VectorXd yData;
};

#endif //OPTIMIZATIONFUNCTOR_H
