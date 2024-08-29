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

std::atomic<bool> run_pool;

struct task {
    typedef void(task_func)(task*);

    task(task_func* func, int* args, int arg_count)
        : args(args), arg_count(arg_count), complete(false), func(func), ret(0) {

    }

    task():task(nullptr, nullptr, 0) {

    }

    void wait() {
        while (!complete && run_pool) {
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

void runner() {
    printf("Start thread\n");
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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        printf("Run task\n");

        if (_task && !_task->complete) {
            _task->func(_task);
        }

        printf("Done task\n");
    }
}

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

void run(int N) {
    int *array = new int[N];

    for (int i = 0; i < N; i++) {
        array[i] = i;
    }

    task *task_max = new task((task::task_func*)&max, array, N);
    task *task_sum = new task((task::task_func*)&sum, array, N);

    {  
        std::lock_guard<std::mutex> lock(queue_mutex);
        printf("Push tasks\n");
        task_pool.push(task_max);
        task_pool.push(task_sum);
    }

    printf("Wait for tasks\n");
    task_max->wait();
    task_sum->wait();

    printf("N: %i, max: %i, sum: %i\n", N, task_max->ret, task_sum->ret);

    delete task_max;
    delete task_sum;
}

int main() {
    run_pool = true;
    std::thread a(runner), b(runner);

    run(1000);

    run_pool = false;
    a.join();
    b.join();
}