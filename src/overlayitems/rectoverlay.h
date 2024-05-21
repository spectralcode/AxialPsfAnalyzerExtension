#ifndef RECTOVERLAY_H
#define RECTOVERLAY_H

#include "overlayitem.h"
#include "anchorpoint.h"
#include <QGraphicsItem>
#include <QPainter>

class RectOverlay : public OverlayItem {
public:
	explicit RectOverlay(QGraphicsItem *parent = nullptr);

	QRectF boundingRect() const override;
	void setRect(QRect rect);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	AnchorPoint *topLeftAnchor;
	AnchorPoint *bottomRightAnchor;

	qreal penWidth;
};

#endif //RECTOVERLAY_H
