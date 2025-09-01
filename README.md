# 🎯 CCEasyEventLoop: C++11 Event Loop with Signal/Slot and Timer

[x] 一个 **轻量级事件驱动框架**，基于 **现代 C++11**，
[x] 支持 **事件循环（Application）**、**Qt 风格 Signal/Slot**、以及 **功能丰富的定时器（CTimer）**。
[x] 完全 **跨平台**，仅依赖标准库，适合嵌入式、服务端或工具程序。

Notes: 这个是一个玩具的小项目，用来实战C++的事件驱动框架的！
---

## ✨ 核心特性

### Application

* 单例管理全局事件循环：`CApplication::global_instance()`(注: 同Qt的使用一致, 需要您在main开始的时候获取一次句柄后，在事件循环的末尾添加上`app->exec()`阻塞程序)
* 提供 `exec()` 运行循环，直到 `quit()` 被调用

### Signal/Slot

* 模板化 Signal 支持任意参数：`CSignal<Args...>`
* 支持同步/异步触发：`emit()` / `emit(…, EmitType::Sync)`
* Qt 风格注册回调：`connect(slot)`
* 所有的异步事件统一投递到 Application 循环，保证线程安全

### CTimer

* **循环/单次定时器**：支持 `start() / stop()`
* **暂停/恢复**：`pause() / resume()`
* **重置间隔**：`reset(interval_ms)`
* **切换单次模式**：`setSingleShot(true)`
* **静态单次定时器**：`CTimer::singleShot(app, ms, func)`

---

## 🛠 使用示例

#### CTimer的使用

```cpp
#include "Application.h"
#include "CTimer.h"
#include <iostream>
int main() {
	CApplication* app = CApplication::global_instance();
	int i = 0;
	CTimer loopTimer([&i] {
		std::cout << "cycle tick: " << i << std::endl;
		i++;
	}, 1000, true);

	CTimer onceTimer([] { std::cout << "single tick" << std::endl; }, 2000, true);
	onceTimer.setSingleShot(true);

	CTimer::singleShot([&app] {
		std::cout << "5s quit" << std::endl;
		app->quit();
	}, 5000);

	loopTimer.start();
	onceTimer.start();
	app->exec();
	std::cout << "App quit!\n";
}
```

#### Signal的异步使用

```cpp
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

```

---

## 🚀 使用场景

* 嵌入式设备：轻量事件驱动，无外部依赖
* 服务端后台：任务调度与定时器管理
* CLI/GUI 工具：跨平台事件循环与 Signal/Timer 支持
* 教学/实验：理解事件循环、消息队列、定时器机制

---

## 🛠 构建
* 构建的流程非常的简单，您只需要:
```bash
mkdir build
cd build
cmake ..
```

默认我们生成静态库，您可以将library文件夹下的内容黏贴到您的项目中直接add_subdirectory后使用

## 📜 许可证

MIT License，允许自由使用、修改和分发。

---

