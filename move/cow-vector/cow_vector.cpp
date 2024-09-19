#include "cow_vector.h"
void COWVector::CleanState() {
    if (state_) {
        if (!(state_->ref_count)) {
            delete state_;
            state_ = nullptr;
        } else {
            --state_->ref_count;
            if (!state_->ref_count) {
                delete state_;
                state_ = nullptr;
            }
        }
    }
}
COWVector::COWVector() : state_(new State{0, {}, false}) {
}
COWVector::~COWVector() {
    CleanState();
}

COWVector::COWVector(const COWVector& other) {
    state_ = other.state_;
    ++(state_->ref_count);
}
COWVector& COWVector::operator=(const COWVector& other) {
    CleanState();
    state_ = other.state_;
    ++(state_->ref_count);
    return *this;
}

COWVector::COWVector(COWVector&& other) {
    state_ = other.state_;
    other.state_ = nullptr;
    // ref_count stays the same
}

COWVector& COWVector::operator=(COWVector&& other) {
    CleanState();
    state_ = other.state_;
    other.state_ = nullptr;
    // ref_count stays the same
    return *this;
}

size_t COWVector::Size() const {
    return state_->v.size();
}

void COWVector::Resize(size_t size) {
    state_->v.resize(size);
}

const std::string& COWVector::Get(size_t at) {
    return state_->v[at];
}

const std::string& COWVector::Back() {
    return state_->v.back();
}

void COWVector::PushBack(const std::string& value) {
    if (state_->deep_copy || state_->v.empty()) {
        state_->v.push_back(value);
    } else {
        State* temp = new State{*state_};
        CleanState();
        temp->deep_copy = true;
        temp->ref_count = 1;
        temp->v.push_back(value);
        state_ = temp;
    }
}

void COWVector::Set(size_t at, const std::string& value) {
    if (state_->deep_copy || state_->v.empty()) {
        state_->v[at] = value;
    } else {
        State* temp = new State{*state_};
        CleanState();
        temp->deep_copy = true;
        temp->ref_count = 1;
        temp->v[at] = value;
        state_ = temp;
    }
}