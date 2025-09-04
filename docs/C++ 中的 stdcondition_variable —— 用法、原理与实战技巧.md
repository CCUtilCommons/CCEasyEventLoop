# C++ 中的 `std::condition_variable` —— 用法、原理与实战技巧

`std::condition_variable` 是 C++11 引入的线程同步原语，用于实现线程间的等待与通知机制，特别适用于生产者/消费者、线程池、事件等待等场景。它通常与 `std::mutex` 或 `std::shared_mutex` 配合使用，允许线程在某个条件满足之前挂起执行，避免忙等（busy-wait）浪费 CPU 资源。

------

## 基本概念与工作原理

`std::condition_variable` 的核心作用是让一个或多个线程在特定条件满足之前挂起执行，直到被其他线程通知。它的工作流程包括：

1. **等待线程**：
   - 首先通过 `std::unique_lock<std::mutex>` 获取互斥锁。
   - 然后调用 `wait()`、`wait_for()` 或 `wait_until()` 方法，这些方法会原子地释放互斥锁并挂起当前线程，直到被通知或超时。
2. **通知线程**：
   - 在某些条件满足时，调用 `notify_one()` 或 `notify_all()` 方法来通知一个或所有等待的线程。

需要注意的是，`std::condition_variable` 只与 `std::unique_lock<std::mutex>` 一起使用，而不能与 `std::lock_guard<std::mutex>` 一起使用。

------

## 主要成员函数详解

####  `wait()`

```cpp
void wait(std::unique_lock<std::mutex>& lock);
```

- 原子地释放互斥锁并挂起当前线程，直到被通知。
- 在被唤醒后，重新获取互斥锁。
- 可能会发生虚假唤醒（spurious wakeup），因此在调用 `wait()` 后应检查条件。一般我们会调用带有条件检查的重载，也就是——等到Pred条件成立后才会继续执行。

#### `wait_for()`

```cpp
template <class Rep, class Period>
std::cv_status wait_for(std::unique_lock<std::mutex>& lock,
                        const std::chrono::duration<Rep, Period>& rel_time);
```

- 原子地释放互斥锁并挂起当前线程，直到被通知或超时。
- 返回一个 `std::cv_status` 枚举值，指示是被通知还是超时。

#### `wait_until()`

```cpp
template <class Clock, class Duration>
std::cv_status wait_until(std::unique_lock<std::mutex>& lock,
                          const std::chrono::time_point<Clock, Duration>& abs_time);
```

- 原子地释放互斥锁并挂起当前线程，直到被通知或达到指定的时间点。
- 返回一个 `std::cv_status` 枚举值，指示是被通知还是超时。

#### `notify_one()`

```cpp
void notify_one();
```

- 唤醒一个正在等待的线程。
- 如果没有线程在等待，调用此方法没有任何效果。

#### `notify_all()`

```cpp
void notify_all();
```

- 唤醒所有正在等待的线程。
- 如果没有线程在等待，调用此方法没有任何效果。

------

## 使用示例：生产者/消费者模型

以下是一个简单的生产者/消费者模型示例，演示了如何使用 `std::condition_variable` 实现线程间的同步：

```cpp
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

std::queue<int> buffer;
std::mutex mtx;
std::condition_variable cv;

void producer() {
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push(i);
        std::cout << "Produced: " << i << std::endl;
        cv.notify_all();
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !buffer.empty(); });
        int value = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << value << std::endl;
        if (value == 9) break;
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    return 0;
}
```

