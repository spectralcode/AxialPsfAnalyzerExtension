#ifndef LINEPLOT_H
#define LINEPLOT_H

#include "qcustomplot.h"

class LinePlot : public QCustomPlot
{
	Q_OBJECT
public:
	explicit LinePlot(QWidget *parent = nullptr);
	~LinePlot();

	void setCurveColor(QColor color);
	void setReferenceCurveColor(QColor color);
	void setCurveName(QString name);
	void setReferenceCurveName(QString name);
	void setLegendVisible(bool visible);
	void setAxisVisible(bool visible);
	void plotCurve(QVector<qreal> x, QVector<qreal> y);
	void plotReferenceCurve(QVector<qreal> x, QVector<qreal> y);
	void plotCurves(double* curve, double* referenceCurve, unsigned int samples); //todo: maybe use one template function instead instead one function for double* and one for float*
	void plotCurves(float* curve, float* referenceCurve, unsigned int samples);
	void addDataToCurves(double curveDataPoint, double referenceDataPoint);
	void roundCorners(bool enable){this->drawRoundCorners = enable;}
	void clearPlot();


private:
	void setAxisColor(QColor color);
	void setGridColor(QColor color);
	void zoomOutSlightly();
	void combineSelections();
	void zoomSelectedAxisWithMouseWheel();
	void dragSelectedAxes();


	QVector<qreal> sampleNumbers;
	QVector<qreal> curve;
	QVector<qreal> referenceCurve;
	bool drawRoundCorners;
	QColor curveColor;
	QColor referenceCurveColor;
	int referenceCurveAlpha;
	QCPItemStraightLine* lineA;
	QCPItemStraightLine* lineB;
	double customRangeLower;
	double customRangeUpper;
	bool customRange;
	bool curveUsed;
	bool referenceCurveUsed;
	int dataPointCounter;
	bool autoScaleEnabled;


protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void changeEvent(QEvent* event) override;

signals:
	void info(QString info);
	void error(QString error);


public slots:
	void plotLine(QVector<qreal> line);
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	void slot_saveToDisk();
	void enableAutoScaling(bool autoScaleEnabled);
	void setVerticalLine(double xPos);
	void setHorizontalLine(double yPos);
	void setVerticalLineVisible(bool visible);
	void setHorizontalLineVisible(bool visible);
	void scaleYAxis(double min, double max);
	bool saveCurveDataToFile(QString fileName);
	bool saveAllCurvesToFile(QString fileName);

};


#endif // LINEPLOT_H
