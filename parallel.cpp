//https://stackoverflow.com/questions/54525473/can-someone-help-me-parallelize-this-c-code

/*
1) take a positive integer N as an argument
2) create an integer array of size N
3) populate the integers from range [1,1000]
4) Find the largest integer and the sum of the array in parallel
5) print the largest integer and the sum of the array. 
*/

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

struct task {
    typedef void(task_func)(task*);

    task(task_func* func, int* args, int arg_count)
        : args(args), arg_count(arg_count), complete(false), func(func), ret(0) {

    }

    task():task(nullptr, nullptr, 0) {

    }

    void wait() {
        while (!complete) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    int ret;
    int* args;
    int arg_count;
    std::atomic<bool> complete;
    task_func* func;
};

std::mutex queue_mutex;
std::queue<task*> task_pool;
std::atomic<bool> run_pool;

void runner() {
    while (run_pool) {
        bool empty = false;
        task* _task = nullptr;
        { //check for task
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (task_pool.empty()) {
                empty = true;
            } else {
                _task = task_pool.front();
            }
        }

        if (empty) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (_task && !_task->complete) {
            _task->func(_task);
        }
    }
}

void max (task* t) {
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

void run(int N) {
    int max = 0;
    int sum = 0;
    int array[N];

    for (int i = 0; i < N; i++) {
        array[i] = i;
    }

    task *max = new task((task::task_func)max, new int[N], N);
    task *sum = new task((task::task_func)&sum, new int[N], N);

    {  
        std::lock_guard<std::mutex> lock(queue_mutex);
        task_pool.push(max);
        task_pool.push(sum);
    }

    delete max;
    delete sum;
}

int main() {

}