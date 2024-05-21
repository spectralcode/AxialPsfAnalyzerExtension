#ifndef PEAKFIT_H
#define PEAKFIT_H

#include <QObject>
#include <QVector>
#include <QRect>
#include <QApplication>
#include <QtMath>
#include <QPair>
#include "axialpsfanalyzerparameters.h"


class PeakFit : public QObject
{
	Q_OBJECT
public:
	explicit PeakFit(QObject *parent = nullptr);

private:
	bool isPeakFitting;
	AxialPsfAnalyzerParameters params;

	int findMaxValuePosition(const QVector<qreal>& line);
	QRect clampRoi(QRect roi, unsigned int samplesPerLine, unsigned int linesPerFrame);
	QVector<qreal> calculateAveragedLine(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	template <typename T> QVector<qreal> calculateAveragedLine(QRect roi, T* frame, unsigned int samplesPerLine, unsigned int linesPerFrame);
	QPair<QVector<qreal>, QVector<qreal>> clampLine(QVector<qreal> line, int startIndex, int endIndex);

signals:
	void averagedLineCalculated(QVector<qreal> x, QVector<qreal> y);
	void fitCalculated(QVector<qreal> x, QVector<qreal> y);
	void peakPositionFound(double pos);
	void fwhmCalculated(double fwhm);
	void info(QString);
	void error(QString);

public slots:
	void fitPeak(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void setRoi(QRect roi);
	void setParams(AxialPsfAnalyzerParameters params);
};

#endif //PEAKFIT
