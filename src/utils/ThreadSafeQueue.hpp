#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Disable copying
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        m_condVar.notify_one();
    }

    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    void waitAndPop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condVar.wait(lock, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::queue<T> emptyQueue;
        std::swap(m_queue, emptyQueue);
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condVar;
};
