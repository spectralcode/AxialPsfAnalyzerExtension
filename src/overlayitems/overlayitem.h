#ifndef OVERLAYITEM_H
#define OVERLAYITEM_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "anchorpoint.h"

class OverlayItem : public QObject,  public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	explicit OverlayItem(QGraphicsItem *parent = nullptr);

	virtual QRectF boundingRect() const override = 0;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;

	QVariantMap saveState() const;
	void loadState(const QVariantMap& state);

	void addAnchorPoint(AnchorPoint *anchor);
	QList<AnchorPoint *> getAnchorPoints() const { return this->anchorPoints; }

	QString getName() const { return this->name; }
	void setName(const QString &name) { this->name = name; }

	void onAnchorPointPositionChanged();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
	QString name;
	QPointF originalPosition;
	QList<AnchorPoint *> anchorPoints;

	bool isClickOnAnchorPoint(const QPointF& clickPos) const;

signals:
	void positionChanged(OverlayItem* item);
	void visibilityChanged(OverlayItem* item);
};

#endif //OVERLAYITEM_H
