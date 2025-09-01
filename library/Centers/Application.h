#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
class CEvent;

class CApplication {
public:
	using PlainHooks = std::function<void()>;
	static CApplication* global_instance() noexcept;

	void postActions(PlainHooks event);

	void exec();
	void quit();

protected:
	template <typename... Args>
	friend class CSignal;
	void direct_post_callbacks(PlainHooks hooks);

private:
	std::queue<PlainHooks> hooks;
	std::atomic<bool> loop_activate { false };
	std::mutex mtx;
	std::condition_variable cv;

private:
	CApplication();
	CApplication(const CApplication&) = delete;
	CApplication& operator=(const CApplication&) = delete;

	void event_loop();
};
