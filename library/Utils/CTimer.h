#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
class CApplication;

class CTimer {
public:
	CTimer() = delete;
	CTimer(const CTimer&) = delete;
	CTimer& operator=(const CTimer&) = delete;

	CTimer(std::function<void()> func, int interval = 1000, bool isRepeatType = true);
	~CTimer();

	bool start();
	bool stop();

	bool pause();
	bool resume();

	void reset(int new_interval_ms, bool restart = true);

	static void singleShot(std::function<void()> func, int interval_ms = 1000);
	inline void setSingleShot(bool once) {
		std::lock_guard<std::mutex> lock(mtx);
		repeat = !once;
	}

	inline bool isSingleShot() const {
		return !repeat;
	}

private:
	void run_loop();
	CApplication* app { nullptr };

	std::function<void()> task;
	int interval;
	bool repeat { false };

	std::atomic<bool> running;
	bool paused { false };
	std::thread worker;

	std::mutex mtx;
	std::condition_variable cv;
};
