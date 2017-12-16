#include <vector>

// TODO: change to relative paths
#include </opt/vc/include/bcm_host.h>
#include </opt/vc/include/interface/mmal/util/mmal_util.h>
#include </opt/vc/include/interface/mmal/util/mmal_util_params.h>

#include "hardware/Camera.h"
#include "misc/logging.h"
#include "misc/utility.h"

#include <opencv2/opencv.hpp>

// definitions for things that should exist in the MMAL library.
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT   1
#define MMAL_CAMERA_CAPTURE_PORT   2

// useful port numbers
#define SPLITTER_RAW_PORT     0
#define SPLITTER_ENCODER_PORT 1


Camera::Camera() : Camera(0, std::make_shared<Observations>()) {}
Camera::Camera(Config &config, std::shared_ptr<Observations> obs) : 
	Camera(config.cameraPort(), obs) {}
Camera::Camera(int port, std::shared_ptr<Observations> obs) :
   	m_port(port), m_width(320), m_height(240), m_framerate(90), m_obs(obs),
	m_running(false), m_positionEnabled(false), m_hasFirstFrame(false) {

	MMAL_PARAMETER_INT32_T cameraNum;
	MMAL_PORT_T *preview_port, *video_port, *still_port;
	MMAL_ES_FORMAT_T *format;
	MMAL_STATUS_T status;
	
	m_frameBufferLength = m_height*m_width*3;
	m_firstFrameBuffer = new char[m_frameBufferLength];
	m_previousFrameBuffer = new char[m_frameBufferLength];
	m_currentFrameBuffer = new char[m_frameBufferLength];

	m_firstFrame = cv::Mat(m_height, m_width, CV_8UC3, m_firstFrameBuffer);
	m_previousFrame = cv::Mat(m_height, m_width, CV_8UC3, m_previousFrameBuffer);
	m_currentFrame = cv::Mat(m_height, m_width, CV_8UC3, m_currentFrameBuffer);
	
	///////////////////////////////////////////////////////////////////////////
	/// CAMERA ////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	// create the camera with default port
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &m_camera);
	if (status != MMAL_SUCCESS) 
		throw CameraException("could not create camera component");
	cameraNum.hdr.id = MMAL_PARAMETER_CAMERA_NUM;
	cameraNum.hdr.size = sizeof(cameraNum);
	cameraNum.value = m_port;
	status = mmal_port_parameter_set(m_camera->control, &cameraNum.hdr);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not set camera to port");

	// sanity check
	if (!m_camera->output_num) 
		throw CameraException("camera does not have any output ports");

	// set default sensor mode
	status = mmal_port_parameter_set_uint32(m_camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, 0);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not set default sensor config");

	// enable camera control port
	status = mmal_port_enable(m_camera->control, callbackControl);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not enable camera control");

	preview_port = m_camera->output[MMAL_CAMERA_PREVIEW_PORT];
	video_port = m_camera->output[MMAL_CAMERA_VIDEO_PORT];
	still_port = m_camera->output[MMAL_CAMERA_CAPTURE_PORT];

	// set the preview format
	format = preview_port->format;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->es->video.width = VCOS_ALIGN_UP(m_width, 32);
	format->es->video.height = VCOS_ALIGN_UP(m_height, 16);
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = m_width;
	format->es->video.crop.height = m_height;
	format->es->video.frame_rate.num = 0;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(preview_port);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not commit preview port");

	// set the video format
	format = video_port->format;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->es->video.width = VCOS_ALIGN_UP(m_width, 32);
	format->es->video.height = VCOS_ALIGN_UP(m_height, 16);
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = m_width;
	format->es->video.crop.height = m_height;
	format->es->video.frame_rate.num = m_framerate;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(video_port);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not commit video port");
	if (video_port->buffer_num < 3)
		video_port->buffer_num = 3;

	// set the still format
	format = still_port->format;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->es->video.width = VCOS_ALIGN_UP(m_width, 32);
	format->es->video.height = VCOS_ALIGN_UP(m_height, 16);
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = m_width;
	format->es->video.crop.height = m_height;
	format->es->video.frame_rate.num = 0;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(still_port);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not commit still port");
	if (still_port->buffer_num < 3)
		still_port->buffer_num = 3;

	// set camera defaults
	cameraSetDefaults(m_camera);
	
	// enable camera
	if((status = mmal_component_enable(m_camera)) != MMAL_SUCCESS)
		throw CameraException("could not enable camera");

	///////////////////////////////////////////////////////////////////////////
	/// ENCODER ///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	// create encoder
	MMAL_PORT_T *encoder_input, *encoder_output;
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &m_encoder);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not create camera encoder");

	encoder_input = m_encoder->input[0];
	encoder_output = m_encoder->output[0];
	mmal_format_copy(encoder_output->format, encoder_input->format);

	encoder_output->format->encoding = MMAL_ENCODING_H264;
	encoder_output->format->bitrate = 17000000;
	encoder_output->buffer_size = encoder_output->buffer_size_recommended;
	if (encoder_output->buffer_num < encoder_output->buffer_num_min)
		encoder_output->buffer_size = encoder_output->buffer_size_min;
	encoder_output->buffer_num = encoder_output->buffer_num_recommended;
	if (encoder_output->buffer_num < encoder_output->buffer_num_min)
		encoder_output->buffer_num = encoder_output->buffer_num_min;

	encoder_output->format->es->video.frame_rate.num = 0;
	encoder_output->format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(encoder_output);
	if (status != MMAL_SUCCESS)
		throw CameraException("encode_output not formatted");

	// set the video profile type
	MMAL_PARAMETER_VIDEO_PROFILE_T videoParameter;
	videoParameter.hdr.id = MMAL_PARAMETER_PROFILE;
	videoParameter.hdr.size = sizeof(videoParameter);
	videoParameter.profile[0].profile = MMAL_VIDEO_PROFILE_H264_HIGH;
	videoParameter.profile[0].level = MMAL_VIDEO_LEVEL_H264_4;
	status = mmal_port_parameter_set(encoder_output, &videoParameter.hdr);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not set profile");

	// set the inline vectors
	status = mmal_port_parameter_set_boolean(encoder_output, 
			MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, 1);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not set INLINE_VECTORS");
	
	// enable the encoder
	if ((status = mmal_component_enable(m_encoder)) != MMAL_SUCCESS)
		throw CameraException("could not enable encoder");
	
	// initialize a port pool for the encoder port
	m_encoderPool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);
	if (!m_encoderPool)
		throw CameraException("could not create pool for encoder");
	
	///////////////////////////////////////////////////////////////////////////
	/// SPLITTER //////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &m_splitter);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not create splitter");
	
	// initialize input format
	mmal_format_copy(m_splitter->input[0]->format, 
			m_camera->output[MMAL_CAMERA_VIDEO_PORT]->format);
	if (m_splitter->input[0]->buffer_num < 3)
		m_splitter->input[0]->buffer_num = 3;
	status = mmal_port_format_commit(m_splitter->input[0]);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not commit splitter input 0 format");

	// initialize output formats
	for (int i = 0; i < (int)m_splitter->output_num; i++) {
		mmal_format_copy(m_splitter->output[i]->format, m_splitter->input[0]->format);

		if (i == SPLITTER_RAW_PORT) {
			format = m_splitter->output[i]->format;
			if (mmal_util_rgb_order_fixed(m_camera->output[MMAL_CAMERA_CAPTURE_PORT]))
				format->encoding = MMAL_ENCODING_RGB24;
			else
				format->encoding = MMAL_ENCODING_BGR24;
			format->encoding_variant = 0;
		}

		status = mmal_port_format_commit(m_splitter->output[i]);
		if (status != MMAL_SUCCESS)
			throw CameraException("could not commit splitter output port");
	}

	// enable splitter
	status = mmal_component_enable(m_splitter);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not enable the splitter");

	auto splitter_raw = m_splitter->output[SPLITTER_RAW_PORT];
	m_splitterPool = mmal_port_pool_create(splitter_raw, splitter_raw->buffer_num,
			splitter_raw->buffer_size);
	if (!m_splitterPool)
		throw CameraException("could not create the splitter pool");
	///////////////////////////////////////////////////////////////////////////
	/// PREVIEW ///////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	mmal_format_copy(m_camera->output[MMAL_CAMERA_PREVIEW_PORT]->format,
			m_camera->output[MMAL_CAMERA_VIDEO_PORT]->format);
	status = mmal_port_format_commit(m_camera->output[MMAL_CAMERA_PREVIEW_PORT]);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not enable preview port");

	// create null sink
	status = mmal_component_create("vc.null_sink", &m_preview);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not create null sink");
	status = mmal_component_enable(m_preview);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not enable null sink");

	///////////////////////////////////////////////////////////////////////////
	/// FINAL STEPS ///////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	// connect preview port to null sink
	status = mmal_connection_create(&m_previewConnection, 
			m_camera->output[MMAL_CAMERA_PREVIEW_PORT], m_preview->input[0],
			MMAL_CONNECTION_FLAG_TUNNELLING | 
			MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not connect preview to null sink");
	if ((status = mmal_connection_enable(m_previewConnection)) != MMAL_SUCCESS)
		throw CameraException("could not enable the preview connection");
	
	// connect video to splitter
	status = mmal_connection_create(&m_splitterConnection,
			m_camera->output[MMAL_CAMERA_VIDEO_PORT], m_splitter->input[0],
			MMAL_CONNECTION_FLAG_TUNNELLING |
			MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not create the splitter connection");
	if ((status = mmal_connection_enable(m_splitterConnection)) != MMAL_SUCCESS)
		throw CameraException("could not enable the splitter connection");

	// connect the splitter to the encoder
	status = mmal_connection_create(&m_encoderConnection,
			m_splitter->output[SPLITTER_ENCODER_PORT],
		   	m_encoder->input[0],
			MMAL_CONNECTION_FLAG_TUNNELLING | 
			MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not create the encoder connection");
	if ((status = mmal_connection_enable(m_encoderConnection)) != MMAL_SUCCESS)
		throw CameraException("could not enable the encoder connection");
	
	// use this object as the userdata
	m_encoder->output[0]->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	m_splitter->output[SPLITTER_RAW_PORT]->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	
	// enable encoder and raw
	if ((status = mmal_port_enable(m_splitter->output[SPLITTER_RAW_PORT], callbackRaw)) != MMAL_SUCCESS)
		throw CameraException("could not create raw callback");
	if ((status = mmal_port_enable(m_encoder->output[0], callbackEncoder)) != MMAL_SUCCESS)
		throw CameraException("could not enable encoder");
	
	m_cols = (m_width + 15) / 16 + 1;
	m_rows = (m_height + 15) / 16;
	m_blocks = m_cols * m_rows;

	console->info("camera width: {}", m_width);	
	console->info("camera height: {}", m_height);	
	console->info("camera framerate: {}", m_framerate);	
}

void Camera::callbackControl(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	// function can take a look at buffer->cmd
	//  - MMAL_EVENT_PARAMETER_CHANGED
	//  - MMAL_EVENT_ERROR
	console->info("callbackControl");
	mmal_buffer_header_release(buffer);
}

void Camera::callbackEncoder(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	Camera *camera = (Camera *)port->userdata;
	if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
		// we have an inline vector, lets use it.
		double x_motion = 0.0;
		double y_motion = 0.0;
		
		mmal_buffer_header_mem_lock(buffer);
		auto last = (MotionData*)buffer->data + camera->m_blocks;
		for (MotionData* block = (MotionData*)buffer->data; 
			   block != last; block++) {
			x_motion += block->x;
			y_motion += block->y;
		}
		mmal_buffer_header_mem_unlock(buffer);

		// normalize the motion and use lazy motion blur
		x_motion /= camera->m_blocks;
		y_motion /= -camera->m_blocks;
		x_motion *= 0.025;
		y_motion *= 0.025;
		// x_motion -= tan(camera->m_obs->skylineAngRollVel/30.0);
		// y_motion -= tan(camera->m_obs->skylineAngPitchVel/30.0);
		x_motion =  x_motion*ALPHA + (1.0-ALPHA)*camera->m_obs->cameraVelocityX;
		y_motion =  y_motion*ALPHA + (1.0-ALPHA)*camera->m_obs->cameraVelocityY;

		// set new motion
		camera->m_obs->cameraVelocityX = x_motion;
		camera->m_obs->cameraVelocityY = y_motion;
	}
	mmal_buffer_header_release(buffer);

	if (port->is_enabled) {
		buffer = mmal_queue_get(camera->m_encoderPool->queue);
		if (!buffer) {
		   console->warn("no buffer");
		   return;
		}
		if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS)
			console->info("could not send buffer to encoder");
	}
}

void Camera::resetPosition() {
	m_hasFirstFrame = false;
	m_obs->cameraPositionX = 0.0;
	m_obs->cameraPositionY = 0.0;
	m_obs->cameraPositionYaw = 0.0;
}

void Camera::callbackRaw(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	Camera *camera = (Camera *)port->userdata;
	double x, y;

	// TODO: speed up by swapping frame buffers.

	// flush
	if (!buffer->length) {
		mmal_buffer_header_release(buffer);
		return;
	}

	if (camera->m_positionEnabled) {
		// copy to current frame
		mmal_buffer_header_mem_lock(buffer);
		::memcpy(camera->m_currentFrameBuffer, buffer->data, buffer->length);
		mmal_buffer_header_mem_unlock(buffer);
		
		// if this is first frame, keep it.
		if (!camera->m_hasFirstFrame) {
			camera->m_hasFirstFrame = true;
			std::vector<int> params;
			cv::imwrite("first.png", camera->m_currentFrame, params);
			::memcpy(camera->m_firstFrameBuffer, camera->m_currentFrameBuffer, camera->m_frameBufferLength);
			::memcpy(camera->m_previousFrameBuffer, camera->m_currentFrameBuffer, camera->m_frameBufferLength);
		}

		// compare to first frame
		cv::Mat estimate = cv::estimateRigidTransform(camera->m_firstFrame, 
				camera->m_currentFrame, false);
		if (!estimate.empty()) {
			// slowly transition from lost frame to position.
			x = -1.0*estimate.at<double>(0,2) * 2.0/240.0 * camera->m_obs->infraredHeight;
			y = estimate.at<double>(1,2) * 2.0/320.0 * camera->m_obs->infraredHeight;
			camera->m_obs->cameraPositionX = x*ALPHA + camera->m_obs->cameraPositionX*(1.0-ALPHA);
			camera->m_obs->cameraPositionY = y*ALPHA + camera->m_obs->cameraPositionY*(1.0-ALPHA);
			goto done;
		}

		// we could not compare to first frame, lets now compare to previous
		estimate = cv::estimateRigidTransform(camera->m_previousFrame,
				camera->m_currentFrame, false);
		x = -1.0*estimate.at<double>(0,2) * 2.0/240.0 * camera->m_obs->infraredHeight;
		y = estimate.at<double>(1,2) * 2.0/320.0 * camera->m_obs->infraredHeight;
		
		// add the delta
		camera->m_obs->cameraPositionX = camera->m_obs->cameraPositionX + x;
		camera->m_obs->cameraPositionY = camera->m_obs->cameraPositionY + y;

		// copy current frame to the previous
		::memcpy(camera->m_previousFrameBuffer, camera->m_currentFrameBuffer, camera->m_frameBufferLength);
	}

done:
	mmal_buffer_header_release(buffer);
	if (port->is_enabled) {
		buffer = mmal_queue_get(camera->m_splitterPool->queue);
		if (!buffer)
			return;
		if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS)
			console->info("could not send buffer to encoder");
	}
}

void Camera::sendBuffers(MMAL_PORT_T* port, MMAL_POOL_T* pool) {
	int numBuffers = mmal_queue_length(pool->queue);
	for (int i = 0; i < numBuffers; i++) {
		auto buffer = mmal_queue_get(pool->queue);
		if (!buffer) {
		   console->warn("no buffer");
		   continue;
		}
		if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS) {
			console->info("could not send buffer");
		}
	}
}

void Camera::enablePosition() {
	m_positionEnabled = true;
}

void Camera::disablePosition() {
	m_positionEnabled = false;
}

void Camera::start() {
	MMAL_STATUS_T status;

	if (m_running)
		return;
	status = mmal_port_parameter_set_boolean(
			m_camera->output[MMAL_CAMERA_VIDEO_PORT], 
			MMAL_PARAMETER_CAPTURE, 1);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not start camera");
	sendBuffers(m_encoder->output[0], m_encoderPool);
	sendBuffers(m_splitter->output[0], m_splitterPool);

	m_running = true;
}

void Camera::stop() {
	MMAL_STATUS_T status;

	if (!m_running)
		return;
	status = mmal_port_parameter_set_boolean(
			m_camera->output[MMAL_CAMERA_VIDEO_PORT], 
			MMAL_PARAMETER_CAPTURE, 0);
	if (status != MMAL_SUCCESS)
		throw CameraException("could not stop camera");

	m_running = false;
}

Camera::~Camera() {
	// stop recording
	stop();

	// disable ports
	if (m_encoder) {
		mmal_port_disable(m_encoder->output[0]);
	}
	if (m_splitter){
		mmal_port_disable(m_splitter->output[SPLITTER_RAW_PORT]);
	}
	
	
	// disable components
	if (m_splitter)
		mmal_component_disable(m_splitter);
	if (m_encoder)
		mmal_component_disable(m_encoder);
	if (m_preview)
		mmal_component_disable(m_preview);
	if (m_camera)
		mmal_component_disable(m_camera);


	// destroy connections
	if (m_encoderConnection) {
		mmal_connection_destroy(m_encoderConnection);
		m_encoderConnection = NULL;
	}
	if (m_previewConnection) {
		mmal_connection_destroy(m_previewConnection);
		m_encoderConnection = NULL;
	}
	if (m_splitterConnection) {
		mmal_connection_destroy(m_splitterConnection);
		m_encoderConnection = NULL;
	}

	// destroy pools
	if (m_encoderPool) {
		mmal_port_pool_destroy(m_encoder->output[0], m_encoderPool);
		m_encoderPool = NULL;
	}
	if (m_splitterPool) {
		mmal_port_pool_destroy(m_splitter->output[SPLITTER_RAW_PORT], m_splitterPool);
		m_splitterPool = NULL;
	}
	
	// destroy components
	if (m_splitter) {
	 	mmal_component_destroy(m_splitter);
	 	m_splitter = NULL;
	}
	if (m_encoder) {
		mmal_component_destroy(m_encoder);
		m_encoder = NULL;
	}
	if (m_preview) {
		mmal_component_destroy(m_preview);
		m_preview = NULL;
	}
	if (m_camera) {
		mmal_component_destroy(m_camera);
		m_camera = NULL;
	}

	if (m_firstFrameBuffer) {
		delete m_firstFrameBuffer;
		m_firstFrameBuffer = NULL;
	}

	if (m_previousFrameBuffer) {
		delete m_previousFrameBuffer;
		m_previousFrameBuffer = NULL;
	}

	if (m_currentFrameBuffer) {
		delete m_currentFrameBuffer;
		m_currentFrameBuffer = NULL;
	}
}

void Camera::initialize() {
	bcm_host_init();
}

void Camera::cameraSetDefaults(MMAL_COMPONENT_T *camera) {
	MMAL_RATIONAL_T rational;
	int status = 0;
	
	// set saturation
	rational.num = 0; rational.den = 100;
	status |= mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SATURATION, rational);
	
	// set contrast
	rational.num = 0; rational.den = 100;
	status |= mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_CONTRAST, rational);

	// set brightness
	rational.num = 50; rational.den = 100;
	status |= mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_BRIGHTNESS, rational);

	// set ISO
	status |= mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_ISO, 0);
	
	// set video stabilisation
	status |= mmal_port_parameter_set_boolean(camera->control, MMAL_PARAMETER_VIDEO_STABILISATION, 0);
	
	// set exposure compensation
	status |= mmal_port_parameter_set_int32(camera->control, MMAL_PARAMETER_EXPOSURE_COMP, 0);

	// set exposure mode
	MMAL_PARAMETER_EXPOSUREMODE_T exp_mode;
	exp_mode.hdr.id = MMAL_PARAMETER_EXPOSURE_MODE; exp_mode.hdr.size = sizeof(exp_mode);
	exp_mode.value = MMAL_PARAM_EXPOSUREMODE_AUTO;
	status |= mmal_port_parameter_set(camera->control, &exp_mode.hdr);

	// set flicker avoid mode
	MMAL_PARAMETER_FLICKERAVOID_T fl_mode;
	fl_mode.hdr.id = MMAL_PARAMETER_FLICKER_AVOID; fl_mode.hdr.size = sizeof(fl_mode);
	fl_mode.value = MMAL_PARAM_FLICKERAVOID_OFF;
	status |= mmal_port_parameter_set(camera->control, &fl_mode.hdr);

	// set exposure meter mode
	MMAL_PARAMETER_EXPOSUREMETERINGMODE_T m_mode;
	m_mode.hdr.id = MMAL_PARAMETER_EXP_METERING_MODE; m_mode.hdr.size = sizeof(m_mode);
	m_mode.value = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	status |= mmal_port_parameter_set(camera->control, &m_mode.hdr);

	// awb mode
	MMAL_PARAMETER_AWBMODE_T awb_mode;
	awb_mode.hdr.id = MMAL_PARAMETER_AWB_MODE; awb_mode.hdr.size = sizeof(awb_mode);
	awb_mode.value = MMAL_PARAM_AWBMODE_AUTO;
	status |= mmal_port_parameter_set(camera->control, &awb_mode.hdr);
	
	// image effect
	MMAL_PARAMETER_IMAGEFX_T img_fx;
	img_fx.hdr.id = MMAL_PARAMETER_IMAGE_EFFECT; img_fx.hdr.size = sizeof(img_fx);
	img_fx.value = MMAL_PARAM_IMAGEFX_NONE;
	status |= mmal_port_parameter_set(camera->control, &img_fx.hdr);

	// set colour fx
	MMAL_PARAMETER_COLOURFX_T colfx;
	colfx.hdr.id = MMAL_PARAMETER_COLOUR_EFFECT; colfx.hdr.size = sizeof(colfx);
	colfx.enable = 0; colfx.u = 128; colfx.v = 128;
	status |= mmal_port_parameter_set(camera->control, &colfx.hdr);

	if (status)
		throw CameraException("error setting defaults");
}

// constants
const double Camera::ALPHA = 0.2;
