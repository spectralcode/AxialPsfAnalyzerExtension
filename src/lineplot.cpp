#include "lineplot.h"
#include <QPainterPathStroker>

LinePlot::LinePlot(QWidget *parent) : QCustomPlot(parent){
	//default colors
	this->referenceCurveAlpha = 255;
	this->setBackground( QColor(50, 50, 50));
	this->axisRect()->setBackground(QColor(25, 25, 25));
	this->curveColor.setRgb(55, 100, 250);
	this->referenceCurveColor.setRgb(250, 50, 50, referenceCurveAlpha);

	//default appearance of legend
	this->legend->setBrush(QBrush(QColor(0,0,0,100))); //semi transparent background
	this->legend->setTextColor(QColor(200,200,200,255));
	QFont legendFont = font();
	legendFont.setPointSize(8);
	this->legend->setFont(legendFont);
	this->legend->setSelectedFont(legendFont);
	this->legend->setBorderPen(QPen(QColor(0,0,0,0))); //set legend border invisible
	this->legend->setColumnSpacing(0);
	this->legend->setRowSpacing(0);
	this->axisRect()->insetLayout()->setAutoMargins(QCP::msNone);
	this->axisRect()->insetLayout()->setMargins(QMargins(0,0,0,0));

	this->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop);

	//configure curve graph
	this->addGraph();
	this->setCurveColor(curveColor);

	//configure reference curve graph
	this->addGraph();
	this->setReferenceCurveColor(referenceCurveColor);

	//configure axis
	this->setAxisVisible(true);
	this->setAxisColor(Qt::white);
	this->setGridColor(Qt::darkGray);
	this->xAxis->setLabel("Position in px");
	this->yAxis->setLabel("Intensity in a.u.");

	//user interactions
	this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

	//init axis select and zoom
	connect(this, &QCustomPlot::selectionChangedByUser, this, &LinePlot::combineSelections);
	connect(this, &QCustomPlot::mouseWheel, this, &LinePlot::zoomSelectedAxisWithMouseWheel);
	connect(this, &QCustomPlot::mousePress, this, &LinePlot::dragSelectedAxes);

	//maximize size of plot area by hiding axis labels
//	this->axisRect()->setAutoMargins(QCP::msNone);
//	this->axisRect()->setMargins(QMargins(0,0,0,0));

	//initalize vertical lines which chan be used to indicate a selected x range in the plot
	this->lineA = new QCPItemStraightLine(this); //from the documentation: The created item is automatically registered with parentPlot. This QCustomPlot instance takes ownership of the item, so do not delete it manually but use QCustomPlot::removeItem() instead.
	this->lineB = new QCPItemStraightLine(this);
	QPen linePen = QPen(QColor(30,0,0));
	QPen linePenB = QPen(QColor(Qt::GlobalColor::lightGray));
	linePen.setWidth(1);
	this->lineA->setPen(linePen);
	this->lineB->setPen(linePenB);
	this->lineA->setVisible(false);
	this->lineB->setVisible(false);
	this->setVerticalLine(0);
	this->setHorizontalLine(0);

	//round corners
	this->roundCorners(false);

	//init scaling
	this->customRange = false;

	//init bools that are used to check if curve and referenceCurved have been used
	this->curveUsed =false;
	this->referenceCurveUsed = false;

	this->dataPointCounter = 0;

	this->autoScaleEnabled = true;
}

LinePlot::~LinePlot() {
}

void LinePlot::setCurveColor(QColor color) {
	this->curveColor = color;
	QPen curvePen = QPen(color);
	curvePen.setWidth(1);
	this->graph(0)->setPen(curvePen);
}

void LinePlot::setReferenceCurveColor(QColor color) {
	this->referenceCurveColor = color;
	QPen referenceCurvePen = QPen(color);
	referenceCurvePen.setWidth(1);
	this->graph(1)->setPen(referenceCurvePen);
}

void LinePlot::setCurveName(QString name) {
	this->graph(0)->setName(name);
	this->replot();
}

void LinePlot::setReferenceCurveName(QString name) {
	this->graph(1)->setName(name);
	this->replot();
}

void LinePlot::setLegendVisible(bool visible) {
	this->legend->setVisible(visible);
	this->replot();
}

void LinePlot::setAxisVisible(bool visible) {
	this->yAxis->setVisible(visible);
	this->xAxis->setVisible(visible);
}

void LinePlot::plotLine(QVector<qreal> line) {
	// Check if the line data is empty
	if (line.isEmpty()) {
		emit error(tr("Could not plot data. Data seems to be empty."));
		return;
	}

	//set x coordinates as sequential numbers
	QVector<double> x(line.size());
	for (int i = 0; i < line.size(); ++i) {
		x[i] = i;
	}

	//set data for the graph
	this->graph(0)->setData(x, line, true);

	//update plot
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	this->replot();
}

void LinePlot::plotCurve(QVector<qreal> x, QVector<qreal> y) {
	if(x.size() <= 0 || x.size() <= 0){
		emit error(tr("Could not plot data. Data seems to be missing."));
		return;
	}
	this->sampleNumbers = x;
	this->curve = y; //todo: have a look at save function and use values directly from plot instead of having a QVector curve as member
	this->graph(0)->setData(x, y, true);
	//update plot
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	this->replot();
}

void LinePlot::plotReferenceCurve(QVector<qreal> x, QVector<qreal> y) {
	if(x.size() <= 0 || x.size() <= 0){
		emit error(tr("Could not plot data. Data seems to be missing."));
		return;
	}
	this->referenceCurve = y;
	this->graph(1)->setData(x, y, true);
	//update plot
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	this->replot();
}

void LinePlot::plotCurves(double *curve, double *referenceCurve, unsigned int samples) {
	if(samples == 0){return;}
	int size = static_cast<int>(samples);

	 //set values for x-axis
	 if(this->sampleNumbers.size() != size){
		 this->sampleNumbers.resize(size);
		 for(int i = 0; i<size; i++){
			 this->sampleNumbers[i] = i;
		 }
	 }

	 //fill curve data
	 if(curve != nullptr){
		 this->curveUsed = true;
		 if(this->curve.size() != size){
			 this->curve.resize(size);
		 }
		 for(int i = 0; i<size; i++){
			 this->curve[i] = static_cast<double>(curve[i]);
		 }
		 this->graph(0)->setData(this->sampleNumbers, this->curve, true);
	 }

	 //fill reference curve data
	 if(referenceCurve != nullptr){
		 this->referenceCurveUsed = true;
		 if(this->referenceCurve.size() != size){
			 this->referenceCurve.resize(size);
		 }
		 for(int i = 0; i<size; i++){
			 this->referenceCurve[i] = static_cast<double>(referenceCurve[i]);
		 }
		 this->graph(1)->setData(this->sampleNumbers, this->referenceCurve, true);
	 }

	 //update plot
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	 this->replot();
}

void LinePlot::plotCurves(float *curve, float *referenceCurve, unsigned int samples) {
	if(samples == 0){return;}
	int size = static_cast<int>(samples);

	 //set values for x-axis
	 if(this->sampleNumbers.size() != size){
		 this->sampleNumbers.resize(size);
		 for(int i = 0; i<size; i++){
			 this->sampleNumbers[i] = i;
		 }
	 }

	 //fill curve data
	 if(curve != nullptr){
		 this->curveUsed = true;
		 if(this->curve.size() != size){
			 this->curve.resize(size);
		 }
		 for(int i = 0; i<size; i++){
			 this->curve[i] = static_cast<double>(curve[i]);
		 }
		 this->graph(0)->setData(this->sampleNumbers, this->curve, true);
	 }

	 //fill reference curve data
	 if(referenceCurve != nullptr){
		 this->referenceCurveUsed = true;
		 if(this->referenceCurve.size() != size){
			 this->referenceCurve.resize(size);
		 }
		 for(int i = 0; i<size; i++){
			 this->referenceCurve[i] = static_cast<double>(referenceCurve[i]);
		 }
		 this->graph(1)->setData(this->sampleNumbers, this->referenceCurve, true);
	 }

	 //update plot
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	 this->replot();
}

void LinePlot::addDataToCurves(double curveDataPoint, double referenceDataPoint){
	this->dataPointCounter++;
	if(this->dataPointCounter > 10000000){
		this->graph(0)->data()->clear();
		this->graph(1)->data()->clear();
		this->replot();
		this->dataPointCounter = 0;
	}

	this->graph(0)->addData(dataPointCounter, curveDataPoint);
	this->graph(1)->addData(dataPointCounter, referenceDataPoint);

	//auto scroll plot:
	this->xAxis->setRange(dataPointCounter, 512, Qt::AlignRight);
	if(this->autoScaleEnabled){
		this->rescaleAxes();
		this->zoomOutSlightly();
	}
	this->replot();
}

void LinePlot::clearPlot() {
	float dummyValue = 0.0;
	if(this->curveUsed){
		this->plotCurves(&dummyValue, nullptr, 1);
	}
	if(this->referenceCurveUsed){
		this->plotCurves(nullptr, &dummyValue, 1);
	}
}

void LinePlot::setAxisColor(QColor color) {
	this->xAxis->setBasePen(QPen(color, 1));
	this->yAxis->setBasePen(QPen(color, 1));
	this->xAxis->setTickPen(QPen(color, 1));
	this->yAxis->setTickPen(QPen(color, 1));
	this->xAxis->setSubTickPen(QPen(color, 1));
	this->yAxis->setSubTickPen(QPen(color, 1));
	this->xAxis->setTickLabelColor(color);
	this->yAxis->setTickLabelColor(color);
	this->xAxis->setLabelColor(color);
	this->yAxis->setLabelColor(color);
}

void LinePlot::setGridColor(QColor color){
	this->xAxis->grid()->setPen(QPen(color, 1, Qt::DashLine));
	this->yAxis->grid()->setPen(QPen(color, 1, Qt::DashLine));
//	this->xAxis->grid()->setSubGridPen(QPen(color, 1, Qt::DotLine));
//	this->yAxis->grid()->setSubGridPen(QPen(color, 1, Qt::DotLine));
//	this->xAxis->grid()->setSubGridVisible(true);
//	this->yAxis->grid()->setSubGridVisible(true);
	this->xAxis->grid()->setZeroLinePen(Qt::NoPen);
	this->yAxis->grid()->setZeroLinePen(Qt::NoPen);
}

void LinePlot::zoomOutSlightly() {
	this->yAxis->scaleRange(1.1, this->yAxis->range().center());
	this->xAxis->scaleRange(1.1, this->xAxis->range().center());
}

void LinePlot::combineSelections() {
	// axis label, axis and tick labels should act as a s single selectable item
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel) || this->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
		this->xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels | QCPAxis::spAxisLabel);
	}
	if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxisLabel) || this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
		this->xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels | QCPAxis::spAxisLabel);
	}
	if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel) || this->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)) {
		this->yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels | QCPAxis::spAxisLabel);
	}
	if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxisLabel) || this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || this->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)) {
		this->yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels | QCPAxis::spAxisLabel);
	}
}

void LinePlot::zoomSelectedAxisWithMouseWheel() {
	QList<QCPAxis*> selectedAxes;
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->xAxis);
	}
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->yAxis);
	}
	else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->xAxis2);
	}
	else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->yAxis2);
	}
	else {
		//no axis is selected --> enable zooming for all axes
		selectedAxes.append(this->xAxis);
		selectedAxes.append(this->yAxis);
		selectedAxes.append(this->xAxis2);
		selectedAxes.append(this->yAxis2);
	}

	this->axisRect()->setRangeZoomAxes(selectedAxes);
}

void LinePlot::dragSelectedAxes() {
	QList<QCPAxis*> selectedAxes;
	if (this->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->xAxis);
	}
	else if (this->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->yAxis);
	}
	else if (this->xAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->xAxis2);
	}
	else if (this->yAxis2->selectedParts().testFlag(QCPAxis::spAxis)) {
		selectedAxes.append(this->yAxis2);
	}
	else {
		//no axis is selected --> enable dragging for all axes
		selectedAxes.append(this->xAxis);
		selectedAxes.append(this->yAxis);
		selectedAxes.append(this->xAxis2);
		selectedAxes.append(this->yAxis2);
	}

	this->axisRect()->setRangeDragAxes(selectedAxes);
}


void LinePlot::contextMenuEvent(QContextMenuEvent *event) {
	QMenu menu(this);
	QAction savePlotAction(tr("Save Plot as..."), this);
	connect(&savePlotAction, &QAction::triggered, this, &LinePlot::slot_saveToDisk);
	menu.addAction(&savePlotAction);
	menu.exec(event->globalPos());
}

void LinePlot::mouseMoveEvent(QMouseEvent *event) {
	if(!(event->buttons() & Qt::LeftButton)){
		double x = this->xAxis->pixelToCoord(event->pos().x());
		double y = this->yAxis->pixelToCoord(event->pos().y());
		this->setToolTip(QString("%1 , %2").arg(x).arg(y));
	}else{
		QCustomPlot::mouseMoveEvent(event);
	}
}

void LinePlot::resizeEvent(QResizeEvent *event) {
	if(this->drawRoundCorners){
		QRect plotRect = this->rect();
		const int radius = 6;
		QPainterPath path;
		path.addRoundedRect(plotRect, radius, radius);
		QRegion mask = QRegion(path.toFillPolygon().toPolygon());
		this->setMask(mask);
	}
	QCustomPlot::resizeEvent(event);
}

void LinePlot::changeEvent(QEvent *event) {
	if(event->ActivationChange){
		if(!this->isEnabled()){
			this->curveColor.setAlpha(55);
			this->referenceCurveColor.setAlpha(25);
			this->setCurveColor(this->curveColor);
			this->setReferenceCurveColor(this->referenceCurveColor);
			this->replot();
		} else {
			this->curveColor.setAlpha(255);
			this->referenceCurveColor.setAlpha(this->referenceCurveAlpha);
			this->setCurveColor(this->curveColor);
			this->setReferenceCurveColor(this->referenceCurveColor);
			this->replot();
		}
	}
	QCustomPlot::changeEvent(event);

}

void LinePlot::mouseDoubleClickEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		this->rescaleAxes();
		this->zoomOutSlightly();
		if(customRange){
			this->scaleYAxis(this->customRangeLower, this->customRangeUpper);
		}
		this->replot();
	}
}

void LinePlot::slot_saveToDisk() {
	QString filters("Image (*.png);;Vector graphic (*.pdf);;CSV (*.csv)");
	QString defaultFilter("CSV (*.csv)");
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot"), QDir::currentPath(), filters, &defaultFilter);
	if(fileName == ""){
		emit error(tr("Save plot to disk canceled."));
		return;
	}
	bool saved = false;
	if(defaultFilter == "Image (*.png)"){
		saved = this->savePng(fileName);
	}else if(defaultFilter == "Vector graphic (*.pdf)"){
		saved = this->savePdf(fileName);
	}else if(defaultFilter == "CSV (*.csv)"){
		saved = this->saveAllCurvesToFile(fileName);
	}
	if(saved){
		emit info(tr("Plot saved to ") + fileName);
	}else{
		emit error(tr("Could not save plot to disk."));
	}
}

void LinePlot::setVerticalLineVisible(bool visible) {
	this->lineA->setVisible(visible);
}

void LinePlot::setHorizontalLineVisible(bool visible) {
	this->lineB->setVisible(visible);
}

void LinePlot::scaleYAxis(double min, double max) {
	this->customRange = true;
	this->customRangeLower = min;
	this->customRangeUpper = max;
	this->yAxis->setRange(min, max);
	this->yAxis2->setRange(min, max);
	this->zoomOutSlightly();
	this->replot();
}

bool LinePlot::saveCurveDataToFile(QString fileName) {
	bool saved = false;
	QFile file(fileName);
	if (file.open(QFile::WriteOnly|QFile::Truncate)) {
		QTextStream stream(&file);
		stream << "Sample Number" << ";" << "Sample Value" << "\n";
		for(int i = 0; i < this->sampleNumbers.size(); i++){
			stream << QString::number(this->sampleNumbers.at(i)) << ";" << QString::number(this->curve.at(i)) << "\n";
		}
	file.close();
	saved = true;
	}
	return saved;
}

bool LinePlot::saveAllCurvesToFile( QString fileName) {
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
		return false;
	}

	QTextStream stream(&file);
	
	//check how many graphs are in the plot
	int numGraphs = this->graphCount();
	if (numGraphs == 0) {
		file.close(); 
		return false; 
	}

	//write header with column names
	stream << "Key";
	for (int i = 0; i < numGraphs; i++) {
		if (this->graph(i)) {
			stream << ";" << this->graph(i)->name();
		}
	}
	stream << "\n";

	//use maps for storing key-value pairs for each graph
	QVector<QMap<double, double>> allData(numGraphs);
	for (int i = 0; i < numGraphs; i++) {
		if (this->graph(i)) {
			for (int j = 0; j < this->graph(i)->data()->size(); j++) {
				allData[i][this->graph(i)->data()->at(j)->key] = this->graph(i)->data()->at(j)->value;
			}
		}
	}

	//combine all keys from all graphs
	QSet<double> allKeys;
	for (const auto &dataMap : allData) {
		allKeys.unite(QSet<double>::fromList(dataMap.keys()));
	}
	QList<double> sortedKeys = QList<double>::fromSet(allKeys);
	std::sort(sortedKeys.begin(), sortedKeys.end());

	//write keys (x coordinate) and values (y coordinate)
	for (double key : sortedKeys) {
		stream << QString::number(key);
		for (const auto &dataMap : allData) {
			double value = dataMap.value(key, std::numeric_limits<double>::quiet_NaN());
			QString valueString = std::isnan(value) ? "" : QString::number(value);
			stream << ";" << valueString;
		}
		stream << "\n";
	}

	file.close();
	return true;
}


void LinePlot::enableAutoScaling(bool autoScaleEnabled) {
	this->autoScaleEnabled = autoScaleEnabled;
}

void LinePlot::setVerticalLine(double xPos) {
	this->lineA->point1->setCoords(xPos, 0);
	this->lineA->point2->setCoords(xPos, 1);
	this->replot();
}

void LinePlot::setHorizontalLine(double yPos) {
	this->lineB->point1->setCoords(0, yPos);
	this->lineB->point2->setCoords(1, yPos);
	this->replot();
}

