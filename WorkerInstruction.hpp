#include <vector>
#include <stdint.h>

class WorkerInstruction
{
    public:
    WorkerInstruction(/* args */);
    ~WorkerInstruction();
    uint32_t worked_index;
    uint32_t worker_start;
    uint32_t worker_end;
    uint32_t initial_value;
    uint32_t final_xor;
    uint32_t match_value;
    std::vector<unsigned char> data;
};

WorkerInstruction::WorkerInstruction(/* args */)
{
}

WorkerInstruction::~WorkerInstruction()
{
}
