#pragma once

#include <atomic>
#include <array>
#include <vector>
#include <optional>
#include "stack_i.h"
#include "memory_lead_finder.h"

template <class T, class TestSuit = Dummy>
class AtomicStack : public StackI<AtomicStack<T, TestSuit>, T> {
public:
    AtomicStack() {}
    ~AtomicStack() { // not thread safe
        auto node = head.exchange(nullptr);
        while (node) {
            auto next = node->next;
            delete node;
            node = next;
        }

        node = snapshot.exchange(nullptr);
        while (node) {
            auto next = node->next;
            delete node;
            node = next;
        }

        node = garbage.exchange(nullptr);
        while (node) {
            auto next = node->next;
            delete node;
            node = next;
        }
    }

    void push(T item) ;
    T pop();

    struct Node : TestSuit {
        T data;
        Node* next = nullptr;
        uint16_t who_can_delete_me = 0;

        Node(T _data) : data(_data) {}
        Node() : data(T{}) {}
    };

//private:
    std::atomic<Node*> head {nullptr};
    std::atomic<uint32_t> count_data {0}; // 16 bit a_active_thread, 16 bit rank
    std::atomic<Node*> garbage {nullptr};
    std::atomic<Node*> snapshot {nullptr};
    enum AtomicState : int { state_empty = 0, state_locked = 1, state_ready = 2};
    std::atomic<AtomicState> snapshot_state {state_empty};

    // todo delete later
    std::atomic<int> garbage_count {0};
    std::atomic<int> garbage_count_max {0};

    bool circular_is_greater(uint16_t a, uint16_t b) {
        uint16_t ref = (count_data.load() >> 16) + 2;
        a = (a >= ref) ? a - ref : std::numeric_limits<uint16_t>::max() - ref + a;
        b = (b >= ref) ? b - ref : std::numeric_limits<uint16_t>::max() - ref + b;
        return a >= b;
    }
};

template <class T, class TestSuit>
inline void AtomicStack<T, TestSuit>::push(T item) {
    Node* new_head = new Node(item);
    new_head->next = head.load();
    while(!head.compare_exchange_weak(new_head->next, new_head));
}

template <class T, class TestSuit>
inline T AtomicStack<T, TestSuit>::pop() {
    uint32_t r_count_data_1 = count_data.fetch_add(0x0001'0001);

    Node* old_head;
    do {
        old_head = head.load();
        if (old_head == nullptr) 
            return T{};
    } while(!head.compare_exchange_strong(old_head, old_head->next));
    
    uint32_t r_count_data_2 = count_data.fetch_sub(1);
    
    r_count_data_1 += 0x0001'0001;
    r_count_data_2 -= 1;
    uint16_t my_rank = r_count_data_1 >> 16;
    uint16_t n_threads_before = r_count_data_1 & 0xFFFF;
    uint16_t last_rank = r_count_data_2 >> 16;
    uint16_t n_threads_in_the_loop = r_count_data_2 & 0xFFFF;

    auto tmp = old_head->data;
    if (n_threads_in_the_loop == 0) {
        delete old_head;
    } else {
        old_head->who_can_delete_me = last_rank + 1;
        Node* expected = nullptr;
        if (snapshot.compare_exchange_strong(expected, old_head)) {
            // nothing to do, just leave it there
        } else {
            // expected is loaded with old_head
            Node* dummy_pointer = (Node*) 1;
            if (snapshot.compare_exchange_strong(expected, dummy_pointer)) {
                if (circular_is_greater(my_rank, expected->who_can_delete_me)) {
                    delete expected;
                    snapshot.store(nullptr);
                }
                else {
                    snapshot.store(expected);
                }
            }
        }
    }
/*
    if (n_threads_in_the_loop == 0) {
        delete old_head;
    } else {
        { 
            // todo delete later
            auto reading = garbage_count.fetch_add(1) + 1;
            if (reading > garbage_count_max.load()) {
                garbage_count_max.store(reading);
            }
        }

        old_head->who_can_delete_me = last_rank + 1;
        old_head->next = garbage.load();
        while (!garbage.compare_exchange_weak(old_head->next, old_head));
    }

    AtomicState expected = state_ready;
    if (snapshot_state.compare_exchange_strong(expected, state_locked)) {
        Node* p = snapshot.load();
        while (p) {
            if (circular_is_greater(my_rank, p->who_can_delete_me)) {
                auto tmp = p->next;
                delete p;
                p = tmp;

                // todo delete later
                garbage_count.fetch_sub(1);
            } else {
                break;
            }
        }
        if (p) { // break happened
            snapshot.store(p);
            snapshot_state.store(state_ready);
        } else {
            snapshot.store(nullptr);
            snapshot_state.store(state_empty);
        }
    } else if (expected == state_empty) { // note: expected gets its value from last if statement
        if (snapshot_state.compare_exchange_strong(expected, state_locked)) {
            auto garbage_old = garbage.exchange(nullptr);
            if (garbage_old) {
                snapshot.store(garbage_old);
                snapshot_state.store(state_ready);
            } else {
                snapshot_state.store(state_empty);
            }
        }
    }
*/

    return tmp;
}
