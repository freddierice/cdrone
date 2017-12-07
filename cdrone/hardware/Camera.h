#ifndef __CAMERA_H__
#define __CAMERA_H__
#include <atomic>
#include <string>
#include <memory>

#include "misc/Config.h"
#include "misc/exception.h"
#include "misc/Observations.h"

#include </opt/vc/include/interface/mmal/mmal.h>
// #include </opt/vc/include/interface/mmal/mmal_parameters_camera.h>
#include </opt/vc/include/interface/mmal/mmal_component.h>
#include </opt/vc/include/interface/mmal/util/mmal_default_components.h>
#include </opt/vc/include/interface/mmal/util/mmal_connection.h>

#include <opencv2/opencv.hpp>

CDRONE_EXCEPTION(HardwareException, CameraException);

class Camera {
public:
	Camera();
	Camera(Config& config, std::shared_ptr<Observations> obs);
	Camera(int port, std::shared_ptr<Observations> obs);
	~Camera();

	static void initialize();
	
	// start or stop recording
	void start();
	void stop();

	// turn on/off position
	void enablePosition();
	void resetPosition();
	void disablePosition();
	
private:

	// a callback from the mmal interface
	static void callbackControl(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);
	static void callbackEncoder(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void callbackRaw(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void sendBuffers(MMAL_PORT_T* port, MMAL_POOL_T* pool);

	// set defaults for a camera component
	static void cameraSetDefaults(MMAL_COMPONENT_T *camera);

	// camera component
	MMAL_COMPONENT_T* m_camera;
	// encoder component
	MMAL_COMPONENT_T* m_encoder;
	// splitter from the video for raw data
	MMAL_COMPONENT_T* m_splitter;
	MMAL_COMPONENT_T* m_preview;
	// connection from camera to encoder
	MMAL_CONNECTION_T* m_encoderConnection;
	MMAL_CONNECTION_T* m_splitterConnection;
	MMAL_CONNECTION_T* m_previewConnection;
	// buffer pool for encoder
	MMAL_POOL_T *m_encoderPool;
	MMAL_POOL_T *m_splitterPool;

	// details about camera
	int m_port;
	int m_width, m_height, m_framerate;
	int m_cols, m_rows, m_blocks;
	std::string m_name;

	// observations
	std::shared_ptr<Observations> m_obs;

	// camera running
	bool m_running;

	// position information
	bool m_positionEnabled;
	bool m_hasFirstFrame;
	int m_frameBufferLength;
	cv::Mat m_firstFrame, m_previousFrame, m_currentFrame;
	char *m_firstFrameBuffer, *m_previousFrameBuffer,
		 *m_currentFrameBuffer;
	
	// constants
	static const double ALPHA;
};

// type definition for the inline vectors
typedef struct MotionData_struct {
	int8_t x;
	int8_t y;
	uint16_t sad;
} MotionData;

#endif /* __CAMERA_H__ */
