#include "axialpsfanalyzerform.h"
#include "ui_axialpsfanalyzerform.h"
#include <QtGlobal>

AxialPsfAnalyzerForm::AxialPsfAnalyzerForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AxialPsfAnalyzerForm) {
	ui->setupUi(this);

	this->firstRun = true;

	this->imageDisplay = this->ui->widget_imageDisplay;
	connect(this->imageDisplay, &ImageDisplay::info, this, &AxialPsfAnalyzerForm::info);
	connect(this->imageDisplay, &ImageDisplay::error, this, &AxialPsfAnalyzerForm::error);
	connect(this->imageDisplay, QOverload<QRect>::of(&ImageDisplay::roiChanged), this, [this](QRect roiRect) {
		this->parameters.roi = roiRect;
		emit roiChanged(roiRect);
		emit paramsChanged(this->parameters);
	});

	this->linePlot = this->ui->widget_linePlot;
	this->linePlot->setCurveName("Original");
	this->linePlot->setReferenceCurveName("Fit");
	this->linePlot->setLegendVisible(true);
	connect(this->linePlot, &LinePlot::info, this, &AxialPsfAnalyzerForm::info);
	connect(this->linePlot, &LinePlot::error, this, &AxialPsfAnalyzerForm::error);


	//fetch push button and checkbox
	connect(this->ui->pushButton_fetch, &QPushButton::clicked, this, &AxialPsfAnalyzerForm::singleFetchRequested); 
	connect(this->ui->checkBox_autoFetch, &QCheckBox::stateChanged, this, [this](int state){
		bool autoFetch = false;
		if(state == Qt::Checked){
			autoFetch = true;
		} 
		this->ui->pushButton_fetch->setDisabled(autoFetch);
		this->ui->spinBox_nthBuffer->setDisabled(!autoFetch);
		emit this->autoFetchRequested(autoFetch);
		this->parameters.autoFetchingEnabled = autoFetch;
		emit paramsChanged(this->parameters);
	});
	
	//nth buffer
	connect(this->ui->spinBox_nthBuffer, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int nthBuffer) {
		this->parameters.nthBuffer = nthBuffer;
		emit nthBufferChanged(nthBuffer);
		emit paramsChanged(this->parameters);
	});
	
	//SpinBox buffer
	this->ui->spinBox_buffer->setMaximum(2);
	this->ui->spinBox_buffer->setMinimum(-1);
	this->ui->spinBox_buffer->setSpecialValueText(tr("All"));
	connect(this->ui->spinBox_buffer, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int bufferNr) {
		this->parameters.bufferNr = bufferNr;
		emit bufferNrChanged(bufferNr);
		emit paramsChanged(this->parameters);
	});

	//frame slider and spinBox
	connect(this->ui->horizontalSlider_frame, &QSlider::valueChanged, this->ui->spinBox_frame, &QSpinBox::setValue);
	connect(this->ui->spinBox_frame, QOverload<int>::of(&QSpinBox::valueChanged), this->ui->horizontalSlider_frame, &QSlider::setValue);
	connect(this->ui->horizontalSlider_frame, &QSlider::valueChanged, this, [this](int frameNr) {
		this->parameters.frameNr = frameNr;
		emit frameNrChanged(frameNr);
		emit paramsChanged(this->parameters);
	});
	this->setMaximumFrameNr(512);	

	//fit mode
	connect(this->ui->radioButton_linearFitMode, &QRadioButton::toggled, this, [this](bool enabled){
		bool logartihmMode = !enabled;
		this->parameters.fitModeLogarithmEnabled = logartihmMode;
		emit fitModeLogarithmEnabled(logartihmMode);
		emit paramsChanged(this->parameters);
	});

	//autoscaling
	connect(this->ui->checkBox_autoscaling, &QCheckBox::stateChanged, this, [this](int state) {
		if (state == Qt::Checked) {
			this->parameters.autoScalingEnabled = true;
		} else {
			this->parameters.autoScalingEnabled = false;
		}
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		emit paramsChanged(this->parameters);
	});

	//splitter state
	connect(this->ui->splitter, &QSplitter::splitterMoved, this, [this](){
		this->parameters.splitterState = this->ui->splitter->saveState();
		emit paramsChanged(this->parameters);
	});

	//window size and position changes are intercepted by event filter
	this->installEventFilter(this);


	//default values
	this->parameters.bufferSource = PROCESSED;
	this->parameters.roi = QRect(50,50, 400, 800);
	this->parameters.frameNr = 0;
	this->parameters.bufferNr= -1;
	this->parameters.nthBuffer = 10;
	this->parameters.autoScalingEnabled = true;
	this->parameters.autoFetchingEnabled = true;
	this->parameters.fitModeLogarithmEnabled = false;
}

AxialPsfAnalyzerForm::~AxialPsfAnalyzerForm() {
	delete ui;
}

void AxialPsfAnalyzerForm::setSettings(QVariantMap settings) {
	//update parameters struct
	if (!settings.isEmpty()) {
		this->parameters.bufferNr = settings.value(AXIALPSF_BUFFER).toInt();
		this->parameters.bufferSource = static_cast<BUFFER_SOURCE>(settings.value(AXIALPSF_SOURCE).toInt());
		this->parameters.frameNr = settings.value(AXIALPSF_FRAME).toInt();
		this->parameters.nthBuffer = settings.value(AXIALPSF_NTH_BUFFER).toInt();
		int roiX = settings.value(AXIALPSF_ROI_X).toInt();
		int roiY = settings.value(AXIALPSF_ROI_Y).toInt();
		int roiWidth = settings.value(AXIALPSF_ROI_WIDTH).toInt();
		int roiHeight = settings.value(AXIALPSF_ROI_HEIGHT).toInt();
		this->parameters.roi = QRect(roiX, roiY, roiWidth, roiHeight);
		this->parameters.autoScalingEnabled = settings.value(AXIALPSF_AUTOSCALING_ENABLED).toBool();
		this->parameters.autoFetchingEnabled = settings.value(AXIALPSF_AUTOFETCHING_ENABLED).toBool();
		this->parameters.fitModeLogarithmEnabled = settings.value(AXIALPSF_LOG_FIT_ENABLED).toBool();
		this->parameters.splitterState = settings.value(AXIALPSF_SPLITTER_STATE).toByteArray();
		this->parameters.windowState = settings.value(AXIALPSF_WINDOW_STATE).toByteArray();
	}

	//update GUI elements
	this->ui->spinBox_buffer->setValue(this->parameters.bufferNr);
	this->ui->spinBox_nthBuffer->setValue(this->parameters.nthBuffer);
	this->ui->checkBox_autoFetch->setChecked(this->parameters.autoFetchingEnabled);
	this->ui->horizontalSlider_frame->setValue(this->parameters.frameNr);
	this->ui->widget_imageDisplay->setRoi(this->parameters.roi);
	this->ui->checkBox_autoscaling->setChecked(this->parameters.autoScalingEnabled);
	this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
	this->ui->radioButton_linearFitMode->setChecked(!this->parameters.fitModeLogarithmEnabled);
	this->ui->splitter->restoreState(this->parameters.splitterState);
	this->restoreGeometry(this->parameters.windowState);
}

void AxialPsfAnalyzerForm::getSettings(QVariantMap* settings) {
	if (!settings) {
		return;
	}

	settings->insert(AXIALPSF_BUFFER, this->parameters.bufferNr);
	settings->insert(AXIALPSF_SOURCE, static_cast<int>(this->parameters.bufferSource));
	settings->insert(AXIALPSF_FRAME, this->parameters.frameNr);
	settings->insert(AXIALPSF_NTH_BUFFER, this->parameters.nthBuffer);
	settings->insert(AXIALPSF_ROI_X, this->parameters.roi.x());
	settings->insert(AXIALPSF_ROI_Y, this->parameters.roi.y());
	settings->insert(AXIALPSF_ROI_WIDTH, this->parameters.roi.width());
	settings->insert(AXIALPSF_ROI_HEIGHT, this->parameters.roi.height());
	settings->insert(AXIALPSF_AUTOSCALING_ENABLED, this->parameters.autoScalingEnabled);
	settings->insert(AXIALPSF_AUTOFETCHING_ENABLED, this->parameters.autoFetchingEnabled);
	settings->insert(AXIALPSF_LOG_FIT_ENABLED, this->parameters.fitModeLogarithmEnabled);
	settings->insert(AXIALPSF_SPLITTER_STATE, this->parameters.splitterState);
	settings->insert(AXIALPSF_WINDOW_STATE, this->parameters.windowState);
}

bool AxialPsfAnalyzerForm::eventFilter(QObject *watched, QEvent *event) {
	if (watched == this) {
		if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
			this->parameters.windowState = this->saveGeometry();
			emit paramsChanged(this->parameters);
		}
	}
	return QWidget::eventFilter(watched, event);
}

void AxialPsfAnalyzerForm::setMaximumFrameNr(int maximum) {
	this->ui->horizontalSlider_frame->setMaximum(maximum);
	this->ui->spinBox_frame->setMaximum(maximum);
}

void AxialPsfAnalyzerForm::setMaximumBufferNr(int maximum) {
	this->ui->spinBox_buffer->setMaximum(maximum);
}

void AxialPsfAnalyzerForm::plotLine(QVector<qreal> line) {
	if(this->firstRun){
		this->enableAutoScalingLinePlot(true);
		this->linePlot->plotLine(line);
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		this->firstRun = false;
		return;
	}
	this->linePlot->plotLine(line);
}

void AxialPsfAnalyzerForm::plotData(QVector<qreal> x, QVector<qreal> y) {
	if(this->firstRun){
		this->enableAutoScalingLinePlot(true);
		this->linePlot->plotCurve(x, y);
		QCoreApplication::processEvents();
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		this->firstRun = false;
		return;
	}
	this->linePlot->plotCurve(x, y);
}

void AxialPsfAnalyzerForm::plotFittedData(QVector<qreal> x, QVector<qreal> y) {
	if(this->firstRun){
		this->enableAutoScalingLinePlot(true);
		this->linePlot->plotReferenceCurve(x, y);
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		this->firstRun = false;
		return;
	}
	this->linePlot->plotReferenceCurve(x, y);
}

void AxialPsfAnalyzerForm::plotPeakPositionIndicator(double pos) {
	if(pos < 0){
		this->linePlot->setVerticalLineVisible(false);
	} else {
		this->linePlot->setVerticalLineVisible(true);
		this->linePlot->setVerticalLine(pos);
	}
}

void AxialPsfAnalyzerForm::displayPeakPositionValue(double pos) {
	if(qIsNaN(pos)){
		this->ui->lineEdit_peakPosition->setText(tr("No peak detected"));
	} else {
		this->ui->lineEdit_peakPosition->setText(QString::number(pos, 'f', 2));
	}
}

void AxialPsfAnalyzerForm::displayFwhmValue(double value) {
	if(value < 0){
		this->ui->lineEdit_fwhm->setText(tr("Fit not possible"));
	} else {
		this->ui->lineEdit_fwhm->setText(QString::number(value, 'f', 2) + " px");
	}
}

void AxialPsfAnalyzerForm::enableAutoScalingLinePlot(bool autoScaleEnabled) {
	this->linePlot->enableAutoScaling(autoScaleEnabled);
}

