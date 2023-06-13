#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj
{
public:
    ReserveProxyObj() = default;
    ReserveProxyObj(const size_t value)
        : capacity_to_reserve_(value)
    {
    }

    size_t GetCapacity() const
    {
        return capacity_to_reserve_;
    }

private:
    size_t capacity_to_reserve_ = 0;
};

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : items_(size), size_(size), capacity_(size)
    {
    }

    SimpleVector(size_t size, const Type &value)
        : items_(size), size_(size), capacity_(size)
    {
        if (size == 0)
        {
            return;
        }
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector &other)
        : items_(other.GetCapacity()), size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (*this == rhs)
        {
            return *this;
        }
        SimpleVector<Type> temp{rhs};
        swap(temp);
        return *this;
    }

    SimpleVector(SimpleVector &&other)
    {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(items_, other.items_);
    }

    SimpleVector &operator=(SimpleVector &&rhs)
    {
        if (&items_ == &(rhs.items_))
        {
            return *this;
        }
        SimpleVector<Type> temp{std::move(rhs)};
        swap(temp);
        return *this;
    }

    SimpleVector(ReserveProxyObj rhs)
    {
        Reserve(rhs.GetCapacity());
    };

    void swap(SimpleVector &other) noexcept
    {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void PushBack(const Type &item)
    {
        ResizeCurrentVector();
        items_[size_ - 1] = item;
    }

    void PushBack(Type &&item)
    {
        ResizeCurrentVector();
        items_[size_ - 1] = std::move(item);
    }

    void PopBack() noexcept
    {
        assert(!IsEmpty());
        --size_;
    }

    Iterator Insert(ConstIterator pos, const Type &value)
    {
        if (pos == cend())
        {
            PushBack(value);
            return (end() - 1);
        }
        size_t curr_index = pos - items_.Get();
        ResizeCurrentVector();
        Iterator it = std::copy_backward(&items_[curr_index], (end() - 1), end());
        --it;
        *it = value;
        return it;
    }

    Iterator Insert(ConstIterator pos, Type &&value)
    {
        assert(pos <= end());
        assert(pos >= begin());
        if (pos == cend())
        {
            PushBack(std::move(value));
            return (end() - 1);
        }
        size_t curr_index = pos - items_.Get();
        ResizeCurrentVector();
        Iterator it = std::move_backward(&items_[curr_index], (end() - 1), end());
        --it;
        *it = std::move(value);
        return it;
    }

    Iterator Erase(ConstIterator pos)
    {
        assert(pos <= end());
        assert(pos >= begin());
        size_t curr_index = pos - begin();
        if (pos < (end() - 1))
        {
            std::move(&items_[(curr_index + 1)], end(), &items_[curr_index]);
        }
        --size_;

        return &items_[curr_index];
    }

    size_t GetSize() const noexcept
    {
        return size_;
    }

    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    Type &operator[](size_t index) noexcept
    {
        assert(index < size_);
        return items_[index];
    }

    const Type &operator[](size_t index) const noexcept
    {
        assert(index < size_);
        return items_[index];
    }

    Type &At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    const Type &At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    void Resize(size_t new_size)
    {
        if (new_size > capacity_)
        {
            size_t new_capacity = std::max(new_size, (capacity_ == 0 ? 1 : capacity_ * 2));
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        else if (new_size > size_)
        {
            for (size_t i = size_; i < new_size; ++i)
            {
                items_[i] = Type{};
            }
        }
        size_ = new_size;
    }

    void Reserve(size_t new_capacity)
    {
        if (capacity_ < new_capacity)
        {
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            items_.swap(temp);

            capacity_ = new_capacity;
        }
    }

    void Clear() noexcept
    {
        size_ = 0;
    }

    Iterator begin() noexcept
    {
        return items_.Get();
    }

    Iterator end() noexcept
    {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept
    {
        return cbegin();
    }

    ConstIterator end() const noexcept
    {
        return cend();
    }

    ConstIterator cbegin() const noexcept
    {
        return items_.Get();
    }

    ConstIterator cend() const noexcept
    {
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
    void ResizeCurrentVector()
    {
        if (capacity_ <= size_)
        {
            Resize(size_ + 1);
        }
        else
        {
            ++size_;
        }
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}