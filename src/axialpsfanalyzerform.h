#ifndef AXIALPSFANALYZERFORM_H
#define AXIALPSFANALYZERFORM_H

#include <QWidget>
#include <QRect>
#include "axialpsfanalyzerparameters.h"
#include "lineplot.h"
#include "imagedisplay.h"

namespace Ui {
class AxialPsfAnalyzerForm;
}

class AxialPsfAnalyzerForm : public QWidget
{
	Q_OBJECT

public:
	explicit AxialPsfAnalyzerForm(QWidget *parent = 0);
	~AxialPsfAnalyzerForm();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);

	ImageDisplay* getImageDisplay(){return this->imageDisplay;}
	LinePlot* getLinePlot(){return this->linePlot;}

	Ui::AxialPsfAnalyzerForm* ui;

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;


public slots:
	void setMaximumFrameNr(int maximum);
	void setMaximumBufferNr(int maximum);
	void plotLine(QVector<qreal> line);
	void plotData(QVector<qreal> x, QVector<qreal> y);
	void plotFittedData(QVector<qreal> x, QVector<qreal> y);
	void plotPeakPositionIndicator(double pos);
	void displayPeakPositionValue(double pos);
	void displayFwhmValue(double value);
	void enableAutoScalingLinePlot(bool autoScaleEnabled);

private:
	ImageDisplay* imageDisplay;
	LinePlot* linePlot;
	AxialPsfAnalyzerParameters parameters;
	bool firstRun;

signals:
	void paramsChanged(AxialPsfAnalyzerParameters);
	void frameNrChanged(int);
	void bufferNrChanged(int);
	void featureChanged(int);
	void bufferSourceChanged(BUFFER_SOURCE);
	void roiChanged(QRect);
	void singleFetchRequested();
	void autoFetchRequested(bool isRequested);
	void nthBufferChanged(int nthBuffer);
	void fitModeLogarithmEnabled(bool enabled);
	void info(QString);
	void error(QString);
};

#endif //AXIALPSFANALYZERFORM_H
