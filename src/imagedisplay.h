#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include "bitdepthconverter.h"
#include "rectoverlay.h"

class ImageDisplay : public QGraphicsView
{
	Q_OBJECT
	QThread converterThread;

public:
	explicit ImageDisplay(QWidget *parent = nullptr);
	~ImageDisplay();

	QRect getRoi(){return this->currentRoi;}

private:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void scaleView(qreal scaleFactor);

private:
	BitDepthConverter* bitConverter;
	QGraphicsScene* scene;
	QGraphicsPixmapItem* inputItem;
	int frameWidth;
	int frameHeight;
	int mousePosX;
	int mousePosY;
	RectOverlay* roiRect;
	QRect currentRoi;

public slots:
	void zoomIn();
	void zoomOut();
	void receiveFrame(void* frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void displayFrame(uchar* frame, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void setRoi(QRect roi);

signals:
	void non8bitFrameReceived(void *frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void roiChanged(QRect);
	void info(QString);
	void error(QString);

};

#endif //IMAGEDISPLAY_H
