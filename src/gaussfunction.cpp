#include "gaussfunction.h"


GaussFunction::GaussFunction(double a, double k, double m, double s)
: a(a), k(k), m(m), s(s)
{

}

// Compute the Gaussian value at x
double GaussFunction::operator()(double x) const {
	return this->k + (this->a - this->k) * qExp(-((x - this->m) * (x - this->m)) / (2.0 * this->s * this->s));
}

double GaussFunction::getA() const {
	return this->a;
}

double GaussFunction::getK() const {
	return this->k;
}

double GaussFunction::getM() const {
	return this->m;
}

double GaussFunction::getS() const {
	return this->s;
}

void GaussFunction::setA(double newA) {
	this->a = newA;
}

void GaussFunction::setK(double newK) {
	this->k = newK;
}

void GaussFunction::setM(double newM) {
	this->m = newM;
}

void GaussFunction::setS(double newS) {
	this->s = newS;
}

double GaussFunction::getFWHM() {
	return qAbs(2.354820045 * this->s); //info: 2*sqrt(2*ln(2)) ==  2.354820045
}
