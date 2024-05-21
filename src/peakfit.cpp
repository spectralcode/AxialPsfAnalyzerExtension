#include "peakfit.h"
#include <QtMath>
#include "gaussfit.h"

PeakFit::PeakFit(QObject *parent)
	: QObject(parent),
	isPeakFitting(false)
{

}

void PeakFit::setParams(AxialPsfAnalyzerParameters params) {
	this->params = params;
}

void PeakFit::fitPeak(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	if (!this->isPeakFitting) {
		this->isPeakFitting = true;

		//average all A-scans within roi
		QVector<qreal> averagedLine = this->calculateAveragedLine(frameBuffer, bitDepth, samplesPerLine, linesPerFrame);

		//clamp averaged line to only use values within roi for fit
		int startIndex = this->params.roi.normalized().x();
		int endIndex = startIndex + this->params.roi.normalized().width()-1;
		QPair<QVector<qreal>, QVector<qreal>> result = this->clampLine(averagedLine, startIndex, endIndex);
		QVector<qreal> xValuesAveragedLine = result.first;
		QVector<qreal> yValuesAveragedLine = result.second;
		int samplesInClampedLine = xValuesAveragedLine.size();
		emit averagedLineCalculated(xValuesAveragedLine, yValuesAveragedLine);

		//convert QVector to Eigen::VectorXd for GaussFit
		Eigen::VectorXd xData(samplesInClampedLine);
		Eigen::VectorXd yData(samplesInClampedLine);

		for (int i = 0; i < samplesInClampedLine; i++) {
			xData[i] = xValuesAveragedLine[i]; // x values are the indices
			yData[i] = yValuesAveragedLine[i]; // y values are the data points
		}

		//perform Gauss fit on the data
		GaussFit gaussFit(xData, yData);
		int maxPos = this->findMaxValuePosition(averagedLine);
		double maxValue = averagedLine.at(qAbs(maxPos));
		gaussFit.setInitialGuessForM(maxPos);
		gaussFit.setInitialGuessForA(maxValue);
		gaussFit.fit();

		//get the fitted Gaussian function
		GaussFunction fittedGauss = gaussFit.getGaussianFunction();

		//generate fitted curve data for plot
		QVector<qreal> fitX;
		QVector<qreal> fitY;
		int fitLength = samplesInClampedLine * 10;
		fitX.resize(fitLength);
		fitY.resize(fitLength);
		double step = static_cast<double>(samplesInClampedLine)/static_cast<double>(fitLength);
		for (int i = 0; i < fitLength; i++) {
			fitX[i] = xValuesAveragedLine.at(0)+ step*i;
			fitY[i] = fittedGauss(fitX.at(i));
		}
		emit fitCalculated(fitX, fitY);

		//calculate and emit fwhm
		double fwhm = fittedGauss.getFWHM();
		emit fwhmCalculated(fwhm);

		//emit peak position
		double peakPosition = fittedGauss.getM();
		emit peakPositionFound(peakPosition);

		this->isPeakFitting = false;
	}
}

void PeakFit::setRoi(QRect roi) {
	this->params.roi = roi;
}

int PeakFit::findMaxValuePosition(const QVector<qreal>& line) {
	if (line.isEmpty()) {
		return -1;
	}

	int maxPos = -1;
	qreal max = line[0];

	for (int i = 1; i < line.size(); i++) {
		if (line[i] > max) {
			max = line[i];
			maxPos = i;
		}
	}

	return maxPos;
}

QRect PeakFit::clampRoi(QRect roi, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QRect clampedRoi(0, 0, 0, 0);
	QRect normalizedRoi = roi.normalized();

	int roiX = normalizedRoi.x();
	int roiWidth = normalizedRoi.width();
	int roiY = normalizedRoi.y();
	int roiHeight = normalizedRoi.height();

	int frameWidth = static_cast<int>(samplesPerLine);
	int frameHeight = static_cast<int>(linesPerFrame);

	//check if roi is fully outside of frame and return zero sized roi
	if(roiX >= frameWidth || roiY >= frameHeight || (roiX + roiWidth) < 0 || (roiY + roiHeight) < 0){
		return clampedRoi;
	}

	//clamp roi to ensure it is fully within the frame
	int endX = (qMin(roiX + roiWidth, frameWidth));
	int endY = (qMin(roiY + roiHeight, frameHeight));
	int clampedX = qMax(0, roiX);
	int clampedY = qMax(0, roiY);
	int clampedWidth = qMin(endX-clampedX, frameWidth);
	int clampedHeight = qMin(endY-clampedY, frameHeight);
	clampedRoi.setX(clampedX);
	clampedRoi.setY(clampedY);
	clampedRoi.setWidth(clampedWidth);
	clampedRoi.setHeight(clampedHeight);

	return clampedRoi;
}

QVector<qreal> PeakFit::calculateAveragedLine(void *frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QVector<qreal> averagedLine;

	if (bitDepth <= 8) {
		unsigned char* frame = static_cast<unsigned char*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned char>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	} else if (bitDepth > 8 && bitDepth <= 16) {
		unsigned short* frame = static_cast<unsigned short*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned short>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	} else if (bitDepth > 16 && bitDepth <= 32) {
		unsigned long* frame = static_cast<unsigned long*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned long>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	}

	return averagedLine;
}

QPair<QVector<qreal>, QVector<qreal> > PeakFit::clampLine(QVector<qreal> line, int startIndex, int endIndex) {
	QVector<qreal> clampedXValues;
	QVector<qreal> clampedYValues;

	//ensure the indices are within bounds
	startIndex = qBound(0, startIndex, line.size() - 1);
	endIndex = qBound(0, endIndex, line.size() - 1);

	//swap if startIndex is greater than endIndex
	if (startIndex > endIndex) {
		qSwap(startIndex, endIndex);
	}

	for (int i = startIndex; i <= endIndex; i++) {
		clampedXValues.append(i);
		clampedYValues.append(line[i]);
	}

	return QPair<QVector<qreal>, QVector<qreal>>(clampedXValues, clampedYValues);
}

template<typename T>
QVector<qreal> PeakFit::calculateAveragedLine(QRect roi, T* frame, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QVector<qreal> averagedLine(samplesPerLine, 0);
	QRect clampedRoi = this->clampRoi(roi, samplesPerLine, linesPerFrame);
	int roiY = clampedRoi.y();
	int roiHeight = clampedRoi.height();
	int roiX = clampedRoi.x();
	int roiWidth = clampedRoi.width();

	//if roi is out of the frame clampedRoi(..) will return a QRect with 0 width and 0 height
	if (roiWidth <= 0 || roiHeight <= 0) {
		return averagedLine;
	}

	//loop through ROI and sum up the values
	int endY = roiY + roiHeight;
	int endX = roiX + roiWidth;
	QVector<qreal> sumLine(roiWidth, 0);

	for (int y = roiY; y < endY; y++) {
		for (int x = roiX; x < endX; x++) {
			sumLine[x - roiX] += frame[y * samplesPerLine + x];
		}
	}

	// compute average per column in ROI
	for (int i = 0; i < roiWidth; i++) {
		averagedLine[roiX + i] = sumLine[i] / roiHeight;
	}

	return averagedLine;
}
