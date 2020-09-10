#ifndef SMARTPOINTERS_SHAREDPTR_H
#define SMARTPOINTERS_SHAREDPTR_H

#include <iostream>
#include <memory>

template<typename Enabled>
struct EnableSharedFromThis;

struct Counter {
    void* ptr;
    size_t shared_count;
    size_t weak_count;

    template<typename T>
    Counter(T* ptr, size_t shared_count = 1, size_t weak_count = 0)
            : ptr(static_cast<void*>(ptr)),
              shared_count(shared_count),
              weak_count(weak_count) {}
    virtual ~Counter() = default;

    virtual void destroy() {
        ::operator delete(ptr);
        delete this;
    }
};

template<typename Alloc>
struct CounterForAllocateShared: public Counter {
    Alloc alloc;
    const int size;

    template<typename T>
    CounterForAllocateShared(T* ptr, size_t shared_count = 1, size_t weak_count = 0)
            : Counter(ptr, shared_count, weak_count),
              alloc(),
              size(sizeof(T)) {}
    ~CounterForAllocateShared() override = default;

    void destroy() override {
        typename std::allocator_traits<Alloc>::template rebind_alloc<bool>().
                deallocate(reinterpret_cast<bool*>(this), size + sizeof(CounterForAllocateShared));
    }
};

template<typename T>
class SharedPtr {
public:
    template<typename U>
    friend class WeakPtr;
    template<typename U>
    friend class SharedPtr;

    template<typename U, typename ...Args>
    friend SharedPtr<U> makeShared(Args&&... args);
    template<typename U, typename Alloc, typename... Args >
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

    SharedPtr();
    template<typename U>
    explicit SharedPtr(U* ptr);

    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other);

    template<typename U>
    SharedPtr(const SharedPtr<U>& other);
    template<typename U>
    SharedPtr(SharedPtr<U>&& other);

    template<typename Other>
    SharedPtr<T>& operator=(Other&& other);

    ~SharedPtr();

    long use_count() const noexcept;

    template<typename U>
    void reset(U* ptr);
    void reset();
    void swap(SharedPtr& other) noexcept;

    T& operator*() const noexcept;
    T* operator->() const noexcept;
    T* get() const noexcept;

private:
    SharedPtr(Counter* counter, T* ptr);

    template<typename Alloc, typename ...Args>
    SharedPtr(const Alloc& alloc, Args&& ...args);

    Counter* counter_;
    T* ptr_;
};

template<typename T, typename ...Args>
SharedPtr<T> makeShared(Args&& ...args) {
    return SharedPtr<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T, typename Alloc, typename ...Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return SharedPtr<T>(alloc, std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {
public:
    template<typename U>
    friend class WeakPtr;

    WeakPtr();
    template<typename U>
    WeakPtr(const SharedPtr<U>& sharedPtr);

    WeakPtr(const WeakPtr& other);
    WeakPtr(WeakPtr&& other);

    template<typename U>
    WeakPtr(const WeakPtr<U>& other);
    template<typename U>
    WeakPtr(WeakPtr<U>&& other);

    template<typename U>
    WeakPtr<T>& operator=(const SharedPtr<U>& other);
    template<typename Other>
    WeakPtr<T>& operator=(Other&& other);

    ~WeakPtr();

    bool expired() const;
    SharedPtr<T> lock() const;
    long use_count() const;
    void swap(WeakPtr& other);
private:
    Counter* counter_;
    T* ptr_;
};

template<typename T>
struct EnableSharedFromThis {
private:
    WeakPtr<T> weak_ptr_ = WeakPtr<T>();

public:
    template<typename U>
    friend class SharedPtr;

    SharedPtr<T> shared_from_this() {
        if (weak_ptr_.use_count() == 0) {
            throw std::bad_weak_ptr();
        }
        return weak_ptr_.lock();
    }
};

#endif //SMARTPOINTERS_SHAREDPTR_H
