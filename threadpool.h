#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<pthread.h>
#include<list>
#include"locker.h"
#include<exception>
#include<cstdio>
using namespace std;

//线程池类，定义成模板类方便复用
template<typename T>
class threadpool{
public:
    threadpool(int thread_number = 8, int max_requests = 1000);
    ~threadpool();
    bool append(T* request);

private:
    static void * worker(void * arg);
    void run();
private:
    //线程数量
    int m_thread_number;

    //线程池数组
    pthread_t * m_threads;

    //请求队列最多允许的，等待的处理请求数量
    int m_max_requests;

    //请求队列
    list <T *> m_workqueue;

    //互斥锁
    locker m_queuelocker;

    //信号量：判断是否有任务要处理
    sem m_queuestat;

    //是否结束线程
    bool m_stop;
};
template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
    m_thread_number(thread_number),m_max_requests(max_requests),
    m_threads(NULL),m_stop(false){
    if(thread_number <= 0 || max_requests <= 0){
        throw exception();
    }

    m_threads = new pthread_t[m_thread_number];
    if(!m_threads){
        throw exception();
    }

    //创建thread_number个线程，并将他们设置为线程分离
    for(int i=0; i<m_thread_number; ++i){
        printf("create the %dth thread\n", i);
       if(pthread_create(m_threads + i, NULL, worker, this)!=0){
            //创建失败
            delete[] m_threads;
            throw exception();
       }
       if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw exception();
       }
    }

}
template<typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
    m_stop = true;
}
template<typename T>
bool threadpool<T>::append(T* request){
    //操作工作队列时一定要加锁，因为它被所有线程共享。
    m_queuelocker.lock();
    if(m_workqueue.size() > m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
void *threadpool<T>::worker(void *arg){
    threadpool * pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run(){
    while (!m_stop){
        m_queuestat.wait();
        m_queuelocker.lock();

        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if(!request){
            continue;
        }
        request->process();
    }
     
}
#endif