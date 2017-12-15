#include "controller/IOController.h"
#include "misc/logging.h"

IOController::IOController(Config &config) : m_server(config) {
}

IOController::~IOController() {
}

std::shared_ptr<IO> IOController::accept() try {
	return m_server.accept();
} catch (ServerException &ex) {
	throw IOControllerException(ex.what());
}
