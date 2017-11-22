#ifndef __EXCEPTION_H__ 
#define __EXCEPTION_H__

#include <stdexcept>

// a macro to make an exception subclass. 
#define CDRONE_EXCEPTION(parent, child) class child: public parent { \
public: \
	child(std::string const& msg) : parent(msg) {} \
}

// CDroneException is the base exception for project exceptions.
CDRONE_EXCEPTION(std::runtime_error, CDroneException);

// HardwareException encompasses any exception that occurs as a result of 
// either missing hardware or unexpected behavior.
CDRONE_EXCEPTION(CDroneException, HardwareException);

// ControllerException encompasses any exception that occurs as a result of
// errors 
CDRONE_EXCEPTION(CDroneException, ControllerException);

#endif /* __EXCEPTION_H__ */
