#include "Application.h"
#include "CTimer.h"
#include <iostream>
int main() {
	CApplication* app = CApplication::global_instance();
	int i = 0;
	CTimer loopTimer([&i] {
		std::cout << "cycle tick: " << i << std::endl;
		i++;
	},
	                 1000, true);

	CTimer onceTimer([] { std::cout << "single tick" << std::endl; }, 2000, true);
	onceTimer.setSingleShot(true);

	CTimer::singleShot([&app] {
		std::cout << "5s quit" << std::endl;
		app->quit();
	},
	                   5000);

	loopTimer.start();
	onceTimer.start();
	app->exec();
	std::cout << "App quit!\n";
}