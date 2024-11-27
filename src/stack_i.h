#pragma once

template <class Stack_t, class T>
class StackI {
public:
    void push(T val) { static_cast<Stack_t*>(this)->push(val); }
    T pop() { return static_cast<Stack_t*>(this)->pop(); }
};