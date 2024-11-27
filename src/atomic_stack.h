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

    bool circular_is_greater(uint16_t a, uint16_t b) {
        // todo correct here
        uint16_t ref = (count_data.load() >> 16) + 20000;
        a = (a >= ref) ? a - ref : std::numeric_limits<uint16_t>::max() - ref + a;
        b = (b >= ref) ? b - ref : std::numeric_limits<uint16_t>::max() - ref + b;
        return a <= b;
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
    uint32_t r_count_data_1 = count_data.fetch_add(0x0001'0001) + 0x0001'0001;
    uint16_t rank1 = r_count_data_1 >> 16;
    uint16_t n_active_threads_1 = r_count_data_1 & 0xFFFF;

    Node* old_head;
    do {
        old_head = head.load();
        if (old_head == nullptr) 
            return T{};
    } while(!head.compare_exchange_strong(old_head, old_head->next));
    auto tmp = old_head->data;
    
    uint32_t r_count_data_2 = count_data.fetch_sub(1) - 1;
    uint16_t rank2 = r_count_data_2 >> 16;
    uint16_t n_active_threads_2 = r_count_data_2 & 0xFFFF;

    if (n_active_threads_2 == 0) {
        delete old_head;
    } else {
        old_head->who_can_delete_me = rank2 + 10000;
        old_head->next = garbage.load();
        while (!garbage.compare_exchange_weak(old_head->next, old_head));
    }

    AtomicState expected = state_empty;
    if (snapshot_state.compare_exchange_strong(expected, state_locked)) {
        auto garbage_old = garbage.exchange(nullptr);
        if (garbage_old) {
            snapshot.store(garbage_old);
            snapshot_state.store(state_ready);
        } else {
            snapshot_state.store(state_empty);
        }
    } else {
        expected = state_ready;
        if (snapshot_state.compare_exchange_strong(expected, state_locked)) {
            Node* p = snapshot.load();
            while (p) {
                if (circular_is_greater(p->who_can_delete_me, rank1)) {
                    auto tmp = p->next;
                    delete p;
                    p = tmp;
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
        }
    }

    return tmp;
}
