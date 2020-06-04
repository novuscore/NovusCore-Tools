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
#include <Utils/DebugHandler.h>
#include <Utils/ConcurrentQueue.h>
#include <Utils/ByteBuffer.h>
#include <functional>

class MPQFileJobBatch
{
private:
    struct FileJob
    {
        std::string filePath;
        std::function<void(std::shared_ptr<ByteBuffer>)> callback;
    };

public:
    void AddFileJob(std::string filePath, std::function<void(std::shared_ptr<ByteBuffer>)> callback); // Thread safe
    void RemoveDuplicates(); // This is NOT thread safe!
    void Process(); // This is NOT thread safe!

    size_t GetJobCount();

private:
    void ProcessThreadMain();

private:
    moodycamel::ConcurrentQueue<FileJob> _fileJobs;
    std::atomic<size_t> _numJobs;
};