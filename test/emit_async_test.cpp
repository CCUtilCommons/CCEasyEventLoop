#include "Application.h"
#include "Signal.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main() {
	CApplication* app = CApplication::global_instance();

	std::thread s([app]() {
		std::cout << "Yet we shell quit this 5s later\n";
		std::this_thread::sleep_for(std::chrono::seconds(5));
		app->quit();
	});

	CSignal<int, std::string> signals;
	signals.connect([](int number, std::string result) {
		std::cout << "Successfully invoke the slots with: " << number
		          << ", " << result << '\n';
	});

	CSignal<> sleep_tel;
	sleep_tel.connect([]() {
		std::cout << "Successfully invoke the slots! for sleep_tel \n";
	});
	sleep_tel.connect([]() {
		std::cout << "Successfully invoke the slots2! for sleep_tel \n";
	});
	sleep_tel.emit();
	signals.emit(114514, "Charlies Job");

	app->exec();
	s.join();
}
