#include "overlayitem.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>


OverlayItem::OverlayItem(QGraphicsItem *parent)
	: QObject(nullptr),
	QGraphicsItem(parent)
{
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void OverlayItem::addAnchorPoint(AnchorPoint *anchor) {
	anchorPoints.append(anchor);
}

void OverlayItem::onAnchorPointPositionChanged() {
	emit this->positionChanged(this);
}

QVariantMap OverlayItem::saveState() const {
	QVariantMap state;
	QVariantList anchorsList;

	for (const AnchorPoint* anchor : anchorPoints) {
		QVariantMap anchorData;
		anchorData["x"] = anchor->pos().x();
		anchorData["y"] = anchor->pos().y();
		anchorsList.append(anchorData);
	}

	state["anchors"] = anchorsList;
	state["isVisible"] = this->isVisible();
	state["x_position"] = this->pos().x();
	state["y_position"] = this->pos().y();

	return state;
}

void OverlayItem::loadState(const QVariantMap &state) {
	const QVariantList anchorsList = state["anchors"].toList();

	if(anchorsList.isEmpty()){
		return;
	}

	if(anchorsList.size() != this->anchorPoints.size()){
		qWarning() << "Number of anchor points does not match. Expected:"
					<< this->anchorPoints.size() << ", loaded:" << anchorsList.size();
		return;
	}

	for (int i = 0; i < anchorsList.size(); ++i) {
		const QVariant &data = anchorsList[i];
		QVariantMap anchorData = data.toMap();
		QPointF pos(anchorData["x"].toReal(), anchorData["y"].toReal());
		this->anchorPoints[i]->setPos(pos);
	}

	this->setVisible(state["isVisible"].toBool());
	this->setPos(state["x_position"].toReal(), state["y_position"].toReal());
}

void OverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (!isClickOnAnchorPoint(event->pos())) {
		scene()->views().first()->setCursor(Qt::ClosedHandCursor);
		QGraphicsItem::mousePressEvent(event);
		this->originalPosition = this->pos();
	}
}

void OverlayItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	QGraphicsItem::mouseMoveEvent(event);
}

void OverlayItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	scene()->views().first()->unsetCursor();
	QGraphicsItem::mouseReleaseEvent(event);

	if (this->pos() != originalPosition) {
		emit positionChanged(this);
	}
}

QVariant OverlayItem::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (change == ItemPositionHasChanged && this->scene()) {
		//emit this->positionChanged(this); //this can be used for continuous position updates, but continuously emitting many position changes via signal-slot may slow down the application
	} else if (change == ItemVisibleHasChanged) {
		emit visibilityChanged(this);
	}
	return QGraphicsItem::itemChange(change, value);
}

bool OverlayItem::isClickOnAnchorPoint(const QPointF& clickPos) const {
	foreach (AnchorPoint *anchor, anchorPoints) {
		QRectF anchorRect = anchor->boundingRect().translated(anchor->pos());
		if (anchor->isVisible() && anchorRect.contains(clickPos)) {
			return true;
		}
	}
	return false;
}
