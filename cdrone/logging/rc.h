#ifndef __LOGGER_RC_H__ 
#define __LOGGER_RC_H__ 
#include "logging.h"

namespace logging {
		typedef struct rc_struct { 
			uint16_t roll;
			uint16_t pitch;
			uint16_t yaw;
			uint16_t thrust;
			uint16_t aux1;
		} __attribute__((packed)) rc_t;

	class RcVariable : public Variable {
		public:
		virtual std::string name() {
			return "rc";
		}
		virtual unsigned int size() override {
			return  sizeof(uint16_t) +  sizeof(uint16_t) +  sizeof(uint16_t) +  sizeof(uint16_t) +  sizeof(uint16_t) +  0;
		}
		virtual std::vector<std::string> names() {
			return {
			 "roll",
			 "pitch",
			 "yaw",
			 "thrust",
			 "aux1",
			
			};
		}
		virtual std::vector<std::string> types() {
			return {
			 "uint16_t",
			 "uint16_t",
			 "uint16_t",
			 "uint16_t",
			 "uint16_t",
			
			};
		}
	};
	RcVariable rc_variable;
}
#endif
