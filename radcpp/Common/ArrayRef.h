#ifndef RADCPP_ARRAY_REF
#define RADCPP_ARRAY_REF
#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

#include "radcpp/Common/SmallVector.h"

/// ArrayRef - Represent a constant reference to an array (0 or more elements consecutively in memory).
/// This class does not own the underlying data, it is expected to be used in situations
/// where the data resides in some other buffer, whose lifetime extends past that of the ArrayRef.
/// For this reason, it is not in general safe to store an ArrayRef.
/// This is intended to be trivially copyable, so it should be passed by value.
/// Please refer to: https://llvm.org/doxygen/ArrayRef_8h_source.html
template<typename T>
class ArrayRef {
public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = const_pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

private:
    /// The start of the array, in an external buffer.
    const T* m_data = nullptr;

    /// The number of elements.
    size_type m_count = 0;

public:
    /// @name Constructors
    /// @{

    /// Construct an empty ArrayRef.
    /*implicit*/ ArrayRef() = default;

    /// Construct an empty ArrayRef from None.
    /*implicit*/ ArrayRef(nullptr_t) {}

    /// Construct an ArrayRef from a single element.
    /*implicit*/ ArrayRef(const T& OneElement)
        : m_data(&OneElement), m_count(1) {}

    /// Construct an ArrayRef from a pointer and length.
    /*implicit*/ ArrayRef(const T* data, size_t length)
        : m_data(data), m_count(length) {}

    /// Construct an ArrayRef from a range.
    ArrayRef(const T* begin, const T* end)
        : m_data(begin), m_count(end - begin) {}

    /// Construct an ArrayRef from a SmallVector.
    template<size_t DefaultCapacity>
    /*implicit*/ ArrayRef(const SmallVector<T, DefaultCapacity>& Vec)
        : m_data(Vec.data()), m_count(Vec.size()) {
    }

    /// Construct an ArrayRef from a std::vector.
    template<typename A>
    /*implicit*/ ArrayRef(const std::vector<T, A>& Vec)
        : m_data(Vec.data()), m_count(Vec.size()) {}

    /// Construct an ArrayRef from a std::array
    template <size_t N>
    /*implicit*/ constexpr ArrayRef(const std::array<T, N>& Arr)
        : m_data(Arr.data()), m_count(N) {}

    /// Construct an ArrayRef from a C array.
    template <size_t N>
    /*implicit*/ constexpr ArrayRef(const T(&Arr)[N]) : m_data(Arr), m_count(N) {}

    /// Construct an ArrayRef from a std::initializer_list.
    /*implicit*/ ArrayRef(const std::initializer_list<T>& Vec)
        : m_data(Vec.begin() == Vec.end() ? (T*)nullptr : Vec.begin()),
        m_count(Vec.size()) {}

    /// @}
    /// @name Simple Operations
    /// @{

    iterator begin() const { return m_data; }
    iterator end() const { return m_data + m_count; }

    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

    /// empty - Check if the array is empty.
    bool empty() const { return m_count == 0; }

    const T* data() const { return m_data; }

    /// size - Get the array size.
    size_t size() const { return m_count; }

    /// front - Get the first element.
    const T& front() const {
        assert(!empty());
        return m_data[0];
    }

    /// back - Get the last element.
    const T& back() const {
        assert(!empty());
        return m_data[m_count - 1];
    }

    /// equals - Check for element-wise equality.
    bool equals(ArrayRef RHS) const {
        if (m_count != RHS.m_count)
            return false;
        return std::equal(begin(), end(), RHS.begin());
    }

    /// slice(n, m) - Chop off the first N elements of the array, and keep M
    /// elements in the array.
    ArrayRef<T> slice(size_t N, size_t M) const {
        assert(N + M <= size() && "Invalid specifier");
        return ArrayRef<T>(data() + N, M);
    }

    /// slice(n) - Chop off the first N elements of the array.
    ArrayRef<T> slice(size_t N) const { return slice(N, size() - N); }

    /// Drop the first \p N elements of the array.
    ArrayRef<T> drop_front(size_t N = 1) const {
        assert(size() >= N && "Dropping more elements than exist");
        return slice(N, size() - N);
    }

    /// Drop the last \p N elements of the array.
    ArrayRef<T> drop_back(size_t N = 1) const {
        assert(size() >= N && "Dropping more elements than exist");
        return slice(0, size() - N);
    }

    /// Return a copy of *this with the first N elements satisfying the
    /// given predicate removed.
    template <class PredicateT> ArrayRef<T> drop_while(PredicateT Pred) const {
        return ArrayRef<T>(find_if_not(*this, Pred), end());
    }

    /// Return a copy of *this with the first N elements not satisfying
    /// the given predicate removed.
    template <class PredicateT> ArrayRef<T> drop_until(PredicateT Pred) const {
        return ArrayRef<T>(find_if(*this, Pred), end());
    }

    /// Return a copy of *this with only the first \p N elements.
    ArrayRef<T> take_front(size_t N = 1) const {
        if (N >= size())
            return *this;
        return drop_back(size() - N);
    }

    /// Return a copy of *this with only the last \p N elements.
    ArrayRef<T> take_back(size_t N = 1) const {
        if (N >= size())
            return *this;
        return drop_front(size() - N);
    }

    /// Return the first N elements of this Array that satisfy the given
    /// predicate.
    template <class PredicateT> ArrayRef<T> take_while(PredicateT Pred) const {
        return ArrayRef<T>(begin(), find_if_not(*this, Pred));
    }

    /// Return the first N elements of this Array that don't satisfy the
    /// given predicate.
    template <class PredicateT> ArrayRef<T> take_until(PredicateT Pred) const {
        return ArrayRef<T>(begin(), find_if(*this, Pred));
    }

    /// @}
    /// @name Operator Overloads
    /// @{
    const T& operator[](size_t Index) const {
        assert(Index < m_count && "Invalid index!");
        return m_data[Index];
    }

    /// Disallow accidental assignment from a temporary.
    ///
    /// The declaration here is extra complicated so that "arrayRef = {}"
    /// continues to select the move assignment operator.
    template <typename U>
    std::enable_if_t<std::is_same<U, T>::value, ArrayRef<T>>&
        operator=(U&& Temporary) = delete;

    /// Disallow accidental assignment from a temporary.
    ///
    /// The declaration here is extra complicated so that "arrayRef = {}"
    /// continues to select the move assignment operator.
    template <typename U>
    std::enable_if_t<std::is_same<U, T>::value, ArrayRef<T>>&
        operator=(std::initializer_list<U>) = delete;

    /// @}
    /// @name Expensive Operations
    /// @{
    std::vector<T> vec() const {
        return std::vector<T>(m_data, m_data + m_count);
    }

    /// @}
    /// @name Conversion operators
    /// @{
    operator std::vector<T>() const {
        return std::vector<T>(m_data, m_data + m_count);
    }

    /// @}
}; // ArrayRef

#endif // RADCPP_ARRAY_REF