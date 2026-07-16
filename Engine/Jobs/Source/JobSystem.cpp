#include <windows.h>
#ifdef AddJob
#pragma push_macro("AddJob")
#undef AddJob
#endif

#include <Jobs/JobSystem.h>
#include <Jobs/JobParallelFor.h>
#include <Core/Types.h>
#include <Core/Assert.h>
#include <cstdlib>
#include <cstring>

namespace grom
{

JobQueue            JobSystem::s_queues[4];
WorkerThread*       JobSystem::s_workers[MAX_WORKERS];
u32                 JobSystem::s_numWorkers = 0;
bool                JobSystem::s_initialized = false;
std::atomic<u64>    JobSystem::s_handleCounter{ 1 };

JobQueue::JobQueue()
    : m_head(0)
    , m_tail(0)
{
}

JobQueue::~JobQueue()
{
}

bool JobQueue::Push(JobEntry&& entry)
{
    u32 tail = m_tail.load(std::memory_order_relaxed);
    u32 next = (tail + 1) & (RING_SIZE - 1);

    if (next == m_head.load(std::memory_order_acquire))
    {
        return false;
    }

    m_entries[tail] = static_cast<JobEntry&&>(entry);
    std::atomic_thread_fence(std::memory_order_release);
    m_tail.store(next, std::memory_order_release);
    return true;
}

bool JobQueue::Pop(JobEntry& entry)
{
    u32 head = m_head.load(std::memory_order_relaxed);

    if (head == m_tail.load(std::memory_order_acquire))
    {
        return false;
    }

    std::memcpy(&entry, &m_entries[head], sizeof(JobEntry));
    std::atomic_thread_fence(std::memory_order_release);
    m_head.store((head + 1) & (RING_SIZE - 1), std::memory_order_release);
    return true;
}

bool JobQueue::PeekCriticalCount(i32& count) const
{
    u32 head = m_head.load(std::memory_order_acquire);
    u32 tail = m_tail.load(std::memory_order_acquire);

    if (head == tail)
    {
        count = 0;
        return false;
    }

    count = m_entries[head].Counter.load(std::memory_order_acquire);
    return true;
}

bool JobQueue::IsEmpty() const
{
    return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
}

bool JobQueue::ContainsHandle(JobHandle handle) const
{
    u32 head = m_head.load(std::memory_order_acquire);
    u32 tail = m_tail.load(std::memory_order_acquire);

    u32 idx = head;
    while (idx != tail)
    {
        if (m_entries[idx].Handle == handle)
        {
            return true;
        }
        idx = (idx + 1) & (RING_SIZE - 1);
    }
    return false;
}

i32 JobQueue::GetHandleCount(JobHandle handle) const
{
    u32 head = m_head.load(std::memory_order_acquire);
    u32 tail = m_tail.load(std::memory_order_acquire);

    u32 idx = head;
    while (idx != tail)
    {
        if (m_entries[idx].Handle == handle)
        {
            return m_entries[idx].Counter.load(std::memory_order_acquire);
        }
        idx = (idx + 1) & (RING_SIZE - 1);
    }
    return 0;
}

WorkerThread::WorkerThread(u32 index)
    : m_index(index)
    , m_running(false)
{
}

WorkerThread::~WorkerThread()
{
    Stop();
    Join();
}

void WorkerThread::Start()
{
    m_running = true;
    m_thread = std::thread(&WorkerThread::Run, this);
}

void WorkerThread::Stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }
    m_cv.notify_one();
}

void WorkerThread::Join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void WorkerThread::Wake()
{
    m_cv.notify_one();
}

void WorkerThread::Run()
{
    wchar_t threadName[64];
    swprintf_s(threadName, L"GromWorker_%u", m_index);
    SetThreadDescription(GetCurrentThread(), threadName);

    while (m_running)
    {
        JobEntry entry;

        if (JobSystem::s_queues[static_cast<u32>(EJobPriority::Critical)].Pop(entry) ||
            JobSystem::s_queues[static_cast<u32>(EJobPriority::High)].Pop(entry) ||
            JobSystem::s_queues[static_cast<u32>(EJobPriority::Normal)].Pop(entry) ||
            JobSystem::s_queues[static_cast<u32>(EJobPriority::Low)].Pop(entry))
        {
            if (entry.Decl.Function)
            {
                entry.Decl.Function(entry.Decl.Data);
            }

            i32 prev = entry.Counter.fetch_sub(1, std::memory_order_acq_rel);
            GROM_ASSERT(prev > 0, "Job counter underflow");
        }
        else
        {
            bool stole = false;
            for (u32 i = 0; i < JobSystem::s_numWorkers; ++i)
            {
                if (i == m_index) continue;

                const u32 queueOrder[4] = { 3, 2, 1, 0 };
                for (u32 q = 0; q < 4; ++q)
                {
                    if (JobSystem::s_queues[queueOrder[q]].Pop(entry))
                    {
                        if (entry.Decl.Function)
                        {
                            entry.Decl.Function(entry.Decl.Data);
                        }

                        i32 prev = entry.Counter.fetch_sub(1, std::memory_order_acq_rel);
                        GROM_ASSERT(prev > 0, "Job counter underflow");
                        stole = true;
                        break;
                    }
                }
                if (stole) break;
            }

            if (!stole)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait_for(lock, std::chrono::milliseconds(1), [this] { return !m_running; });
            }
        }
    }
}

void JobSystem::Initialize(u32 numWorkerThreads)
{
    if (s_initialized) return;

    if (numWorkerThreads == 0)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numWorkerThreads = sysInfo.dwNumberOfProcessors;
    }

    if (numWorkerThreads > MAX_WORKERS)
    {
        numWorkerThreads = MAX_WORKERS;
    }

    s_numWorkers = numWorkerThreads;
    s_handleCounter.store(1, std::memory_order_relaxed);

    for (u32 i = 0; i < s_numWorkers; ++i)
    {
        s_workers[i] = new WorkerThread(i);
        s_workers[i]->Start();
    }

    s_initialized = true;
}

void JobSystem::Shutdown()
{
    if (!s_initialized) return;

    for (u32 i = 0; i < s_numWorkers; ++i)
    {
        if (s_workers[i])
        {
            s_workers[i]->Stop();
            s_workers[i]->Join();
            delete s_workers[i];
            s_workers[i] = nullptr;
        }
    }

    s_numWorkers = 0;
    s_initialized = false;
}

JobHandle JobSystem::AddJob(JobDecl& decl, EJobPriority priority)
{
    GROM_ASSERT(s_initialized, "JobSystem not initialized");

    JobHandle handle = s_handleCounter.fetch_add(1, std::memory_order_relaxed);

    JobEntry entry;
    std::memcpy(&entry.Decl, &decl, sizeof(JobDecl));
    entry.Handle = handle;
    entry.Counter.store(1, std::memory_order_relaxed);

    u32 queueIdx = static_cast<u32>(priority);
    if (queueIdx >= 4) queueIdx = 1;

    bool pushed = s_queues[queueIdx].Push(static_cast<JobEntry&&>(entry));

    if (!pushed)
    {
        for (u32 attempt = 0; attempt < 64; ++attempt)
        {
            for (u32 q = 0; q < 4; ++q)
            {
                if (s_queues[q].Push(static_cast<JobEntry&&>(entry)))
                {
                    pushed = true;
                    break;
                }
            }
            if (pushed) break;
            std::this_thread::yield();
        }
    }

    GROM_ASSERT(pushed, "Failed to push job to any queue");

    for (u32 i = 0; i < s_numWorkers; ++i)
    {
        if (s_workers[i])
        {
            s_workers[i]->Wake();
        }
    }

    return handle;
}

void JobSystem::WaitForJob(JobHandle handle)
{
    if (handle == 0) return;

    RunPipelined();

    while (!IsJobComplete(handle))
    {
        JobEntry entry;
        if (TryPopJob(entry))
        {
            if (entry.Decl.Function)
            {
                entry.Decl.Function(entry.Decl.Data);
            }

            i32 prev = entry.Counter.fetch_sub(1, std::memory_order_acq_rel);
            GROM_ASSERT(prev > 0, "Job counter underflow");
        }
        else
        {
            std::this_thread::yield();
        }
    }
}

void JobSystem::WaitForAll()
{
    RunPipelined();

    bool allEmpty;
    do
    {
        allEmpty = true;
        for (u32 q = 0; q < 4; ++q)
        {
            if (!s_queues[q].IsEmpty())
            {
                allEmpty = false;
                break;
            }
        }

        if (!allEmpty)
        {
            JobEntry entry;
            if (TryPopJob(entry))
            {
                if (entry.Decl.Function)
                {
                    entry.Decl.Function(entry.Decl.Data);
                }

                i32 prev = entry.Counter.fetch_sub(1, std::memory_order_acq_rel);
                GROM_ASSERT(prev > 0, "Job counter underflow");
            }
            else
            {
                std::this_thread::yield();
            }
        }
    } while (!allEmpty);
}

bool JobSystem::IsJobComplete(JobHandle handle)
{
    for (u32 q = 0; q < 4; ++q)
    {
        if (s_queues[q].ContainsHandle(handle))
        {
            i32 cnt = s_queues[q].GetHandleCount(handle);
            if (cnt > 0) return false;
        }
    }
    return true;
}

u32 JobSystem::GetNumWorkers()
{
    return s_numWorkers;
}

void* JobSystem::AllocateJobData(u32 size)
{
    return malloc(size);
}

void JobSystem::RunPipelined()
{
    JobEntry entry;
    while (TryPopJob(entry))
    {
        if (entry.Decl.Function)
        {
            entry.Decl.Function(entry.Decl.Data);
        }

        i32 prev = entry.Counter.fetch_sub(1, std::memory_order_acq_rel);
        GROM_ASSERT(prev > 0, "Job counter underflow");
    }
}

bool JobSystem::TryPopJob(JobEntry& entry)
{
    if (s_queues[static_cast<u32>(EJobPriority::Critical)].Pop(entry)) return true;
    if (s_queues[static_cast<u32>(EJobPriority::High)].Pop(entry)) return true;
    if (s_queues[static_cast<u32>(EJobPriority::Normal)].Pop(entry)) return true;
    if (s_queues[static_cast<u32>(EJobPriority::Low)].Pop(entry)) return true;
    return false;
}

bool JobSystem::TryStealJob(JobEntry& entry)
{
    for (u32 i = 0; i < s_numWorkers; ++i)
    {
        const u32 queueOrder[4] = { 3, 2, 1, 0 };
        for (u32 q = 0; q < 4; ++q)
        {
            if (s_queues[queueOrder[q]].Pop(entry)) return true;
        }
    }
    return false;
}

}
