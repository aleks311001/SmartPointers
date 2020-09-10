#include "SharedPtr.h"

template<typename T>
SharedPtr<T>::SharedPtr(): counter_(nullptr), ptr_(nullptr) {}
template<typename T>
template<typename U>
SharedPtr<T>::SharedPtr(U* ptr)
        : counter_(new Counter(static_cast<T*>(ptr))), ptr_(static_cast<T*>(ptr)) {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<U>, U>) {
        ptr->weak_ptr_ = WeakPtr<T>(ptr);
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(Counter* counter, T* ptr): counter_(counter), ptr_(ptr) {
    ++counter->shared_count;
}

template<typename T>
template<typename Alloc, typename... Args>
SharedPtr<T>::SharedPtr(const Alloc&, Args&& ... args) {
    Alloc alloc;
    std::allocator_traits<Alloc> traitsAlloc;

    typename std::allocator_traits<Alloc>::template rebind_alloc<bool> allocBool;
    bool* doublePtr = allocBool.allocate(sizeof(T) + sizeof(CounterForAllocateShared<Alloc>));

    typename std::allocator_traits<Alloc>::template rebind_alloc<CounterForAllocateShared<Alloc>> allocCounter;
    typename std::allocator_traits<Alloc>::template rebind_traits<CounterForAllocateShared<Alloc>> traitsAllocCounter;
    auto* counter = reinterpret_cast<CounterForAllocateShared<Alloc>*>(doublePtr);
    ptr_ = reinterpret_cast<T*>(doublePtr + sizeof(CounterForAllocateShared<Alloc>));

    traitsAlloc.construct(alloc, ptr_, std::forward<Args>(args)...);
    traitsAllocCounter.construct(allocCounter, counter, ptr_);

    counter_ = dynamic_cast<Counter*>(counter);

    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
        ptr_->weak_ptr_ = WeakPtr<T>(*this);
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other): counter_(other.counter_), ptr_(other.ptr_) {
    if (counter_ != nullptr) {
        ++counter_->shared_count;
    }
}
template<typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other): counter_(other.counter_), ptr_(other.ptr_) {
    other.counter_ = nullptr;
    other.ptr_ = nullptr;
}
template<typename T>
template<typename U>
SharedPtr<T>::SharedPtr(const SharedPtr<U>& other):
        counter_(static_cast<Counter*>(other.counter_)), ptr_(static_cast<T*>(other.ptr_)) {
    ++counter_->shared_count;
}
template<typename T>
template<typename Other>
SharedPtr<T>::SharedPtr(SharedPtr<Other>&& other):
        counter_(static_cast<Counter*>(other.counter_)), ptr_(static_cast<T*>(other.ptr_)) {
    other.counter_ = nullptr;
    other.ptr_ = nullptr;
}

template<typename T>
template<typename Other>
SharedPtr<T>& SharedPtr<T>::operator=(Other&& other) {
    SharedPtr<T>(std::forward<Other>(other)).swap(*this);
    return *this;
}

template<typename T>
SharedPtr<T>::~SharedPtr() {
    if (counter_ != nullptr) {
        if (counter_->shared_count == 1) {
            ptr_->~T();
            if (counter_->weak_count == 0) {
                counter_->destroy();
            } else {
                --counter_->shared_count;
            }
        } else {
            --counter_->shared_count;
        }
    }
}

template<typename T>
long SharedPtr<T>::use_count() const noexcept {
    return counter_->shared_count;
}

template<typename T>
void SharedPtr<T>::reset() {
    SharedPtr<T>().swap(*this);
}
template<typename T>
template<typename U>
void SharedPtr<T>::reset(U* ptr) {
    SharedPtr<T>(ptr).swap(*this);
}
template<typename T>
void SharedPtr<T>::swap(SharedPtr& other) noexcept {
    std::swap(counter_, other.counter_);
    std::swap(ptr_, other.ptr_);
}

template<typename T>
T& SharedPtr<T>::operator*() const noexcept {
    return *get();
}
template<typename T>
T* SharedPtr<T>::operator->() const noexcept {
    return get();
}
template<typename T>
T* SharedPtr<T>::get() const noexcept {
    return ptr_;
}
