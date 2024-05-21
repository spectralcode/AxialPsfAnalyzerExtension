#ifndef BITDEPTHCONVERTER_H
#define BITDEPTHCONVERTER_H

#include <QObject>

class BitDepthConverter : public QObject
{
	Q_OBJECT
public:
	explicit BitDepthConverter(QObject *parent = nullptr);
	~BitDepthConverter();

private:
	uchar* output8bitData;
	int bitDepth;
	int length;
	bool conversionRunning;

public slots:
	void convertDataTo8bit(void *inputData, int bitDepth, int samplesPerLine, int linesPerFrame);

signals:
	void converted8bitData(uchar *output8bitData, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void info(QString);
	void error(QString);
};
#endif // BITDEPTHCONVERTER_H
