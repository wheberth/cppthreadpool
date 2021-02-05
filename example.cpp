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

#include "ThreadPool.hpp"
#include <iostream>

void usefulFunction(int in, int &out)
{
    int sum = 0;
    for (int i = 0 ; i <= in; ++i)
        sum += i;
    out = sum;
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 512));
    std::cout << " Input : " << in << " Output : " << out << "\n";
}

int NUM_JOBS = 16;
int NUM_THREADS = 8;
int main()
{
    std::vector<int> results (NUM_JOBS);
    tp::Pool threadPool (NUM_THREADS);
    std::vector<std::shared_ptr<tp::Job>> jobs;
    for (int i = 0; i < NUM_JOBS; ++i)
    {
        auto jobptr = std::make_shared<tp::Job>(std::bind(
            usefulFunction, i,          // function name and input parameter
            std::ref(results[i])        // output parameter (by ref)
        ));
        jobs.emplace_back(jobptr);      // keep job to retrieve result later
        threadPool.submit(jobs.back()); // submit job
    }
    std::cout << "All josbs submmited\n"; 

    // Wait for the jobs to finish
    for(auto & j : jobs) j->wait();
    std::cout << "All josbs finished\n";

    // Print the results
    std::cout << "{" ;
    for (auto & res  : results) std::cout << res << ", ";
    std::cout << "\b\b}\n" ; 
    
    return 0;
}