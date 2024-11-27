#include <gtest/gtest.h>
#include "stack_test.h"

class MyFixture : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}

    using Stack = AtomicStack<int, MemoryLeakFinder<100'000'000>>;
    //using Stack = AtomicStack<int, Dummy>;
    Stack stack;

    void check_leak() {
        stack.~AtomicStack();
        ASSERT_TRUE(Stack::Node::check());
    }
};

TEST_F(MyFixture, BasicTest1) {
    stack.push(1);
    ASSERT_EQ(stack.pop(), 1);
    check_leak();
}

TEST_F(MyFixture, BasicTest2) {
    stack.push(1);
    stack.push(2);
    stack.push(3);
    stack.push(4);
    ASSERT_EQ(stack.pop(), 4);
    ASSERT_EQ(stack.pop(), 3);
    ASSERT_EQ(stack.pop(), 2);
    ASSERT_EQ(stack.pop(), 1);
    check_leak();
}

TEST_F(MyFixture, BasicTest3) { for (int i = 0; i < 100; i++) {stack.push(i);}
                                for (int i = 99; i >= 0; i--) {ASSERT_EQ(stack.pop(), i);}
                                check_leak(); }
TEST_F(MyFixture, BasicTest4) { for (int i = 99; i >= 0; i--) {ASSERT_EQ(stack.pop(), 0);} check_leak(); }
TEST_F(MyFixture, BasicTest5) { for (int i = 0; i < 100; i++) {stack.push(i);} check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest1) { check_stack(stack, 1000, 2, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest2) { check_stack(stack, 1000, 2, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest3) { check_stack(stack, 1000, 2, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest4) { check_stack(stack, 30000, 2, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest5) { check_stack(stack, 30000, 2, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest6) { check_stack(stack, 20000, 3, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest7) { check_stack(stack, 20000, 3, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelSimpleTest8) { check_stack(stack, 20000, 4, 0.9); check_leak(); }
TEST_F(MyFixture, TwoThreadPushPopTest1) { check_stack_push_pop(stack, 50'000'000, 2, 0.1); check_leak(); }
TEST_F(MyFixture, TwoThreadPushPopTest2) { check_stack_push_pop(stack, 50'000'000, 2, 0.5); check_leak(); }
TEST_F(MyFixture, TwoThreadPushPopTest3) { check_stack_push_pop(stack, 50'000'000, 2, 0.9); check_leak(); }

TEST_F(MyFixture, TwoThreadTest1) { check_stack(stack, 50'000'000, 2, 0.1); check_leak(); }
TEST_F(MyFixture, TwoThreadTest2) { check_stack(stack, 50'000'000, 2, 0.5); check_leak(); }
TEST_F(MyFixture, TwoThreadTest3) { check_stack(stack, 50'000'000, 2, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelTest1) { check_stack(stack, 2000'000, 10, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelTest2) { check_stack(stack, 2000'000, 50, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelTest3) { check_stack(stack, 2000'000, 10, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelTest4) { check_stack(stack, 2000'000, 50, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelTest5) { check_stack(stack, 2000'000, 10, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelTest6) { check_stack(stack, 2000'000, 50, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop1) { check_stack_push_pop(stack, 100'000, 5, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop2) { check_stack_push_pop(stack, 100'000, 50, 0.9); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop3) { check_stack_push_pop(stack, 100'000, 5, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop4) { check_stack_push_pop(stack, 100'000, 50, 0.5); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop5) { check_stack_push_pop(stack, 100'000, 5, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelTestPushPop6) { check_stack_push_pop(stack, 100'000, 5, 0.1); check_leak(); }
TEST_F(MyFixture, ParallelTestPop1) { check_stack_pop(stack, 100'000, 5); check_leak(); }
TEST_F(MyFixture, ParallelTestPop2) { check_stack_pop(stack, 100'000, 10); check_leak(); }
TEST_F(MyFixture, ParallelTestPop3) { check_stack_pop(stack, 100'000, 60); check_leak(); }
