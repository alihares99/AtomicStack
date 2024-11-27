#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <random>
#include <cassert>

#include "atomic_stack.h"

using namespace std;

int main(int, char**) {

    std::cout << "Hello, from AtomicStack!\n";

    using Stack = AtomicStack<int, MemoryLeakFinder<10'000'000>>;

    {
        Stack stack;
        stack.push(1);
        assert(stack.pop() == 1);
        stack.~AtomicStack();
        assert(Stack::Node::check());
    }

    {
        Stack stack;
        stack.push(1);
        stack.push(2);
        stack.push(3);
        stack.push(4);
        assert(stack.pop() == 4);
        assert(stack.pop() == 3);
        assert(stack.pop() == 2);
        assert(stack.pop() == 1);
        stack.~AtomicStack();
        assert(Stack::Node::check());
    }

    return 0;
}
