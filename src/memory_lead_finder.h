#include <vector>
#include <atomic>

template <size_t n>
struct MemoryLeakFinder {
    struct AntiFalseSharingBool {
        alignas(std::hardware_destructive_interference_size) bool d;
        AntiFalseSharingBool(bool _d) : d(_d) {}
        AntiFalseSharingBool() : d(false) {}
    };
    inline static std::vector<AntiFalseSharingBool> mark 
                    = std::vector<AntiFalseSharingBool>(n, false);
    inline static std::atomic<int> last_number {0};
    int my_number;
    MemoryLeakFinder() 
        : my_number(last_number.fetch_add(1, std::memory_order_relaxed)) 
    {
        assert(my_number < n);
    }
    ~MemoryLeakFinder() {
        mark[my_number].d = true;
    }
    static bool check() {
        int last = last_number.load();
        bool res = true;
        for (int i = 0; i < last; i++) {
            if (!mark[i].d) {
                res = false;
            }
            mark[i].d = false;
        }
        last_number.store(0, std::memory_order_relaxed);
        return res;
    }
};

struct Dummy {
    static bool check() { return true; }
};