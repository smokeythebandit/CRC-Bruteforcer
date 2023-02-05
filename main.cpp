// Standard library
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>

// External libraries
#include <CRC.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <argparse/argparse.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/cursor_control.hpp>

// Application
#include "WorkerInstruction.hpp"

GlobalWorkerInfo global_worker_info;
uint64_t last_log_amount = 0;
std::chrono::steady_clock::time_point last_log_time;
std::chrono::steady_clock::time_point program_start;
indicators::ProgressBar bar{
    indicators::option::BarWidth{29},
    indicators::option::Start{"["},
    indicators::option::Fill{"="},
    indicators::option::Lead{">"},
    indicators::option::Remainder{" "},
    indicators::option::End{" ]"},
    indicators::option::ShowElapsedTime{true}, 
    indicators::option::ShowRemainingTime{true},
};

void run_crc(const WorkerInstruction &instructions) {
    // Get data and size
    auto data = instructions.data.data();
    auto data_size = instructions.data.size();
    // Calculate progress numbers
    uint64_t current_amount_attempts = 0;
    uint64_t worker_share_attempts = instructions.worker_info.total_attempts / instructions.worker_info.worker_count;
    uint64_t attempts_per_percent = worker_share_attempts / 1000.0;

    spdlog::debug("Worker {0} has started, starting polynomal 0x{1:x}, ending polynomal 0x{2:x}", instructions.worked_index, instructions.worker_start, instructions.worker_end);
    // Start processing
    for(auto xor_value: instructions.xor_values) {
        for(auto initial_value: instructions.initial_values) {
            for(auto current_polynomal = instructions.worker_start; current_polynomal <= instructions.worker_end; current_polynomal++) {
                auto calculated_crc = CRC::Calculate(data, data_size, CRC::Parameters<crcpp_uint32, 32>({current_polynomal, initial_value, xor_value, true, true}));
                current_amount_attempts++;
                if(calculated_crc == instructions.match_value) {
                    spdlog::info("Found match, polynomal: 0x{0:x}, Checksum: 0x{1:x}", current_polynomal, calculated_crc);
                    exit(0);
                }
                if((current_amount_attempts) % attempts_per_percent == 0) {
                    // Increments attempt amounts
                    instructions.worker_info.processed_entries += current_amount_attempts;
                    current_amount_attempts=0;

                    if(instructions.worked_index == 0) {
                        // Calculate hashrate
                        auto ms_since_last_log = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_log_time).count();
                        auto hashes_per_ms = (instructions.worker_info.processed_entries - last_log_amount) / ms_since_last_log;
                        last_log_amount = instructions.worker_info.processed_entries;
                        last_log_time = std::chrono::steady_clock::now();

                        bar.set_progress(((float)instructions.worker_info.processed_entries / (float)instructions.worker_info.total_attempts) * 100.0);
                        bar.set_option(indicators::option::PostfixText{fmt::format("{} CRCs per Second", std::to_string(hashes_per_ms * 1000))});
                    }
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    program_start = std::chrono::steady_clock::now();
    argparse::ArgumentParser program("CRC-Bruteforcer");

    // Configuration
    program.add_argument("--start-polynomal").default_value(std::string("00000000")).help("Polynomal to start with");
    program.add_argument("--end-polynomal").default_value(std::string("FFFFFFFF")).help("Polynomal to end with");
    program.add_argument("--initial-values").default_value<std::vector<std::string>>({ "FFFFFFFF", "00000000" }).append().help("Initial CRC value");
    program.add_argument("--final-xor").default_value<std::vector<std::string>>({ "FFFFFFFF", "00000000" }).append().help("Initial CRC value");

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
    std::vector<uint32_t> initial_values, xor_values;
    uint32_t start_polynomal, end_polynomal, match_value;
    std::istringstream(program.get<std::string>("checksum")) >> std::hex >> match_value;
    std::istringstream(program.get<std::string>("--start-polynomal")) >> std::hex >> start_polynomal;
    std::istringstream(program.get<std::string>("--end-polynomal")) >> std::hex >> end_polynomal;
    for(auto initial_value : program.get<std::vector<std::string>>("--initial-values")) {
        uint32_t value;
        std::istringstream(initial_value) >> std::hex >> value;
        initial_values.push_back(value);
    }
    for(auto xor_value : program.get<std::vector<std::string>>("--final-xor")) {
        uint32_t value;
        std::istringstream(xor_value) >> std::hex >> value;
        xor_values.push_back(value);
    }

    // Configure settings for each worker
    auto crc_count = end_polynomal - start_polynomal;
    global_worker_info.worker_count =  std::thread::hardware_concurrency();
    global_worker_info.total_attempts = crc_count * initial_values.size() * xor_values.size();
    std::vector<WorkerInstruction> worker_instructions;
    for(unsigned int worker_index = 0; worker_index < global_worker_info.worker_count; ++worker_index) {
        WorkerInstruction worker_instruction(global_worker_info);
        worker_instruction.worked_index = worker_index;
        worker_instruction.data = buffer;
        worker_instruction.worker_start = crc_count / global_worker_info.worker_count * worker_index;
        worker_instruction.worker_end = crc_count / global_worker_info.worker_count * (worker_index + 1);
        worker_instruction.initial_values = initial_values;
        worker_instruction.xor_values = xor_values;
        worker_instruction.match_value = match_value;
        worker_instructions.push_back(worker_instruction);
    }
    auto dash_fold = [](std::string a, uint32_t b)
    {
        if(a.length() == 0) return fmt::format("0x{:08x}",b);
        return a + ", " + fmt::format("0x{:08x}",b);
    };
 
    // Print info
    spdlog::info("-----------Settings-----------");
    spdlog::info("Starting polynomal: 0x{0:08x}", start_polynomal);
    spdlog::info("Ending polynomal  : 0x{0:08x}", end_polynomal);
    spdlog::info("Initial values    : [{0}]", std::accumulate(initial_values.begin(), initial_values.end(), std::string{}, dash_fold));
    spdlog::info("Final XOR value   : [{0}]", std::accumulate(xor_values.begin(), xor_values.end(), std::string{}, dash_fold));
    spdlog::info("CRC to match with : 0x{0:08x}", match_value);
    spdlog::info("Amount of attempts: {}", global_worker_info.total_attempts);
    spdlog::info("-----------------------------");

    spdlog::info("Dispatching {} workers", global_worker_info.worker_count);


    last_log_time = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<std::thread>> threadPool;
    for(auto instruction: worker_instructions) {
        auto worker = std::shared_ptr<std::thread>(new std::thread(run_crc, instruction));
        threadPool.push_back(std::move(worker));
    }
    indicators::show_console_cursor(false);

    for(auto thread: threadPool) {
        thread->join();
    }
    std::cout << "\33[2K\r" << std::flush;
    spdlog::info("Finished execution, took: {} seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - program_start).count());


    /* code */
    return 0;
}
