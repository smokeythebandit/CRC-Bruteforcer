// Standard library
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

// External libraries
#include <CRC.h>
#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>

// Application
#include "WorkerInstruction.hpp"

std::chrono::steady_clock::time_point program_start;

void run_crc(const WorkerInstruction &instructions) {
    spdlog::debug("Worker {0} has started, starting polynomal 0x{1:x}, ending polynomal 0x{2:x}", instructions.worked_index, instructions.worker_start, instructions.worker_end);
    for(uint32_t currentPolynomal = instructions.worker_start; currentPolynomal <= instructions.worker_end; currentPolynomal++) {
        auto calculated_crc = CRC::Calculate(instructions.data.data(), sizeof(instructions.data.size()), CRC::Parameters<crcpp_uint32, 32>({currentPolynomal, instructions.initial_value, instructions.final_xor, true, true}));
        if(calculated_crc == instructions.match_value) {
            spdlog::info("Found match, polynomal: 0x{0:x}, Checksum: 0x{1:x}", currentPolynomal, calculated_crc);
            spdlog::info("Finished execution, took: {} seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - program_start).count());
            exit(0);
        }
        uint32_t modulo = currentPolynomal % 1000000;
    }
}

int main(int argc, char const *argv[])
{
    program_start = std::chrono::steady_clock::now();
    argparse::ArgumentParser program("CRC-Bruteforcer");

    // Configuration
    program.add_argument("--start-polynomal").default_value(std::string("00000000")).help("Polynomal to start with");
    program.add_argument("--end-polynomal").default_value(std::string("FFFFFFFF")).help("Polynomal to end with");
    program.add_argument("--initial-value").default_value(std::string("FFFFFFFF")).help("Initial CRC value");
    program.add_argument("--final-xor").default_value(std::string("FFFFFFFF")).help("Initial CRC value");

    // Input
    program.add_argument("file").help("File to process");
    program.add_argument("checksum").help("Checksum we need to match with");

    // Parse arguments
    try {
        program.parse_args(argc, argv);
    }
        catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    // Open the input file and read contents
    std::ifstream input( program.get<std::string>("file"), std::ios::binary);
    const std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

    // Convert polynomals to number
    uint32_t start_polynomal, end_polynomal, initial_value, final_xor, match_value;
    std::istringstream(program.get<std::string>("checksum")) >> std::hex >> match_value;
    std::istringstream(program.get<std::string>("--start-polynomal")) >> std::hex >> start_polynomal;
    std::istringstream(program.get<std::string>("--end-polynomal")) >> std::hex >> end_polynomal;
    std::istringstream(program.get<std::string>("--initial-value")) >> std::hex >> initial_value;
    std::istringstream(program.get<std::string>("--final-xor")) >> std::hex >> final_xor;

    // Configure settings for each worker
    auto worker_count =  std::thread::hardware_concurrency();
    auto crc_count = end_polynomal - start_polynomal;
    std::vector<WorkerInstruction> worker_instructions;
    for(unsigned int worker_index = 0; worker_index < worker_count; ++worker_index) {
        WorkerInstruction worker_instruction;
        worker_instruction.worked_index = worker_index;
        worker_instruction.data = buffer;
        worker_instruction.worker_start = crc_count / worker_count * worker_index;
        worker_instruction.worker_end = crc_count / worker_count * (worker_index + 1);
        worker_instruction.initial_value = initial_value;
        worker_instruction.final_xor = final_xor;
        worker_instruction.match_value = match_value;
        worker_instructions.push_back(worker_instruction);
    }

    // Print info
    spdlog::info("-----------Settings-----------");
    spdlog::info("Starting polynomal: 0x{0:08x}", start_polynomal);
    spdlog::info("Ending polynomal  : 0x{0:08x}", end_polynomal);
    spdlog::info("Initial value     : 0x{0:08x}", initial_value);
    spdlog::info("Final XOR value   : 0x{0:08x}", final_xor);
    spdlog::info("CRC to match with : 0x{0:08x}", match_value);
    spdlog::info("Amount of attempts: {}", crc_count);
    spdlog::info("-----------------------------");

    spdlog::info("Dispatching {} workers", worker_count);


    std::vector<std::shared_ptr<std::thread>> threadPool;
    for(auto instruction: worker_instructions) {
        auto worker = std::shared_ptr<std::thread>(new std::thread(run_crc, instruction));
        threadPool.push_back(std::move(worker));
    }
    spdlog::info("Succesfully dispatched {} workers", worker_count);


    for(auto thread: threadPool) {
        thread->join();
    }

    spdlog::info("Finished execution, took: {} seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - program_start).count());


    /* code */
    return 0;
}
