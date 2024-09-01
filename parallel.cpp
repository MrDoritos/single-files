//https://stackoverflow.com/questions/54525473/can-someone-help-me-parallelize-this-c-code

/*
1) take a positive integer N as an argument
2) create an integer array of size N
3) populate the integers from range [1,1000]
4) Find the largest integer and the sum of the array in parallel
5) print the largest integer and the sum of the array. 
*/

#include <stdio.h>
#include <random>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>

struct task {
    typedef void(task_func)(task*);

    task(task_func* func, int* args, int arg_count)
        : args(args), arg_count(arg_count), complete(false), func(func), ret(0) {

    }

    task():task(nullptr, nullptr, 0) {

    }

    void wait() {
        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait(lock, [this]() { return complete.load(); });
    }

    int ret;
    int* args;
    int arg_count;
    std::atomic<bool> complete;
    std::condition_variable cv;
    std::mutex cv_m;
    task_func* func;
};


struct task_manager {
    task_manager(int thread_count) {
        start_time = std::chrono::high_resolution_clock::now();
        printf("[%s] Create %i threads\n", get_time(), thread_count);
        run_pool = true;
        for (int i = 0; i < thread_count; i++) {
            threads.push_back(std::thread(&task_manager::runner, this));
        }
    }

    ~task_manager() {
        printf("[%s] Stop threads\n", get_time());
        {
            std::unique_lock<std::mutex> lock(wait_mutex);
            run_pool = false;
            cv.notify_all();
        }
        printf("[%s] Join threads\n", get_time());
        for (auto& thread : threads) {
            if (thread.joinable())
                thread.join();
        }
        printf("[%s] Threads joined\n", get_time());
    }

    char *get_time() {
        std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds ms = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
        static char buffer[32], *bufferp = &buffer[0];
        snprintf(bufferp, 32, "%03ld", ms.count());
        return buffer;
    }

    void runner() {
        std::thread::id id = std::this_thread::get_id();
        printf("[%s] [%i] Thread started\n", get_time(), id);
        while (run_pool) {
            bool empty = false;
            task* _task = nullptr;
            { //check for task
                std::lock_guard<std::mutex> lock(queue_mutex);
                if (task_pool.empty()) {
                    empty = true;
                } else {
                    _task = task_pool.front();
                    task_pool.pop();
                }
            }

            if (empty) {
                printf("[%s] [%i] Wait for task\n", get_time(), id);
                std::unique_lock<std::mutex> lock(wait_mutex);
                cv.wait(lock, [this]() { return !task_pool.empty() || !run_pool; });
                continue;
            }

            printf("[%s] [%i] Run task\n", get_time(), id);

            if (_task && !_task->complete) {
                _task->func(_task);
                _task->cv.notify_all();
            }

            printf("[%s] [%i] Done task\n", get_time(), id);
        }

        printf("[%s] [%i] Thread stopped\n", get_time(), id);
    }

    void add_task(task* t) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        task_pool.push(t);
        cv.notify_one();
    }

    std::mutex wait_mutex;
    std::condition_variable cv;
    std::mutex queue_mutex;
    std::atomic<bool> run_pool;
    std::queue<task*> task_pool;
    std::vector<std::thread> threads;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

void sum(task* t) {
    int sum = 0;
    for (int i = 0; i < t->arg_count; i++) {
        sum += t->args[i];
    }
    t->ret = sum;
    t->complete = true;
}

void max(task* t) {
    int max = 0;
    for (int i = 0; i < t->arg_count; i++) {
        if (t->args[i] > max) {
            max = t->args[i];
        }
    }
    t->ret = max;
    t->complete = true;
}

void run(task_manager *man, int N) {
    int *array = new int[N];

    for (int i = 0; i < N; i++) {
        array[i] = i;
    }

    task *task_max = new task((task::task_func*)&max, array, N);
    task *task_sum = new task((task::task_func*)&sum, array, N);

    man->add_task(task_max);
    man->add_task(task_sum);

    printf("[%s] Wait for tasks\n", man->get_time());
    task_max->wait();
    task_sum->wait();

    printf("[%s] N: %i, max: %i, sum: %i\n", man->get_time(), N, task_max->ret, task_sum->ret);

    delete task_max;
    delete task_sum;
}

int main() {
    task_manager manager(2);

    srand(time(0));

    for (int i = 0; i < 10; i++)
        run(&manager, 1000 + (rand() % 500));

    printf("[%s] Run completed\n", manager.get_time());

    return 0;
}