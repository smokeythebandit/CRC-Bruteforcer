#include <atomic>
#include <vector>
#include <stdint.h>

class GlobalWorkerInfo {
    public:
    uint32_t worker_count;
    uint64_t total_attempts;
    std::atomic_int64_t processed_entries;
};

class WorkerInstruction
{
    public:
    WorkerInstruction(GlobalWorkerInfo &worker_info) : worker_info(worker_info) {};
    uint32_t worked_index;
    uint32_t worker_start;
    uint32_t worker_end;
    uint32_t match_value;
    std::vector<uint32_t> xor_values;
    std::vector<uint32_t> initial_values;
    std::vector<unsigned char> data;
    GlobalWorkerInfo &worker_info;
};
