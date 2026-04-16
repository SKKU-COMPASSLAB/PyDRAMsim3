#include "py_interface.h"
#include "memory_system_wrapper.h"

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void *pydramsim3_create_msys(char *config_file, char *output_dir, int cmd_queue_num, int max_issue_per_cmd_q_per_cycle) {
    return new MemorySystemWrapper(config_file, output_dir, cmd_queue_num, max_issue_per_cmd_q_per_cycle);
}

void  pydramsim3_destroy_msys(void *msys) {
    delete static_cast<MemorySystemWrapper *>(msys);
}


void *pydramsim3_create_msys_cmd(int cmd_q_id, msys_data_t addr, msys_data_t size, char is_write) {
    return new MemorySystemCommand{cmd_q_id, (uint64_t)addr, (uint64_t)size, (bool)is_write, 0, 0, 0, NULL, NULL};
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
    
    int mem_transfer_size = (msys->get_bus_bits() / 8) * msys->get_burst_length();
    int cycles = cmd->size / (mem_transfer_size);

    return cycles;
}


char  pydramsim3_msys_dispatch_cmd(void *msys, void *cmd_p, callback_t dispatch_callback, callback_t execute_callback) {
    MemorySystemCommand *cmd = static_cast<MemorySystemCommand *>(cmd_p);
    MemorySystemWrapper *msys_wrapper = static_cast<MemorySystemWrapper *>(msys);

    cmd->n_req = (cmd->size + (msys_wrapper->get_bus_bits() / 8 * msys_wrapper->get_burst_length()) - 1) /
                  (msys_wrapper->get_bus_bits() / 8 * msys_wrapper->get_burst_length());
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
