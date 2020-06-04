#include "MPQFileJobBatch.h"
#include "MPQLoader.h"

#include "../Utils/ServiceLocator.h"

#include <Utils/StringUtils.h>

void MPQFileJobBatch::AddFileJob(std::string filePath, std::function<void(std::shared_ptr<ByteBuffer>)> callback)
{
    FileJob fileJob;
    fileJob.filePath = filePath;
    fileJob.callback = callback;

    _fileJobs.enqueue(fileJob);
    _numJobs++;
}

void MPQFileJobBatch::RemoveDuplicates()
{
    // First we create some temporary storage to do the sorting with
    robin_hood::unordered_map<u32, bool> alreadyAddedJobs; // u32 is hashed input filepath, bool is just if it's added yet or not
    std::vector<FileJob> nonDuplicateJobs;
    nonDuplicateJobs.reserve(512);

    // Dequeue all our added filejobs
    FileJob fileJob;
    while (_fileJobs.try_dequeue(fileJob))
    {
        // Get their hashed path
        u32 hashedPath = StringUtils::fnv1a_32(fileJob.filePath.c_str(), fileJob.filePath.size());

        // Check if we added this filepath already
        if (alreadyAddedJobs[hashedPath] == true)
            continue;

        // Else we add it
        alreadyAddedJobs[hashedPath] = true;
        nonDuplicateJobs.push_back(fileJob);
    }

    _numJobs = 0;
    // Now we simply need to loop through our std::vector and push them back into the queue
    for (FileJob& job : nonDuplicateJobs)
    {
        _fileJobs.enqueue(job);
        _numJobs++;
    }
}

void MPQFileJobBatch::Process()
{
    size_t numThreads = 1;
    if (std::thread::hardware_concurrency() > 2)
    {
        numThreads = std::thread::hardware_concurrency() - 1;
    }

    std::thread* threads = new std::thread[numThreads];

    // Start processing threads
    for (int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread([this]() {
            ProcessThreadMain();
        });
    }

    // Wait for processing to finish
    for (int i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
}

size_t MPQFileJobBatch::GetJobCount()
{
    return _numJobs;
}

void MPQFileJobBatch::ProcessThreadMain()
{
    std::shared_ptr<MPQLoader> mpqLoader = ServiceLocator::GetMPQLoader();

    FileJob fileJob;

    while (_fileJobs.try_dequeue(fileJob))
    {
        std::shared_ptr<ByteBuffer> byteBuffer = mpqLoader->GetFile(fileJob.filePath);

        if (byteBuffer)
        {
            fileJob.callback(byteBuffer);
        }
        _numJobs--;
    }
}
