#include "imagedisplay.h"

ImageDisplay::ImageDisplay(QWidget *parent) : QGraphicsView(parent)
{
	this->scene = new QGraphicsScene(this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	setScene(scene);
	setCacheMode(CacheBackground);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);

	this->inputItem = new QGraphicsPixmapItem();
	this->roiRect = new RectOverlay(inputItem);
	this->scene->addItem(inputItem);
	this->scene->update();

	//setup roi
	connect(this->roiRect, &RectOverlay::positionChanged, this, [this](OverlayItem* item) {
		auto topLeftAnchor = item->getAnchorPoints().at(0);
		auto bottomRightAnchor = item->getAnchorPoints().at(1);
		QRectF roiRect(topLeftAnchor->scenePos(), bottomRightAnchor->scenePos());
		QPolygonF roiInImageCoordinates = this->inputItem->mapFromItem(this->inputItem, roiRect);
		emit roiChanged(roiRect.toRect());
	});

	//adjust orientation of display to match orientation of octproz main output
	this->rotate(90);
	this->scale(1, -1); //flip vertical

	this->frameWidth = 0;
	this->frameHeight = 0;
	this->mousePosX = 0;
	this->mousePosY = 0;

	//setup bitconverter
	this->bitConverter = new BitDepthConverter();
	this->bitConverter->moveToThread(&converterThread);
	connect(this, &ImageDisplay::non8bitFrameReceived, this->bitConverter, &BitDepthConverter::convertDataTo8bit);
	connect(this->bitConverter, &BitDepthConverter::info, this, &ImageDisplay::info);
	connect(this->bitConverter, &BitDepthConverter::error, this, &ImageDisplay::error);
	connect(this->bitConverter, &BitDepthConverter::converted8bitData, this, &ImageDisplay::displayFrame);
	connect(&converterThread, &QThread::finished, this->bitConverter, &BitDepthConverter::deleteLater);
	converterThread.start();
}

ImageDisplay::~ImageDisplay()
{
	converterThread.quit();
	converterThread.wait();
}

void ImageDisplay::mouseDoubleClickEvent(QMouseEvent *event) {
	this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
	this->ensureVisible(this->inputItem);
	this->centerOn(this->pos());
	this->scene->setSceneRect(this->scene->itemsBoundingRect());
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mousePressEvent(event);
}

void ImageDisplay::mouseMoveEvent(QMouseEvent* event) {
	if ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier)) {
		QPointF oldPosition = mapToScene(this->mousePosX, this->mousePosY);
		QPointF newPosition = mapToScene(event->pos());
		QPointF translation = newPosition - oldPosition;
		this->translate(translation.x(), translation.y());
		this->mousePosX = event->x();
		this->mousePosY = event->y();
	}
	QGraphicsView::mouseMoveEvent(event);
}

void ImageDisplay::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Plus:
		this->zoomIn();
		break;
	case Qt::Key_Minus:
		this->zoomOut();
		break;
	default:
		QGraphicsView::keyPressEvent(event);
	}
}

void ImageDisplay::wheelEvent(QWheelEvent* event) {
	//zoom with mouse wheel, only if CTRL key is pressed
	if (event->modifiers() & Qt::ControlModifier) {
		double angle = event->angleDelta().y();
		double factor = qPow(1.0015, angle);
		QPoint targetViewportPos = event->pos();
		QPointF targetScenePos = mapToScene(event->pos());
		this->scale(factor, factor);
		this->centerOn(targetScenePos);
		QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
		QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
		this->centerOn(mapToScene(viewportCenter.toPoint()));
	}
	QGraphicsView::wheelEvent(event);
}

void ImageDisplay::scaleView(qreal scaleFactor) {
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100){
		return;
	}
	this->scale(scaleFactor, scaleFactor);
}

void ImageDisplay::zoomIn() {
	this->scaleView(qreal(1.2));
}

void ImageDisplay::zoomOut() {
	this->scaleView(1/qreal(1.2));
}

void ImageDisplay::receiveFrame(void *frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	if(!this->isVisible()){
		return;
	}
	if(bitDepth != 8){
		emit non8bitFrameReceived(frame, bitDepth, samplesPerLine, linesPerFrame);
	}else{
		this->displayFrame(static_cast<uchar*>(frame), samplesPerLine, linesPerFrame);
	}
}

void ImageDisplay::displayFrame(uchar* frame, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	//create QPixmap from uchar array and update inputItem
	QImage image(frame, samplesPerLine, linesPerFrame, samplesPerLine, QImage::Format_Grayscale8 );
	this->inputItem->setPixmap(QPixmap::fromImage(image));

	//scale view if input sizes have changed
	if(this->frameWidth != samplesPerLine || this->frameHeight != linesPerFrame){
		this->frameWidth = samplesPerLine;
		this->frameHeight = linesPerFrame;
		this->fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);
		this->ensureVisible(this->inputItem);
		this->centerOn(this->pos());

		//set scene rect back to minimal size
		this->scene->setSceneRect(this->scene->itemsBoundingRect());
	}
}

void ImageDisplay::setRoi(QRect roi) {
	this->roiRect->setRect(roi);
}
