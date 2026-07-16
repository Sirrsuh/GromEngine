#pragma once

#include <Core/Types.h>
#include <Core/Container.h>
#include <Jobs/JobDecl.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace grom
{

struct JobEntry
{
    JobDecl          Decl;
    JobHandle        Handle;
    std::atomic<i32> Counter;

    JobEntry() : Handle(0) { Counter.store(0, std::memory_order_relaxed); }
    JobEntry(JobEntry&& other) noexcept
        : Handle(other.Handle)
    {
        std::memcpy(&Decl, &other.Decl, sizeof(JobDecl));
        Counter.store(other.Counter.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
    JobEntry& operator=(JobEntry&& other) noexcept
    {
        if (this != &other)
        {
            std::memcpy(&Decl, &other.Decl, sizeof(JobDecl));
            Handle = other.Handle;
            Counter.store(other.Counter.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }
    JobEntry(const JobEntry&) = delete;
    JobEntry& operator=(const JobEntry&) = delete;
};

class JobQueue
{
public:
    JobQueue();
    ~JobQueue();

    bool Push(JobEntry&& entry);
    bool Pop(JobEntry& entry);
    bool PeekCriticalCount(i32& count) const;
    bool IsEmpty() const;
    bool ContainsHandle(JobHandle handle) const;
    i32  GetHandleCount(JobHandle handle) const;

    static constexpr u32 RING_SIZE = 4096;

private:
    TStaticArray<JobEntry, RING_SIZE> m_entries;
    std::atomic<u32>                  m_head;
    std::atomic<u32>                  m_tail;

    friend class JobSystem;
    friend class WorkerThread;
};

class WorkerThread
{
public:
    WorkerThread(u32 index);
    ~WorkerThread();

    void Start();
    void Stop();
    void Join();
    void Wake();

    u32 GetIndex() const { return m_index; }

private:
    void Run();

    u32                            m_index;
    std::thread                    m_thread;
    std::mutex                     m_mutex;
    std::condition_variable        m_cv;
    bool                           m_running;
};

class JobSystem
{
public:
    static void Initialize(u32 numWorkerThreads = 0);
    static void Shutdown();

    static JobHandle AddJob(JobDecl& decl, EJobPriority priority = EJobPriority::Normal);
    static void WaitForJob(JobHandle handle);
    static void WaitForAll();
    static bool IsJobComplete(JobHandle handle);
    static u32 GetNumWorkers();
    static void* AllocateJobData(u32 size);
    static void RunPipelined();

    template<typename Func>
    static void ParallelFor(i32 start, i32 end, Func&& func, i32 stride = 1)
    {
        i32 count = end - start;
        if (count <= 0 || stride <= 0) return;

        u32 numWorkers = GetNumWorkers();
        if (numWorkers == 0) numWorkers = 1;

        i32 totalItems = (count + stride - 1) / stride;
        i32 batchSize = (totalItems + numWorkers - 1) / numWorkers;
        if (batchSize < 1) batchSize = 1;

        struct BatchJob
        {
            Func* FuncPtr;
            i32    BatchStart;
            i32    BatchEnd;
            i32    StartVal;
            i32    Stride;
        };

        TArray<JobHandle> handles;
        handles.Reserve(numWorkers);

        for (u32 i = 0; i < numWorkers; ++i)
        {
            i32 batchStart = static_cast<i32>(i) * batchSize;
            if (batchStart >= totalItems) break;

            i32 batchEnd = batchStart + batchSize;
            if (batchEnd > totalItems) batchEnd = totalItems;

            BatchJob* jobData = static_cast<BatchJob*>(AllocateJobData(sizeof(BatchJob)));
            jobData->FuncPtr = &func;
            jobData->BatchStart = batchStart;
            jobData->BatchEnd = batchEnd;
            jobData->StartVal = start;
            jobData->Stride = stride;

            JobDecl decl;
            decl.Function = [](void* data)
            {
                BatchJob* bj = static_cast<BatchJob*>(data);
                for (i32 idx = bj->BatchStart; idx < bj->BatchEnd; ++idx)
                {
                    i32 val = bj->StartVal + idx * bj->Stride;
                    (*bj->FuncPtr)(val);
                }
            };
            decl.Data = jobData;
            decl.Size = sizeof(BatchJob);
            decl.Name = GString("ParallelFor");

            handles.Add(AddJob(decl, EJobPriority::Normal));
        }

        for (u32 i = 0; i < handles.Size(); ++i)
        {
            WaitForJob(handles[i]);
        }
    }

private:
    static bool TryPopJob(JobEntry& entry);
    static bool TryStealJob(JobEntry& entry);

    static constexpr u32 MAX_WORKERS = 64;

    static JobQueue            s_queues[4];
    static WorkerThread*       s_workers[MAX_WORKERS];
    static u32                 s_numWorkers;
    static bool                s_initialized;
    static std::atomic<u64>    s_handleCounter;

    friend class WorkerThread;
};

}
