#include <vector>
#include <thread>
#include <random>
#include <barrier>

#include "../src/atomic_stack.h"

template <class T, class Stack_t>
void check_stack_pop(StackI<Stack_t, T>& stack, int size, int n_threads) {
    std::vector<std::thread> threads;
    std::barrier m_barrier(n_threads);

    for (int i = 0; i < n_threads; i++) {
        threads.emplace_back([&stack, &m_barrier, size](){
            m_barrier.arrive_and_wait();

            for (int i = 0; i < size; i++) {
                stack.push(10);
            }

            m_barrier.arrive_and_wait();

            for (int i = 0; i < size; i++) {
                stack.pop();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    ASSERT_EQ(stack.pop(), 0);
}

template <class T, class Stack_t>
void check_stack_push_pop(StackI<Stack_t, T>& stack, int size, int n_threads, double pop_probability = 0.5) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(pop_probability);

    std::vector<std::thread> threads;
    std::barrier m_barrier(n_threads);
    for (int i = 0; i < n_threads; i++) {
        threads.emplace_back([&dist, &gen, &stack, &m_barrier, size](){
            m_barrier.arrive_and_wait();

            for (int i = 0; i < size; i++) {
                if (dist(gen)) {
                    stack.push(10);
                }
                else {
                    EXPECT_NO_THROW(stack.pop());
                }
            }

            m_barrier.arrive_and_wait();

            for (int i = 0; i < size; i++) {
                EXPECT_NO_THROW(stack.pop());
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}

template <class T, class Stack_t>
void check_stack(StackI<Stack_t, T>& stack, int size, int n_threads, double pop_probability = 0.5) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(pop_probability);

    std::vector<std::thread> threads;
    std::barrier m_barrier(n_threads);
    std::vector<std::vector<int>> all_inputs(n_threads);
    for (int i = 0; i < n_threads; ++i) {
        if (i == 0)
            all_inputs[i].reserve(size * n_threads);
        else 
            all_inputs[i].reserve(size);

        threads.emplace_back([&stack, size, &all_inputs, &dist, &gen, &m_barrier](int rank) {
            int start = rank * size;
            int end = start + size;
            int un_popped = size;

            m_barrier.arrive_and_wait();

            for (int j = start; j < end; ++j) {
                stack.push(j);
                if (dist(gen)) {
                    un_popped -= 1;
                    all_inputs[rank].push_back(stack.pop());
                }
            }

            m_barrier.arrive_and_wait();

            for (int j = un_popped; j > 0; --j) {
                all_inputs[rank].push_back(stack.pop());
            }
        }, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto& aggregate = all_inputs[0];
    aggregate.reserve(size * n_threads);
    for (int i = 1; i < n_threads; ++i) {
        aggregate.insert(aggregate.end(), all_inputs[i].begin(), all_inputs[i].end());
    }
    sort(aggregate.begin(), aggregate.end(), std::greater<T>());
    int last = size * n_threads - 1;
    for (int i = 0; i < aggregate.size(); i++) {
        ASSERT_TRUE(aggregate[i] == last--) << "failed at " << i << std::endl;
    }
}