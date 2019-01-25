#ifndef RAY_QUEUE_H
#define RAY_QUEUE_H
#include <thread>
#include "render.h"
#include <mutex>
#include <queue>
using std::recursive_mutex;
using std::thread;
using std::queue;
using std::vector;  

struct RayTask;

class RayQueue
{
  private:
    int batch_size;
    int rays_total = 0;

  public:
    RayQueue(int batch_size) : batch_size(batch_size){};
    recursive_mutex lock;
    queue<RayTask> queue;
    void push_batch(vector<RayTask> tasks);
    void push(RayTask task);
    RayTask pop();
    bool empty();
    vector<RayTask> pop_batch(int batch_size);
};

#endif