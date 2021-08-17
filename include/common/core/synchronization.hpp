#pragma once

namespace synchronization {

template <typename S>
concept SemaphoreProtocol = requires(S& s) {
    {s.acquire()};
    {s.release()};
    { s.get_count() } -> std::convertible_to<int>;
};

template <SemaphoreProtocol Semaphore>
requires(!std::movable<Semaphore> && !std::copyable<Semaphore>) class Lock {
  public:
    Lock(Semaphore& s) : semaphore(s) { semaphore.acquire(); }

    ~Lock() { semaphore.release(); }

  private:
    Semaphore& semaphore;
};

}  // namespace synchronization