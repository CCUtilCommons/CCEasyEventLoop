#include "Application.h"

CApplication* CApplication::global_instance() noexcept {
	static CApplication application;
	return &application;
}

CApplication::CApplication() {
}

void CApplication::postActions(PlainHooks hooks) {
	direct_post_callbacks(hooks);
}

void CApplication::direct_post_callbacks(PlainHooks hooks) {
	{
		std::lock_guard<std::mutex> lock(mtx);
		this->hooks.emplace(hooks);
	}
	cv.notify_one();
}

void CApplication::exec() {
	loop_activate = true;
	event_loop();
}

void CApplication::quit() {
	std::lock_guard<std::mutex> lock(mtx);
	loop_activate = false;
	cv.notify_all();
}

void CApplication::event_loop() {
	std::unique_lock<std::mutex> lock(mtx);
	while (loop_activate) {
		if (hooks.empty()) {
			cv.wait(lock, [this]() -> bool {
				return !loop_activate || !hooks.empty();
			});
		}

		while (!hooks.empty()) {
			auto front_session = hooks.front();
			hooks.pop();
			lock.unlock();
			front_session();
			lock.lock();
		}
	}
}