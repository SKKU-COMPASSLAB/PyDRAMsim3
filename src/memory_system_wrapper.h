#ifndef __MEMORY_SYSTEM_WRAPPER_H
#define __MEMORY_SYSTEM_WRAPPER_H

#include <map>
#include <queue>

#include "memory_system.h"


typedef struct {
    uint64_t addr;
    uint64_t size;
    bool is_write;
    uint64_t n_req;
    uint64_t dispatch_progress;
    uint64_t execute_progress;
} MemorySystemCommand;

bool check_msys_cmd_executed(MemorySystemCommand *cmd_p);
bool check_msys_cmd_dispatched(MemorySystemCommand *cmd_p);


class MemorySystemWrapper {
private:
    char *_config_file_raw;
    char *_output_dir_raw;

    dramsim3::MemorySystem *_msys_p;
    unsigned int _transfer_size;

    MemorySystemCommand *_current_dispatched_cmd_p = NULL;

    std::map<uint64_t, std::queue<MemorySystemCommand *>> _ongoing_rd_req_cmd_map;
    std::map<uint64_t, std::queue<MemorySystemCommand *>> _ongoing_wr_req_cmd_map;

public:
    MemorySystemWrapper(
        char *config_file, 
        char *output_dir
    );
    ~MemorySystemWrapper();

    bool dispatch_command(MemorySystemCommand *cmd_p);
    void cycle_step();
    void read_callback(uint64_t addr);
    void write_callback(uint64_t addr);
    double get_tck() const;
    int get_bus_bits() const;
    int get_burst_length() const;
};

#endif