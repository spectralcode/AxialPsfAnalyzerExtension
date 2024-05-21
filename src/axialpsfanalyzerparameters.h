#ifndef AXIALPSFANALYZERPARAMETERS_H
#define AXIALPSFANALYZERPARAMETERS_H

#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <QRect>
#include <QByteArray>

#define AXIALPSF_SOURCE "image_source"
#define AXIALPSF_FRAME "frame_number"
#define AXIALPSF_BUFFER "buffer_number"
#define AXIALPSF_NTH_BUFFER "ntz_buffer"
#define AXIALPSF_ROI_X "roi_x"
#define AXIALPSF_ROI_Y "roi_y"
#define AXIALPSF_ROI_WIDTH "roi_width"
#define AXIALPSF_ROI_HEIGHT "roi_height"
#define AXIALPSF_AUTOSCALING_ENABLED "auto_scaling_enabled"
#define AXIALPSF_AUTOFETCHING_ENABLED "auto_fetching_enabled"
#define AXIALPSF_LOG_FIT_ENABLED "logarithm_fit_mode_enabled"
#define AXIALPSF_SPLITTER_STATE "splitter_state"
#define AXIALPSF_WINDOW_STATE "window_state"


enum BUFFER_SOURCE{
	RAW,
	PROCESSED
};

struct AxialPsfAnalyzerParameters {
	BUFFER_SOURCE bufferSource;
	QRect roi;
	int frameNr;
	int bufferNr;
	int nthBuffer;
	bool autoScalingEnabled;
	bool autoFetchingEnabled;
	bool fitModeLogarithmEnabled;
	QByteArray splitterState;
	QByteArray windowState;
};
Q_DECLARE_METATYPE(AxialPsfAnalyzerParameters)


#endif //AXIALPSFANALYZERPARAMETERS_H


