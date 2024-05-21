#include "axialpsfanalyzer.h"


AxialPsfAnalyzer::AxialPsfAnalyzer()
	: Extension(),
	form(new AxialPsfAnalyzerForm()),
	peakFit(new PeakFit()),
	frameNr(0),
	bufferNr(0),
	nthBuffer(10),
	bufferCounter(0),
	active(false),
	isCalculating(false),
	singleFetch(false),
	autoFetch(true),
	copyBufferId(-1),
	bytesPerFrameRaw(0),
	bytesPerFrameProcessed(0),
	lostBuffersProcessed(0)
{
	qRegisterMetaType<AxialPsfAnalyzerParameters>("AxialPsfAnalyzerParameters");

	this->setType(EXTENSION);
	this->displayStyle = SEPARATE_WINDOW;
	this->name = "Axial PSF Analyzer";
	this->toolTip = "FWHM measurement of axial point spread function";

	this->setupGuiConnections();
	this->setupPeakFit();
	this->initializeFrameBuffers();
}

AxialPsfAnalyzer::~AxialPsfAnalyzer() {
	peakFitThread.quit();
	peakFitThread.wait();

	delete this->form;

	this->releaseFrameBuffers(this->frameBuffersProcessed);
	this->releaseFrameBuffers(this->frameBuffersRaw);
}

QWidget* AxialPsfAnalyzer::getWidget() {
	return this->form;
}

void AxialPsfAnalyzer::activateExtension() {
	//this method is called by OCTproZ as soon as user activates the extension. If the extension controls hardware components, they can be prepared, activated, initialized or started here.
	this->active = true;
}

void AxialPsfAnalyzer::deactivateExtension() {
	//this method is called by OCTproZ as soon as user deactivates the extension. If the extension controls hardware components, they can be deactivated, resetted or stopped here.
	this->active = false;
}

void AxialPsfAnalyzer::settingsLoaded(QVariantMap settings) {
	//this method is called by OCTproZ and provides a QVariantMap with stored settings/parameters.
	this->form->setSettings(settings); //update gui with stored settings
}

void AxialPsfAnalyzer::setupGuiConnections() {
	connect(this->form, &AxialPsfAnalyzerForm::info, this, &AxialPsfAnalyzer::info);
	connect(this->form, &AxialPsfAnalyzerForm::error, this, &AxialPsfAnalyzer::error);
	connect(this, &AxialPsfAnalyzer::maxBuffers, this->form, &AxialPsfAnalyzerForm::setMaximumBufferNr);
	connect(this, &AxialPsfAnalyzer::maxFrames, this->form, &AxialPsfAnalyzerForm::setMaximumFrameNr);

	//store settings
	connect(this->form, &AxialPsfAnalyzerForm::paramsChanged, this, &AxialPsfAnalyzer::storeParameters);

	//image display connections
	ImageDisplay* imageDisplay = this->form->getImageDisplay();
	connect(this, &AxialPsfAnalyzer::newFrame, imageDisplay, &ImageDisplay::receiveFrame);
	connect(imageDisplay, &ImageDisplay::roiChanged, this, [this](const QRect& rect) {
		QString rectString = QString("ROI: %1, %2, %3, %4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
		emit this->info(rectString);
	});

	//data acquisition settings inputs from the GUI
	connect(this->form, &AxialPsfAnalyzerForm::frameNrChanged, this, [this](int frameNr) {
		this->frameNr = frameNr;
	});
	connect(this->form, &AxialPsfAnalyzerForm::bufferNrChanged, this, [this](int bufferNr) {
		this->bufferNr = bufferNr;
	});
	connect(this->form, &AxialPsfAnalyzerForm::bufferSourceChanged, this, [this](BUFFER_SOURCE source) {
		this->bufferSource = source;
	});
	connect(this->form, &AxialPsfAnalyzerForm::singleFetchRequested, this, [this]() {
		this->singleFetch = true;
	});
	connect(this->form, &AxialPsfAnalyzerForm::autoFetchRequested, this, [this](bool autoFetchEnabled) {
		this->autoFetch = autoFetchEnabled;
	});
	connect(this->form, &AxialPsfAnalyzerForm::nthBufferChanged, this, [this](int nthBuffer) {
		this->nthBuffer = nthBuffer;
	});
}

void AxialPsfAnalyzer::setupPeakFit() {
	this->peakFit = new PeakFit();
	this->peakFit->moveToThread(&peakFitThread);
	ImageDisplay* imageDisplay = this->form->getImageDisplay();
	connect(this, &AxialPsfAnalyzer::newFrame, this->peakFit, &PeakFit::fitPeak);
	connect(imageDisplay, &ImageDisplay::roiChanged, this->peakFit, &PeakFit::setRoi);
	connect(this->form, &AxialPsfAnalyzerForm::paramsChanged, this->peakFit, &PeakFit::setParams);
	connect(this->peakFit, &PeakFit::info, this, &AxialPsfAnalyzer::info);
	connect(this->peakFit, &PeakFit::error, this, &AxialPsfAnalyzer::error);
	connect(&peakFitThread, &QThread::finished, this->peakFit, &QObject::deleteLater);
	connect(this->peakFit, &PeakFit::averagedLineCalculated, this->form, &AxialPsfAnalyzerForm::plotData);
	connect(this->peakFit, &PeakFit::fitCalculated, this->form, &AxialPsfAnalyzerForm::plotFittedData);
	//connect(this->peakFit, &PeakFit::peakPositionFound, this->form, &AxialPsfAnalyzerForm::plotPeakPositionIndicator);
	connect(this->peakFit, &PeakFit::peakPositionFound, this->form, &AxialPsfAnalyzerForm::displayPeakPositionValue);
	connect(this->peakFit, &PeakFit::fwhmCalculated, this->form, &AxialPsfAnalyzerForm::displayFwhmValue);
	peakFitThread.start();
}

void AxialPsfAnalyzer::initializeFrameBuffers() {
	this->frameBuffersRaw.resize(NUMBER_OF_BUFFERS);
	this->frameBuffersProcessed.resize(NUMBER_OF_BUFFERS);
	for(int i = 0; i < NUMBER_OF_BUFFERS; i++){
		this->frameBuffersRaw[i] = nullptr;
		this->frameBuffersProcessed[i] = nullptr;
	}
}

void AxialPsfAnalyzer::releaseFrameBuffers(QVector<void *> buffers) {
	for (int i = 0; i < buffers.size(); i++) {
		if (buffers[i] != nullptr) {
			free(buffers[i]);
			buffers[i] = nullptr;
		}
	}
}

void AxialPsfAnalyzer::storeParameters() {
	//update settingsMap, so parameters can be reloaded into gui at next start of application
	this->form->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}

void AxialPsfAnalyzer::rawDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	Q_UNUSED(buffer)
	Q_UNUSED(bitDepth)
	Q_UNUSED(samplesPerLine)
	Q_UNUSED(linesPerFrame)
	Q_UNUSED(framesPerBuffer)
	Q_UNUSED(buffersPerVolume)
	Q_UNUSED(currentBufferNr)
}

void AxialPsfAnalyzer::processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	if(this->active){
		if((!this->isCalculating && this->processedGrabbingAllowed && this->autoFetch) || (this->singleFetch && this->processedGrabbingAllowed)){
			//check if current buffer is selected. If it is not selected discard it and do nothing (just return).
			if(this->bufferNr>static_cast<int>(buffersPerVolume-1)){this->bufferNr = static_cast<int>(buffersPerVolume-1);}
			if(!(this->bufferNr == -1 || this->bufferNr == static_cast<int>(currentBufferNr))){
				return;
			}

			//check if this is the nthBuffer
			this->bufferCounter++;
			if(this->bufferCounter < this->nthBuffer){
				return;
			}
			this->bufferCounter = 0;

			this->isCalculating = true;

			//calculate size of single frame
			size_t bytesPerSample = static_cast<size_t>(ceil(static_cast<double>(bitDepth)/8.0));
			size_t bytesPerFrame = samplesPerLine*linesPerFrame*bytesPerSample;

			//check if number of frames per buffer has changed and emit maxFrames to update gui
			if(this->framesPerBuffer != framesPerBuffer){
				emit maxFrames(framesPerBuffer-1);
				this->framesPerBuffer = framesPerBuffer;
			}
			//check if number of buffers per volume has changed and emit maxBuffers to update gui
			if(this->buffersPerVolume != buffersPerVolume){
				emit maxBuffers(buffersPerVolume-1);
				this->buffersPerVolume = buffersPerVolume;
			}

			//check if buffer size changed and allocate buffer memory
			if(this->frameBuffersProcessed[0] == nullptr || this->bytesPerFrameProcessed != bytesPerFrame){
				if(bitDepth == 0 || samplesPerLine == 0 || linesPerFrame == 0 || framesPerBuffer == 0){
					emit error(this->name + ":  " + tr("Invalid data dimensions!"));
					return;
				}
				//(re)create copy buffers
				if(this->frameBuffersProcessed[0] != nullptr){
					this->releaseFrameBuffers(this->frameBuffersProcessed);
				}
				for (int i = 0; i < this->frameBuffersProcessed.size(); i++) {
					this->frameBuffersProcessed[i] = static_cast<void*>(malloc(bytesPerFrame));
				}
				this->bytesPerFrameProcessed = bytesPerFrame;
			}

			//copy single frame of received data and emit it for further processing
			this->copyBufferId = (this->copyBufferId+1)%NUMBER_OF_BUFFERS;
			char* frameInBuffer = static_cast<char*>(buffer);
			if(this->frameNr>static_cast<int>(framesPerBuffer-1)){this->frameNr = static_cast<int>(framesPerBuffer-1);}
			memcpy(this->frameBuffersProcessed[this->copyBufferId], &(frameInBuffer[bytesPerFrame*this->frameNr]), bytesPerFrame);
			emit newFrame(this->frameBuffersProcessed[this->copyBufferId], bitDepth, samplesPerLine, linesPerFrame);

			this->isCalculating = false;
			this->singleFetch = false;
		}
	}
}
