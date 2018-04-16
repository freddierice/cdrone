#ifndef __LOGGER_CAMERA_H__ 
#define __LOGGER_CAMERA_H__ 
#include "logging.h"

namespace logging {
		typedef struct camera_struct { 
			double x_motion;
			double y_motion;
		} __attribute__((packed)) camera_t;

	class CameraVariable : public Variable {
		virtual std::string name() {
			return "camera";
		};
		virtual unsigned int size() {
			return  sizeof(double) +  sizeof(double) +  0;
		};
		virtual std::vector<std::string> names() {
			return {
			 "x_motion",
			 "y_motion",
			
			};
		};
		virtual std::vector<std::string> types() {
			return {
			 "double",
			 "double",
			
			};
		};
	};
	CameraVariable camera_variable;
}
#endif
