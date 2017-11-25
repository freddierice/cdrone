#ifndef __CAMERA_H__
#define __CAMERA_H__
#include <string>
#include "misc/Config.h"
#include "misc/exception.h"

#include </opt/vc/include/interface/mmal/mmal.h>
// #include </opt/vc/include/interface/mmal/mmal_parameters_camera.h>
#include </opt/vc/include/interface/mmal/mmal_component.h>
#include </opt/vc/include/interface/mmal/util/mmal_default_components.h>

CDRONE_EXCEPTION(HardwareException, CameraException);

class Camera {
public:
	Camera(Config& config);
	Camera(const std::string& filename);
	~Camera();

	void initialize();
	
	// atomic reference to motion vectors
private:
	Camera();

	// camera component
	MMAL_COMPONENT_T* m_camera;
};

#endif /* __CAMERA_H__ */
