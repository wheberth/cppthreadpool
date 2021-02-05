/*
    MIT License

    Copyright (c) 2021 Wheberth Dias

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/*
  This project implements a generic thread pool in C++ which reuse threads and 
  schedule the execution of a (thread safe) function void function. Have fun ;)
*/

#pragma once
#ifndef _THREAD_POOL_HPP_
#define _THREAD_POOL_HPP_

#include <future>
#include <functional>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

namespace tp {

    class Job 
    {
    
    private:
        std::promise<void> _workPromise;
        std::future<void> _workFuture;
        std::function<void(void)> _workFunc;
    public:
        inline Job(const std::function<void()> &f) : _workFunc(f){};
        Job& operator=(std::function<void()> &f);
        void execute();
        void wait();
        Job() = delete; 
        friend class Pool;
    };


    class Pool
    {

    public:
        Pool() = delete;
        Pool(unsigned nthreads);
        void submit(std::shared_ptr<Job> newJob);
        ~Pool();

    private:
        bool _stop;
        std::queue<std::shared_ptr<Job>> _jobQueue;
        std::condition_variable _jobQueue_cv; 
        std::mutex _jobQueue_mtx;
        std::vector<std::thread> _workerThreads;
    
    private:
        void workerLoop(unsigned id);
        static void joinThread(std::thread & th);
    };

    inline Job& Job::operator=(std::function<void()> &f)
    {
        this->_workFunc = f;
        return *this;
    };

    inline void Job::execute()
    {
        _workFunc();
        _workPromise.set_value();
    };

    inline void Job::wait()
    {
        _workFuture.wait();
    };

    inline Pool::Pool(unsigned nthreads) : 
    _stop(false)
    {
        for (auto i = 0; i< nthreads; ++i){
            _workerThreads.emplace_back(&Pool::workerLoop, this, i);
        }
    }

    inline Pool::~Pool()
    {
        _stop = true;
        _jobQueue_cv.notify_all();
        std::for_each(_workerThreads.begin(), _workerThreads.end(), joinThread);
    }

    inline void Pool::submit(std::shared_ptr<Job> newJob)
    {
        std::unique_lock<std::mutex> lock (_jobQueue_mtx);
        _jobQueue.push(newJob);
        _jobQueue_cv.notify_one();
        newJob->_workFuture = newJob->_workPromise.get_future();
    } 

    inline void Pool::workerLoop(unsigned id)
    {
        while(_stop == false)
        {
            std::unique_lock<std::mutex> lock (_jobQueue_mtx);
            _jobQueue_cv.wait(lock, [this]() {return _jobQueue.size() || _stop;});
            if (_jobQueue.size())
            {
                auto newJob = _jobQueue.front();
                _jobQueue.pop();
                lock.unlock();
                newJob->execute();
            }
            else return;
        }
    }

    inline void Pool::joinThread(std::thread & th)  {th.join();}

} /* namespace tp */

#endif /*_THREAD_POOL_HPP_ */