#include "CTimer.h"
#include "Application.h"

CTimer::CTimer(std::function<void()> func, int interval, bool isRepeatType)
    : interval(interval)
    , task(func)
    , repeat(isRepeatType)
    , running(false)
    , paused(false) {
}

CTimer::~CTimer() {
	stop();
}

bool CTimer::start() {
	if (running)
		return false; // Already running
	running = true;
	paused = false;
	worker = std::thread(&CTimer::run_loop, this);
	return true;
}

bool CTimer::stop() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		running = false;
		paused = false;
	}
	cv.notify_all();
	if (worker.joinable())
		worker.join();
	return true;
}

bool CTimer::pause() {
	std::lock_guard<std::mutex> lock(mtx);
	paused = true;
	return paused;
}
bool CTimer::resume() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		paused = false;
	}
	cv.notify_all();
	return true;
}

void CTimer::reset(int new_interval_ms, bool restart) {
	{
		std::lock_guard<std::mutex> lock(mtx);
		interval = new_interval_ms;
	}
	if (restart) {
		stop();
		start();
	}
}

void CTimer::singleShot(std::function<void()> func, int interval_ms) {
	std::thread([interval_ms, func] {
		std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
		CApplication* app = CApplication::global_instance();
		app->postActions(func);
	}).detach();
}

void CTimer::run_loop() {
	std::unique_lock<std::mutex> lock(mtx);
	while (running) {
		if (cv.wait_for(lock,
		                std::chrono::milliseconds(interval),
		                [this] { return !running || paused; })) {
			while (paused && running) {
				cv.wait(lock);
			}
			continue;
		}

		if (!running)
			break;
		CApplication* app = CApplication::global_instance();
		lock.unlock();
		app->postActions(task);
		lock.lock();

		if (!repeat) {
			running = false;
		}
	}
}