/*
# MIT License

# Copyright(c) 2018-2020 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/
#pragma once
#include <NovusTypes.h>
#include <Utils/DebugHandler.h>
#include <thread>
#include <vector>
#include <atomic>
#include <shared_mutex>

class JobBatch;
class JobBatchRunner;

class JobBatchToken
{
public:
    bool IsFinished();
    void WaitUntilFinished();

private:
    JobBatchToken(JobBatchRunner* runner, u32 token);

    JobBatchRunner* _runner;
    u32 _batchID;

    friend class JobBatchRunner;
};

struct WorkerThread
{
    u32 threadID = 0;
    u32 getJobAttempts = 0;
    std::thread* thread;
};

class JobBatchRunner
{
public:
    void Start();
    void Stop();

    JobBatchToken AddBatch(JobBatch& batch); // This is NOT thread safe!

private:
    bool IsBatchRunning(u32 batchID);

    void ProcessThreadMain(WorkerThread thread);

    std::shared_mutex _runningBatchesMutex;
    std::vector<JobBatch*> _runningBatches;

    std::vector<bool> _isBatchRunning;

    std::atomic_bool _isRunning;
    std::atomic_bool _shouldStop;
    u32 _numThreads;
    std::vector<WorkerThread> _workerThreads;

    friend class JobBatchToken;
};