#ifndef __LOGGER_VRPN_H__ 
#define __LOGGER_VRPN_H__ 
#include "logging.h"

namespace logging {
		typedef struct vrpn_struct { 
			uint64_t sensor_time;
			uint32_t sensor;
			double x;
			double y;
			double z;
			double quat0;
			double quat1;
			double quat2;
			double quat3;
		} __attribute__((packed)) vrpn_t;

	class VrpnVariable : public Variable {
		virtual std::string name() {
			return "vrpn";
		}
		virtual unsigned int size() {
			return  sizeof(uint64_t) +  sizeof(uint32_t) +  sizeof(double) +  sizeof(double) +  sizeof(double) +  sizeof(double) +  sizeof(double) +  sizeof(double) +  sizeof(double) +  0;
		}
		virtual std::vector<std::string> names() {
			return {
			 "sensor_time",
			 "sensor",
			 "x",
			 "y",
			 "z",
			 "quat0",
			 "quat1",
			 "quat2",
			 "quat3",
			
			};
		}
		virtual std::vector<std::string> types() {
			return {
			 "uint64_t",
			 "uint32_t",
			 "double",
			 "double",
			 "double",
			 "double",
			 "double",
			 "double",
			 "double",
			
			};
		}
	};
	extern VrpnVariable vrpn_variable;
}
#endif
