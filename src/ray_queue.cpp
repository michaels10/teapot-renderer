#include "ray_queue.h"

void RayQueue::push_batch(vector<RayTask> rays) {
    lock.lock();
    for (RayTask &ray : rays) {
        queue.push(ray);
        rays_total++;
    }
    lock.unlock();
}

void RayQueue::push(RayTask ray) {
    lock.lock();
    queue.push(ray);
    rays_total++;
    lock.unlock();
}

RayTask RayQueue::pop(){
  lock.lock();
  auto rv = queue.front();
  queue.pop();
  lock.unlock();
  return rv;
}

vector<RayTask> RayQueue::pop_batch(int batch_size) {
    lock.lock();
    printf("There have been %d rays total!\n", rays_total);
    vector<RayTask> vec(batch_size);
    int remaining = batch_size;
    while(remaining > 0 && !queue.empty()){
      vec.push_back(queue.front());
      queue.pop();
      remaining--;
    }
    lock.unlock();
    return vec;
}

bool RayQueue::empty(){
  lock.lock();
  bool empty = queue.empty();
  lock.unlock();
  return empty;
}