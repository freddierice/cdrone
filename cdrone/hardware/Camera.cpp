#include "Camera.h"

// TODO: change to relative paths
#include </opt/vc/include/bcm_host.h>

Camera::Camera() : Camera("/dev/nyi") {}
Camera::Camera(Config &config) : Camera(config.cameraPort()) {}
Camera::Camera(const std::string& filename) {
	MMAL_STATUS_T status;
	
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &m_camera);
	if (status != MMAL_SUCCESS) 
		throw CameraException("could not create camera component");
}

Camera::~Camera() {
	if(m_camera)
		mmal_component_destroy(m_camera);
}

void Camera::initialize() {
	bcm_host_init();
}
