#pragma once
#include "../Exceptions/UnSupportAsyncEmit.hpp"
#include "Application.h"
#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

enum class EmitType {
	Async,
	Sync
};

/**
 * @brief   CSignal is a simple Signal Type like Qt, which, it only notifies
 *          what shell be happens instead of executing this
 */
template <typename... Args>
class CSignal {
public:
	using Slot = std::function<void(Args...)>; // Must Match the Signals
	CSignal() {
		application = CApplication::global_instance();
	};
	virtual ~CSignal() = default;

	inline void connect(const Slot& _slots) {
		std::lock_guard<std::mutex> guarder(locker);
		slots.emplace_back(_slots);
	}
	inline void disconnect(const Slot& _slots) {
		std::lock_guard<std::mutex> guarder(locker);
		slots.erase(std::remove(slots.begin(), slots.end(), _slots), slots.end());
	}

	void emit(Args... args, const EmitType type = EmitType::Async) {
		switch (type) {
		case EmitType::Async:
			emit_async(args...);
			break;
		case EmitType::Sync:
			emit_sync(args...);
			break;
		default:
			// Bro I am tiring wrapping up this shit!
			throw "Unexpected Connection Type";
			break;
		}
	}

protected:
	inline void emit_async(Args... args) {
		if (!application)
			throw UnSupportAsyncEmit();
		auto cb = [=]() {
			emit_sync(args...);
		};
		application->direct_post_callbacks(cb);
	}

	inline void emit_sync(Args... args) {
		std::lock_guard<std::mutex> guarder(locker);
		for (auto& slot : slots)
			slot(args...);
	}

private:
	std::vector<Slot> slots;
	std::mutex locker;
	CApplication* application { nullptr };
};
