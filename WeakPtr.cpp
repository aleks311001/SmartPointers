#include "SharedPtr.h"

template<typename T>
WeakPtr<T>::WeakPtr(): counter_(nullptr), ptr_(nullptr) {}
template<typename T>
template<typename U>
WeakPtr<T>::WeakPtr(const SharedPtr<U>& sharedPtr):
        counter_(static_cast<Counter*>(sharedPtr.counter_)), ptr_(static_cast<T*>(sharedPtr.ptr_)) {
    ++counter_->weak_count;
}

template<typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other): counter_(other.counter_), ptr_(other.ptr_) {
    ++counter_->weak_count;
}
template<typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& other): counter_(other.counter_), ptr_(other.ptr_) {
    other.counter_ = nullptr;
    other.ptr_ = nullptr;
}

template<typename T>
template<typename U>
WeakPtr<T>::WeakPtr(const WeakPtr<U>& other):
        counter_(static_cast<Counter*>(static_cast<void*>(other.counter_))), ptr_(static_cast<T*>(other.ptr_)) {
    ++counter_->weak_count;
}
template<typename T>
template<typename U>
WeakPtr<T>::WeakPtr(WeakPtr<U>&& other):
        counter_(static_cast<Counter*>(static_cast<void*>(other.counter_))), ptr_(static_cast<T*>(other.ptr_)) {
    other.counter_ = nullptr;
    other.ptr_ = nullptr;
}


template<typename T>
template<typename U>
WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<U>& other) {
    WeakPtr<T>(other).swap(*this);
    return *this;
}
template<typename T>
template<typename Other>
WeakPtr<T>& WeakPtr<T>::operator=(Other&& other) {
    WeakPtr<T>(std::forward<Other>(other)).swap(*this);
    return *this;
}

template<typename T>
WeakPtr<T>::~WeakPtr() {
    if (counter_ != nullptr) {
        --counter_->weak_count;
        if (counter_->weak_count == 0 && expired()) {
            counter_->destroy();
        }
    }
}

template<typename T>
bool WeakPtr<T>::expired() const {
    return use_count() == 0;
}

template<typename T>
SharedPtr<T> WeakPtr<T>::lock() const {
    return SharedPtr<T>(counter_, ptr_);
}

template<typename T>
long WeakPtr<T>::use_count() const {
    if (counter_ != nullptr) {
        return counter_->shared_count;
    }
    return 0;
}

template<typename T>
void WeakPtr<T>::swap(WeakPtr& other) {
    std::swap(counter_, other.counter_);
    std::swap(ptr_, other.ptr_);
}
