#pragma once

namespace synchronization {

template <typename S>
concept LockableProtocol = requires(S& s) {
    {s.acquire()};
    {s.release()};
};

template <LockableProtocol Lockable>
requires(!std::movable<Lockable> && !std::copyable<Lockable>) class Lock {
  public:
    Lock(Lockable& s) : lockable(s) { lockable.acquire(); }

    ~Lock() { lockable.release(); }

  private:
    Lockable& lockable;
};

}  // namespace synchronization