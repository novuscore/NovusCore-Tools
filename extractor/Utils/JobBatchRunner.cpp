#include "JobBatchRunner.h"
#include "JobBatch.h"
#include <cassert>
#include <mutex>

#include <tracy/Tracy.hpp>

bool JobBatchToken::IsFinished()
{
    return !_runner->IsBatchRunning(_batchID);
}

void JobBatchToken::WaitUntilFinished()
{
    ZoneScoped;

    while (!IsFinished())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 checks per second
    }
}

JobBatchToken::JobBatchToken(JobBatchRunner* runner, u32 batchID)
    : _runner(runner)
    , _batchID(batchID)
{

}

void JobBatchRunner::Start()
{
    ZoneScoped;

    assert(!_isRunning);

    _numThreads = 1;
    if (std::thread::hardware_concurrency() > 2)
    {
        _numThreads = std::thread::hardware_concurrency() - 1;
    }

    _isRunning = true;
    _shouldStop = false;

    // Start processing threads
    for (u32 i = 0; i < _numThreads; i++)
    {
        WorkerThread& workerThread = _workerThreads.emplace_back();

        workerThread.threadID = i;
        workerThread.thread = new std::thread([this, workerThread]()
        {
            ProcessThreadMain(workerThread);
        });
    }
}

void JobBatchRunner::Stop()
{
    ZoneScoped;
    _shouldStop = true;

    {
        ZoneScopedN("WaitForThreads");

        // Wait for processing to finish
        for (u32 i = 0; i < _numThreads; i++)
        {
            _workerThreads[i].thread->join();
            delete _workerThreads[i].thread;
        }
    }

    _isRunning = false;
}

JobBatchToken JobBatchRunner::AddBatch(JobBatch& batch)
{
    ZoneScoped;
    std::unique_lock lock(_runningBatchesMutex);

    assert(_isBatchRunning.size() < std::numeric_limits<u32>::max());

    u32 batchID = static_cast<u32>(_isBatchRunning.size());

    batch.batchId = batchID;
    _isBatchRunning.push_back(true);
    _runningBatches.push_back(&batch);

    return JobBatchToken(this, batchID);
}

bool JobBatchRunner::IsBatchRunning(u32 batchID)
{
    assert(batchID < _isBatchRunning.size());

    return _isBatchRunning[batchID];
}

void JobBatchRunner::ProcessThreadMain(WorkerThread thread)
{
    ZoneScoped;

    char threadName[16];
    snprintf(threadName, 16, "WorkerThread %u", thread.threadID);
    tracy::SetThreadName(threadName);

    Job job;

    while (!_shouldStop)
    {
        {
            ZoneScopedN("WaitForJobs");

            // Sleep/Yield to not take up all CPU
            if (thread.getJobAttempts > 25 && thread.getJobAttempts < 100) // Between try 25 and 100, yield
            {
                std::this_thread::yield();
            }
            else if (thread.getJobAttempts < 1000) // Between try 100 and 1000, 100 checks per second
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else // Over 1000 attempts, 10 checks per second
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // Check if there are any running batches
        _runningBatchesMutex.lock_shared();
        if (_runningBatches.empty())
        {
            _runningBatchesMutex.unlock_shared();
            thread.getJobAttempts++;
            continue;
        }

        bool didUnlock = false;
        for (int i = 0; i < _runningBatches.size(); i++)
        {
            JobBatch& batch = *_runningBatches[i];

            // Try to dequeue from the batch
            Job job;
            if (!batch._jobs.try_dequeue(job))
            {
                // This check ensures that if we failed to dequeue, that it was because another thread took the job.
                if (batch._jobs.size_approx() != 0)
                {
                    NC_LOG_FATAL("JobBatchRunner::ProcessThreadMain: Failed to dequeue job from batch with remaining jobs waiting");
                }

                // It is possible to hit this, if the last job was dequeued by another thread at the same time.
                continue;
            }

            _runningBatchesMutex.unlock_shared();
            didUnlock = true;

            ZoneScopedN("ExecuteJobs");

            // If we succeeded, reset getJobAttempts
            thread.getJobAttempts = 0;

            // Then do work
            job.callback();

            if (--batch._numJobs == 0)
            {
                // Remove empty batch from _runningBatches
                std::unique_lock lock(_runningBatchesMutex);
                _isBatchRunning[batch.batchId] = false;

                auto itr = std::find_if(_runningBatches.begin(), _runningBatches.end(), [&currBatch = batch] (const JobBatch* b) -> bool { return currBatch.GetId() == b->GetId(); });
                _runningBatches.erase(itr);
            }

            break;
        }

        if (!didUnlock)
        {
            _runningBatchesMutex.unlock_shared();
        }
    }

    // Finish any running batches before we exit
    while (true)
    {
        _runningBatchesMutex.lock_shared();
        if (_runningBatches.empty())
        {
            _runningBatchesMutex.unlock_shared();
            return; // No more running batches, just return to exit thread
        }
        JobBatch& batch = *_runningBatches[0];
        _runningBatchesMutex.unlock_shared();

        // Finish the batch
        Job job;
        while (batch._jobs.try_dequeue(job))
        {
            job.callback();
            batch._numJobs--;
        }

        // Remove empty batch from _runningBatches
        std::unique_lock lock(_runningBatchesMutex);
        if (_runningBatches.size() > 0 && _runningBatches[0]->GetJobCount() == 0)
            _runningBatches.erase(_runningBatches.begin());
    }

    {
        ZoneScopedN("Exited Worker Thread");
    }
}


