#include "py_interface.h"
#include "memory_system_wrapper.h"

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

void *pydramsim3_create_msys(char *config_file, char *output_dir, int max_issue_per_cmd_q_per_cycle) {
    return new MemorySystemWrapper(config_file, output_dir, max_issue_per_cmd_q_per_cycle);
}

void  pydramsim3_destroy_msys(void *msys) {
    delete static_cast<MemorySystemWrapper *>(msys);
}


void *pydramsim3_create_msys_cmd(msys_data_t addr, msys_data_t size, char is_write) {
    return new MemorySystemCommand{(uint64_t)addr, (uint64_t)size, (bool)is_write, 0, 0, 0, NULL, NULL};
}

void pydramsim3_destroy_msys_cmd(void *cmd_p) {
    delete static_cast<MemorySystemCommand *>(cmd_p);
}


char  pydramsim3_check_msys_cmd_dispatched(void *cmd_p) {
    auto cmd = static_cast<MemorySystemCommand *>(cmd_p);
    if (cmd->n_req == 0)
        return 0;
    return (char)check_msys_cmd_dispatched(cmd);
}

char  pydramsim3_check_msys_cmd_executed(void *cmd_p) {
    auto cmd = static_cast<MemorySystemCommand *>(cmd_p);
    if (cmd->n_req == 0)
        return 0;
    return (char)check_msys_cmd_executed(cmd);
}

int   pydramsim3_get_expected_cmd_cycles(void *msys_p, void *cmd_p) {
    MemorySystemWrapper *msys = static_cast<MemorySystemWrapper *>(msys_p);
    MemorySystemCommand *cmd = static_cast<MemorySystemCommand *>(cmd_p);
    
    uint64_t transfer_size = (msys->get_bus_bits() / 8) * msys->get_burst_length();
    
    if (cmd->size == 0) return 0;
    
    uint64_t start_addr = cmd->addr;
    uint64_t end_addr = cmd->addr + cmd->size - 1;
    uint64_t start_aligned = start_addr & ~(transfer_size - 1);
    uint64_t end_aligned = end_addr & ~(transfer_size - 1);
    
    int cycles = ((end_aligned - start_aligned) / transfer_size) + 1;

    return cycles;
}


char  pydramsim3_msys_dispatch_cmd(void *msys, void *cmd_p, callback_t dispatch_callback, callback_t execute_callback) {
    MemorySystemCommand *cmd = static_cast<MemorySystemCommand *>(cmd_p);
    MemorySystemWrapper *msys_wrapper = static_cast<MemorySystemWrapper *>(msys);

    uint64_t transfer_size = (msys_wrapper->get_bus_bits() / 8 * msys_wrapper->get_burst_length());
    uint64_t start_addr = cmd->addr;
    uint64_t end_addr = cmd->addr + cmd->size - 1;
    uint64_t start_aligned = start_addr & ~(transfer_size - 1);
    uint64_t end_aligned = end_addr & ~(transfer_size - 1);
    
    if (cmd->size == 0) {
        cmd->n_req = 0;
    } else {
        cmd->n_req = ((end_aligned - start_aligned) / transfer_size) + 1;
    }
    
    cmd->dispatch_callback = dispatch_callback;
    cmd->execute_callback = execute_callback;
    return msys_wrapper->dispatch_command(cmd);
}

void  pydramsim3_msys_cycle_step(void *msys) {
    static_cast<MemorySystemWrapper *>(msys)->cycle_step();
}

double pydramsim3_msys_get_tck(void *msys) {
    return static_cast<MemorySystemWrapper *>(msys)->get_tck();
}


#ifdef __cplusplus
}
#endif
