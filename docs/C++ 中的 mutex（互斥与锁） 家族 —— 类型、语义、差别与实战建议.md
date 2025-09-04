# C++ 中的 **mutex（互斥与锁）** 家族 —— 类型、语义、差别与实战建议

## 多线程编程的痛点与为什么我们需要锁

​	在多线程程序里，当多个线程访问**同一份可变数据**时，必须防止同时写或读写的不安全并发 —— 否则会产生数据竞争（data race）和未定义行为。C++ 标准库提供了多种互斥/同步原语来序列化对共享资源的访问，最直观的就是“互斥锁（mutex）”。

​	一个最简单的例子就是

```c++
static int cnt = 0;
void inc_count(){
	for(int i = 0; i < 10000; i++)
        cnt++;
}

int main()
{
    std::thread t1(inc_count);
    std::thread t2(inc_count);
	t1.join();
    t2.join();
}
```

​	一般而言，如果你处于Release Mode下运行，得到的i的值往往不尽如意，这是因为cnt++不是一个原子化的操作，或者说，当我们对一个处于静态区的变量进行自增的时候，我们往往需要做三个步骤：

1. 从内存中取出来cnt
2. 使用表达为inc自增的指令对cnt做加操作
3. 放回去

​	我们很容易想到，在调度的时候，如果我们一个线程在刚去出来cnt的时候被切换到另一个线程再取cnt，这个时候我们都再修改cnt的副本然后放回去，导致本来该加2次，实际上才加了一次。这种视图不一致造成了非法的结果。这就是为什么我们需要锁。

------

## 标准库内的主要互斥类型（一览与核心语义）

> 下列都是 C++ 标准 `<mutex>` / `<shared_mutex>` 中定义的类型（按常用与功能分类）：

#### `std::mutex` — 基本的**独占**互斥锁

语义：一次只允许一个线程拥有；非递归（同一线程再次 `lock()` 会造成未定义/死锁行为）。锁是不可拷贝的（只能移动或保持在同一对象上）。`lock()` 会阻塞直到获得锁，`try_lock()` 立即返回成功/失败。

#### 2) `std::timed_mutex` — 带**超时尝试**的独占锁

在 `std::mutex` 基础上增加 `try_lock_for()` / `try_lock_until()`，方便做超时等待或轮询式获取锁。适合可能需要“等待一段时间然后放弃”的场景。

#### 3) `std::recursive_mutex` — **可重入（递归）**的互斥锁

允许同一线程对同一 mutex 重复 `lock()` 多次（每次 `lock()` 必须有对应 `unlock()`），常用于递归调用或多层函数共享同一锁的情况。但递归锁可能掩盖设计问题（应谨慎使用）。

#### 4) `std::recursive_timed_mutex` — 递归 + 定时尝试（结合 2 和 3）

同时支持递归语义和 `try_lock_for/try_lock_until`，用于需要递归且也需要超时控制的场景。

#### 5) `std::shared_mutex` / `std::shared_timed_mutex` — **读写锁（Readers–Writers）**

提供两种锁模式：**shared（共享读取）** 和 **exclusive（独占写入）**。多个线程可以并发持有 shared（用于读取），但在有线程持有 exclusive 时其它线程既不能读也不能写。`shared_timed_mutex` 还支持 timed try / wait。适用于“读多写少”场景。标准上并**不固定读/写优先策略**（实现/平台可能不同），所以在高竞争环境要留意可能的饥饿或公平性问题。

------

## RAII 与锁封装（推荐用法）

不要直接调用 `mutex.lock()` / `unlock()` 除非有充分理由 —— 推荐使用 RAII 封装，避免异常或路径分支导致忘记解锁。

- `std::lock_guard<Mutex>`：最常用、最轻量的作用域锁（构造就 `lock()`，析构 `unlock()`）。适合简单场景。
- `std::unique_lock<Mutex>`：更通用（可延迟加锁、可解锁/重新加锁、可 `release()` 交出所有权、支持 timed try）；常与 `std::condition_variable` 配合。`unique_lock` 可移动不可拷贝。
- `std::scoped_lock<Mutex...>`（C++17）：一次性安全地对多个 mutex 加锁（内部采用避免死锁的策略），推荐用于需要同时锁住多个互斥量的场景。比 `std::lock` + `lock_guard` 更简洁安全。
- `std::shared_lock<SharedMutex>`：用于 `shared_mutex` 的共享（读）RAII 封装，支持 defer/timed 等策略。

**锁标签（lock tag）**：`std::defer_lock`（延迟不加锁）、`std::try_to_lock`（尝试不阻塞）、`std::adopt_lock`（假定当前线程已拥有锁）可用于 `unique_lock` / `shared_lock` 构造，灵活控制加锁策略。

------

## 四、各锁的典型用法示例（精简）

#### `std::mutex + lock_guard`

```cpp
std::mutex m;
void push(int x) {
    std::lock_guard<std::mutex> lk(m);
    q.push(x);
} // lk 析构时自动 unlock
```

#### `std::unique_lock` + `condition_variable`

```cpp
std::unique_lock<std::mutex> lk(m); 
cv.wait(lk, []{ return ready; }); // wait 需 unique_lock
```

#### `std::timed_mutex` 尝试超时

```cpp
if (m.try_lock_for(std::chrono::milliseconds(10))) {
    // got it
    m.unlock();
} else {
    // give up
}
```

#### `std::recursive_mutex`（谨慎）

```cpp
void f() {
    std::lock_guard<std::recursive_mutex> lk(m);
    // 可以安全地在递归或被其它函数再次 lock()
}
```

#### `std::shared_mutex` + `std::shared_lock`（读写分离）

```cpp
std::shared_mutex rw;
void reader() {
    std::shared_lock<std::shared_mutex> rlk(rw);
    // 多个 reader 并行
}
void writer() {
    std::unique_lock<std::shared_mutex> wlk(rw); // exclusive
}
```

------

## 主要差异速览表（便于记忆）

| 类型                         | 读/写模型               | 递归 | timed 支持            | RAII 常配                   | 典型用途             |
| ---------------------------- | ----------------------- | ---- | --------------------- | --------------------------- | -------------------- |
| `std::mutex`                 | 独占                    | 否   | 否                    | `lock_guard`/`unique_lock`  | 通用互斥             |
| `std::timed_mutex`           | 独占                    | 否   | 是                    | `unique_lock`               | 需要超时重试场景     |
| `std::recursive_mutex`       | 独占                    | 是   | 否                    | `lock_guard`                | 递归调用需要（谨慎） |
| `std::recursive_timed_mutex` | 独占                    | 是   | 是                    | `unique_lock`               | 递归 + 超时          |
| `std::shared_mutex`          | 读/写（共享读、独占写） | 否   | 否（shared_timed 有） | `shared_lock`/`unique_lock` | 读多写少             |
| `std::shared_timed_mutex`    | 读/写                   | 否   | 是                    | `shared_lock`/`unique_lock` | 读多写少 + timed     |

