#pragma once
#include<iostream>
#include<vector>
#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>


class ThreadPool{
public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    // 提交任务队列
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... agrs) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> workers;   // 干活的线程们
    std::queue<std::function<void()>> tasks;    // 任务队列

    std::mutex queue_mutex;         // 保护队列的锁
    std::condition_variable condition;  //条件变量
    bool stop;                          // 线程池停止标志

};

// 构造函数:启动n个线程
inline ThreadPool::ThreadPool(size_t threads) : stop(false){
    for(size_t i =0; i < threads; ++i){
        workers.emplace_back([this] {       // this把上下文传进去，创建线程，lambda是在线程中执行的函数
            while(true){
                std::function<void()> task;
                {
                    // 1.加锁
                    std::unique_lock<std::mutex> lock(this->queue_mutex);  // 配合条件变量使用的lock

                    // 2.如果队列为空且没要求停止，就睡觉（阻塞），直到被唤醒（wait是false阻塞）（unique_lock配合这个condition_variable）
                    this->condition.wait(lock, [this]{
                        return this->stop || !this->tasks.empty();
                    });

                    // 3.如果要求停止，且队列没任务，就退出线程
                    if (this->stop && this->tasks.empty()) return;

                    // 4.取一个任务
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }// 离开作用域自动解锁

                // 5.在锁外面执行任务（在锁里面执行，其他的线程没法拿任务
                task();
            }
        });
    }
}

// 析构函数：通知所有的线程下班
inline ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();     // 叫醒所有睡觉的线程
    for(std::thread& worker : workers){
        worker.join();  // 等所有线程安全退出
    }
}

// 添加任务（模板函数）
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    using return_type = typename std::result_of<F(Args...)>::type;      // 简写

    // 把任务包装成一个无参数、无返回值的 std::function,装入tasks queue中
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop) throw std::runtime_error("enqueu on stopeped ThreadPool");
        
        // 放进队列
        tasks.emplace([task](){(*task)(); });
    }
    condition.notify_one();  // 叫醒一个线程去干活，这是一个非常轻量级的动作，如果没有空闲的线程（没人在wait）就相当于喊了一声空号，就立刻返回

    return res;

}