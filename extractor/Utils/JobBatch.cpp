#include "JobBatch.h"
#include <robin_hood.h>

void JobBatch::AddJob(u32 nameHash, std::function<void()> callback)
{
    Job job;
    job.nameHash = nameHash;
    job.callback = callback;

    _jobs.enqueue(job);
    _numJobs++;
}

void JobBatch::RemoveDuplicates()
{
    // First we create some temporary storage to do the sorting with
    robin_hood::unordered_map<u32, bool> alreadyAddedJobs; // u32 is nameHash, bool is just if it's added yet or not
    std::vector<Job> nonDuplicateJobs;
    nonDuplicateJobs.reserve(_numJobs); // Reserve for worst case

    // Dequeue all our added jobs
    Job job;
    while (_jobs.try_dequeue(job))
    {
        // Check if we added this filepath already
        if (alreadyAddedJobs[job.nameHash] == true)
            continue;

        // Else we add it
        alreadyAddedJobs[job.nameHash] = true;
        nonDuplicateJobs.push_back(job);
    }

    _numJobs = 0;
    // Now we simply need to loop through our std::vector and push them back into the queue
    for (Job& job : nonDuplicateJobs)
    {
        _jobs.enqueue(job);
        _numJobs++;
    }
}

size_t JobBatch::GetJobCount()
{
    return _numJobs;
}
