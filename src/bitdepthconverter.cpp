#include "bitdepthconverter.h"
#include <QtMath>


BitDepthConverter::BitDepthConverter(QObject *parent) : QObject(parent)
{
	this->output8bitData = nullptr;
	this->bitDepth = 0;
	this->length = 0;
	this->conversionRunning = false;
}

BitDepthConverter::~BitDepthConverter()
{
	if(this->output8bitData != nullptr){
		free(this->output8bitData);
	}
}

void BitDepthConverter::convertDataTo8bit(void *inputData, int bitDepth, int samplesPerLine, int linesPerFrame) {
	if(!this->conversionRunning){
		this->conversionRunning = true;
		int length = samplesPerLine * linesPerFrame;

		//check if new output8bitData-buffer needs to be created (due to resize or first time use)
		if(this->output8bitData == nullptr || this->bitDepth != bitDepth || this->length != length){
			if(bitDepth == 0 || length == 0){
				emit error(tr("BitDepthConverter: Invalid data dimensions!"));
				return;
			}
			this->bitDepth = bitDepth;
			this->length = length;
			if(this->output8bitData != nullptr){
				free(this->output8bitData);
				this->output8bitData = nullptr; //assign nullptr to avoid dangling pointer
			}
			this->output8bitData = static_cast<uchar*>(malloc(length*sizeof(uchar)));
		}
		//no conversion needed if inputData is already 8bit or below
		if (bitDepth <= 8){
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i]; //todo: replace this for loop by memcpy
			}
		}
		//convert to 8 bit element by element
		else if (bitDepth >= 9 && bitDepth <=16){
			float factor = 255 / (qPow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i] * factor;
				//this->output8bitData[i] = static_cast<uchar*>(inputData)[2*i+1]; //for 16 bit to 8 bit this is also possible
			}
		}
		else if (bitDepth > 16 && bitDepth <=32){
			float factor = 255 / (qPow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<unsigned int*>(inputData)[i] * factor;
			}
		//do nothing if bit depth is out of range
		}else{
			return;
		}

		emit converted8bitData(output8bitData, samplesPerLine, linesPerFrame);
		this->conversionRunning = false;
	}
}
