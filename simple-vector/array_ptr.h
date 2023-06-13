#pragma once

#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr
{
public:

    ArrayPtr() = default;

    explicit ArrayPtr(size_t size)
    {
        size_t tmp = 0;
        if (size > tmp)
        {
            Type *tmp_ptr = new Type[size]{};
            std::swap(raw_ptr_, tmp_ptr);
        }
    }

    explicit ArrayPtr(Type *raw_ptr) noexcept
    {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr &) = default;

    ArrayPtr &operator=(const ArrayPtr &) = default;

    ArrayPtr(ArrayPtr &&other)
    {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    };

    ArrayPtr &operator=(ArrayPtr &&other)
    {
        if (raw_ptr_ == other.raw_ptr_)
        {
            return *this;
        }
        swap(other);
        return *this;
    };

    ~ArrayPtr()
    {
        delete[] raw_ptr_;
    }

    [[nodiscard]] Type *Release() noexcept
    {
        Type *tmp = raw_ptr_;
        raw_ptr_ = nullptr;
        return tmp;
    }

    Type &operator[](size_t index) noexcept
    {
        return *(raw_ptr_ + index);
    }

    const Type &operator[](size_t index) const noexcept
    {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const
    {
        return !(raw_ptr_ == nullptr);
    }

    Type *Get() const noexcept
    {
        return raw_ptr_;
    }

    void swap(ArrayPtr &other) noexcept
    {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type *raw_ptr_ = nullptr;
};