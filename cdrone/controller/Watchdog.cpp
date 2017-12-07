#include <signal.h>
#include <sys/time.h>
#include "Watchdog.h"

#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <stdexcept>

Watchdog::Watchdog(std::chrono::milliseconds duration) : Watchdog(duration, 
		"(no name)") {}
Watchdog::Watchdog(std::chrono::milliseconds duration,
		const std::string &name) : m_name(name) {
	m_id = m_nextThreadID++;
	m_duration = duration;
	if (m_id >= m_nWatchdogs)
		throw std::runtime_error("too many watchdogs created");
}

// should not be run
Watchdog::Watchdog() {
	throw std::runtime_error("cannot run the default Watchdog constructor");
}

void Watchdog::initialize(int n) {
	spdlog::get("console")->info("Watchdog::initialize");
	m_nextThreadID = 0;
	m_nWatchdogs = n;
	if (m_counts) {
		delete m_counts;
		m_counts = NULL;
	}
	if (m_maxes) {
		delete m_maxes;
		m_maxes = NULL;
	}
	m_counts = new std::atomic<uint64_t>[n]();
	m_maxes = new uint64_t[n]();
	m_shutdown = false;

	m_thread = std::thread(check_thread);
}

void Watchdog::destroy() {
	if (m_nWatchdogs > 0) {
		m_shutdown = true;
		m_thread.join();
		m_nWatchdogs = 0;
	}
	if (m_counts) {
		delete m_counts;
		m_counts = NULL;
	}
	if (m_maxes) {
		delete m_maxes;
		m_maxes = NULL;
	}

}

void Watchdog::check_thread() {
	auto lastTime = std::chrono::high_resolution_clock::now();
	auto extra = std::chrono::milliseconds(0);
	while (!m_shutdown) {
		// pause and calculate how long we were asleep for
		std::this_thread::sleep_for(EPOCH);
		auto nowTime = std::chrono::high_resolution_clock::now();
		extra += std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastTime) - EPOCH;
		uint64_t addCount = 1;
		if (extra > EPOCH) {
			int numOver = (int)(extra.count() / EPOCH.count());
			addCount += numOver;
			extra -= numOver*EPOCH; //XXX: is this ok?
		}
		lastTime = nowTime;

		// increment the m_counts by addCount
		for (uint64_t i = 0; i < m_nWatchdogs; i++)
			m_counts[i] += addCount;
		for (uint64_t i = 0; i < m_nWatchdogs; i++)
			if (m_maxes[i] != 0 && m_counts[i] > m_maxes[i]) {
				// TODO: add killing name.
				kill(0, SIGALRM);
			}
	}
}

void Watchdog::start() {
	m_counts[m_id] = 0;
	m_maxes[m_id] = (uint64_t)(m_duration.count() + EPOCH.count() - 1) / (EPOCH.count());	
}

void Watchdog::ok() {
	m_counts[m_id] = 0;
}

void Watchdog::stop() {
	m_maxes[m_id] = 0;
}


// initialize static members of Watchdog
const std::chrono::milliseconds Watchdog::EPOCH(100);

std::atomic<uint64_t> Watchdog::m_nextThreadID;
std::atomic<uint64_t>* Watchdog::m_counts;
uint64_t* Watchdog::m_maxes;
uint64_t Watchdog::m_nWatchdogs;
std::atomic<bool> Watchdog::m_shutdown;
std::thread Watchdog::m_thread;

