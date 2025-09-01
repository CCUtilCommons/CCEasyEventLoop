#pragma once

#include <stdexcept>
class UnSupportAsyncEmit : public std::runtime_error {
public:
	UnSupportAsyncEmit()
	    : std::runtime_error("Can not runs the async emit due to non apps binds") { };
	virtual ~UnSupportAsyncEmit() = default;
};
