#include "Application.h"
#include "Signal.hpp"
#include <iostream>
#include <string>

int main() {
	CSignal<int, std::string> signals;
	signals.connect([](int number, std::string result) {
		std::cout << "Successfully invoke the slots with: " << number
		          << ", " << result << '\n';
	});

	signals.emit(114514, "Charlies Job", EmitType::Sync);
}
