#pragma once

#include <stdexcept>
class UnBindAppsException : std::runtime_error {
public:
	UnBindAppsException()
	    : std::runtime_error("Unbind the Application Hooks") { };
	virtual ~UnBindAppsException() = default;
};
