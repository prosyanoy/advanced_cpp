#pragma once

#include <algorithm>

class ListHook {
public:
    ListHook() : left_(nullptr), right_(nullptr) {
    }
    bool IsLinked() const {
        return left_ && right_;
    }

    void Unlink() {
        if (left_ && right_) {
            if (left_ == right_) {
                left_->left_ = nullptr;
                left_->right_ = nullptr;
            } else {
                left_->right_ = right_;
                right_->left_ = left_;
            }
        }
        left_ = nullptr;
        right_ = nullptr;
    }

    // Must unlink element from list
    ~ListHook() {
        Unlink();
    }

    ListHook(const ListHook&) = delete;

private:
    ListHook* left_;
    ListHook* right_;

    template <class T>
    friend class List;

    // that helper function might be useful
    void LinkBefore(ListHook* other) {
        if (other->left_) {
            other->left_->right_ = this;
            left_ = other->left_;
            other->left_ = this;
            right_ = other;
        } else {
            other->left_ = this;
            other->right_ = this;
            left_ = other;
            right_ = other;
        }
    }
};

template <typename T>
class List {
public:
    class Iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
        ListHook* element_;

    public:
        Iterator(ListHook* element = nullptr) : element_(element) {
        }

        Iterator& operator++() {
            element_ = element_->right_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        T& operator*() const {
            return *static_cast<T*>(element_);
        }

        T* operator->() const {
            return static_cast<T*>(element_);
        }

        bool operator==(const Iterator& rhs) const {
            return rhs.element_ == element_;
        }
        bool operator!=(const Iterator& rhs) const {
            return rhs.element_ != element_;
        }
    };

    List() : dummy_(ListHook()) {
    }
    List(const List&) = delete;
    List(List&& other) {
        dummy_ = other.dummy_;
        other.dummy_.left_ = nullptr;
        other.dummy_.right_ = nullptr;
    }

    // must unlink all elements from list
    ~List() {
        ListHook* back = &dummy_;
        ListHook* next = dummy_.left_;
        if (!next) {
            return;
        }
        while (next && next != next->left_) {
            back->Unlink();
            back = next;
            next = next->left_;
        }
    }

    List& operator=(const List&) = delete;
    List& operator=(List&& other) {
        dummy_ = other.dummy_;
        other.dummy_.left_ = nullptr;
        other.dummy_.right_ = nullptr;
        return *this;
    }

    bool IsEmpty() const {
        return !(dummy_.IsLinked());
    }
    // that method is allowed to be O(n)
    size_t Size() const {
        size_t size = 0;
        ListHook* ptr = dummy_.right_;
        if (ptr) {
            while (ptr != &dummy_) {
                ptr = ptr->right_;
                ++size;
            }
        }
        return size;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(ListHook* elem) {
        elem->LinkBefore(&dummy_);
    }
    void PushFront(ListHook* elem) {
        if (dummy_.right_) {
            elem->LinkBefore(dummy_.right_);
        } else {
            elem->LinkBefore(&dummy_);
        }
    }

    T& Front() {
        return static_cast<T&>(*dummy_.right_);
    }
    const T& Front() const {
        return static_cast<T&>(*dummy_.right_);
    }

    T& Back() {
        return static_cast<T&>(*dummy_.left_);
    }
    const T& Back() const {
        return static_cast<T&>(*dummy_.left_);
    }

    void PopBack() {
        dummy_.left_->Unlink();
    }
    void PopFront() {
        dummy_.right_->Unlink();
    }

    Iterator Begin() {
        return Iterator(dummy_.right_);
    }
    Iterator Begin() const {
        return Iterator(dummy_.right_);
    }
    Iterator End() {
        return Iterator(&dummy_);
    }
    Iterator End() const {
        return Iterator(&dummy_);
    }
    // complexity of this function must be O(1)
    Iterator IteratorTo(ListHook* element) {
        return Iterator(element);
    }

private:
    ListHook dummy_;
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {  // NOLINT
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {  // NOLINT
    return list.End();
}
