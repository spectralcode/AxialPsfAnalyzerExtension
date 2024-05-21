#include "anchorpoint.h"
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "overlayitem.h"


AnchorPoint::AnchorPoint(QGraphicsItem *parent)
	: QObject(nullptr),
	QGraphicsEllipseItem(-45, -45, 90, 90, parent),
	defaultBrush(QColor(0, 0, 255, 128)),
	hoverBrush(QColor(255, 128, 128, 128))
{
	setBrush(defaultBrush);
	setFlags(ItemIsMovable | ItemSendsGeometryChanges);
	setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);
	setFlag(ItemIgnoresParentOpacity);
	setOpacity(0.01);
}

void AnchorPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	painter->setRenderHint(QPainter::Antialiasing, true);
	QGraphicsEllipseItem::paint(painter, option, widget);
}

int AnchorPoint::type() const {
	return Type;
}

void AnchorPoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	setOpacity(1);
	setBrush(hoverBrush);
	setPen(QPen(Qt::NoPen));
	scene()->views().first()->setCursor(Qt::OpenHandCursor);
	QGraphicsEllipseItem::hoverEnterEvent(event);
}

void AnchorPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	setBrush(defaultBrush);
	setPen(QPen(Qt::NoPen));
	QGraphicsEllipseItem::hoverLeaveEvent(event);
	this->setOpacity(0.25);
	QTimer::singleShot(2000, this, [this]() {
		if (!this->isUnderMouse()) {
			this->setOpacity(0.01);
		}
	});
	scene()->views().first()->unsetCursor();
}

void AnchorPoint::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	this->originalPosition = this->pos();
	scene()->views().first()->setCursor(Qt::ClosedHandCursor);
	QGraphicsEllipseItem::mousePressEvent(event);
}

void AnchorPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	scene()->views().first()->setCursor(Qt::OpenHandCursor);
	QGraphicsEllipseItem::mouseReleaseEvent(event);

	if(this->pos() != this->originalPosition){
		OverlayItem* parentOverlay = dynamic_cast<OverlayItem*>(parentItem());
		if (parentOverlay) {
			parentOverlay->onAnchorPointPositionChanged();
		}
	}
}

QVariant AnchorPoint::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (change == ItemPositionChange && this->scene()) {
		if (this->parentItem()) {
			this->parentItem()->update();
		}
	}
	if (change == ItemPositionHasChanged && this->scene()) {
//		OverlayItem* parentOverlay = dynamic_cast<OverlayItem*>(parentItem());
//		if (parentOverlay) {
//			parentOverlay->onAnchorPointPositionChanged();
//		}
	}
	return QGraphicsEllipseItem::itemChange(change, value);
}

bool AnchorPoint::isUnderMouse() const {
	if (!scene() || scene()->views().isEmpty()){
		return false;
	}
	auto view = scene()->views().first();
	QPointF scenePos = view->mapToScene(view->mapFromGlobal(QCursor::pos()));
	if (this->contains(this->mapFromScene(scenePos))) {
		 return true;
	}
	return false;
}
