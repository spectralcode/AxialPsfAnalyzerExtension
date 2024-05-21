#ifndef ANCHORPOINT_H
#define ANCHORPOINT_H

#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QObject>

class AnchorPoint : public QObject, public QGraphicsEllipseItem
{
	Q_OBJECT
public:
	explicit AnchorPoint(QGraphicsItem *parent = nullptr);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	int type() const override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
	QBrush defaultBrush;
	QBrush hoverBrush;
	QPointF originalPosition;

	bool isUnderMouse() const;

signals:
	void positionChanged(QPointF pos);
};

#endif //ANCHORPOINT_H
