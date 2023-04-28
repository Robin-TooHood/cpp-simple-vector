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

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : items_(size), size_(size), capacity_(size)
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value)
        : items_(size), size_(size), capacity_(size)
    {
        if (size == 0)
        {
            return;
        }
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    // Конструктор копирования
    SimpleVector(const SimpleVector &other)
        : items_(other.GetCapacity()), size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (&items_ == &(rhs.items_))
        {
            return *this;
        }
        SimpleVector<Type> temp{rhs};
        swap(temp);
        return *this;
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector &&other)
    {
        other.capacity_ = std::exchange(capacity_, other.capacity_);
        other.size_ = std::exchange(size_, other.size_);
        other.items_ = std::exchange(items_, other.items_);
    }

    // Оператор перемещения
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

    // Резервирование вместимости
    SimpleVector(ReserveProxyObj rhs)
    {
        Reserve(rhs.GetCapacity());
    };

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept
    {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Вставляет элемент в конец вектора
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

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept
    {
        if (!IsEmpty())
        {
            --size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
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

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {
        size_t curr_index = pos - begin();
        if (pos < (end() - 1))
        {
            std::move(&items_[(curr_index + 1)], end(), &items_[curr_index]);
        }
        --size_;

        return &items_[curr_index];
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size > capacity_)
        {
            size_t new_capacity = std::max(new_size, (capacity_ == 0 ? 1 : capacity_ * 2));
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            items_.swap(temp);

            size_ = new_size;
            capacity_ = new_capacity;
        }
        else if (new_size > size_)
        {
            for (size_t i = size_; i < new_size; ++i)
            {
                items_[i] = Type{};
            }
            size_ = new_size;
        }
        else
        {
            size_ = new_size;
        }
    }

    // Резервирование памяти
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

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        size_ = 0;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
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