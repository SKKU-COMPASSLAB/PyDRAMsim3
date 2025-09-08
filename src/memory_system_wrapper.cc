#include <iostream>
#include "memory_system_wrapper.h"


bool check_msys_cmd_executed(MemorySystemCommand *cmd_p) {
    return cmd_p->execute_progress >= cmd_p->n_req;
}

bool check_msys_cmd_dispatched(MemorySystemCommand *cmd_p) {
    return cmd_p->dispatch_progress >= cmd_p->n_req;
}


MemorySystemWrapper::MemorySystemWrapper(
    char *config_file, 
    char *output_dir
) {
    this->_config_file_raw = strdup(config_file);
    this->_output_dir_raw = strdup(output_dir);

    this->_msys_p = new dramsim3::MemorySystem(
        this->_config_file_raw, this->_output_dir_raw,
        std::bind(&MemorySystemWrapper::read_callback, this, std::placeholders::_1),
        std::bind(&MemorySystemWrapper::write_callback, this, std::placeholders::_1)
    );

    this->_transfer_size = (this->_msys_p->GetBusBits() / 8) * this->_msys_p->GetBurstLength();
}

MemorySystemWrapper::~MemorySystemWrapper() {
    free(this->_config_file_raw);
    free(this->_output_dir_raw);
    delete this->_msys_p;
}

bool MemorySystemWrapper::dispatch_command(MemorySystemCommand *cmd_p) {
    if (this->_current_dispatched_cmd_p != NULL) {
        return false;
    }
    
    this->_current_dispatched_cmd_p = cmd_p;
    this->_current_dispatched_cmd_p->n_req = (this->_current_dispatched_cmd_p->size + this->_transfer_size - 1) / this->_transfer_size;
    this->_current_dispatched_cmd_p->dispatch_progress = 0;
    this->_current_dispatched_cmd_p->execute_progress = 0;

    return true;
}

void MemorySystemWrapper::cycle_step() {
    _msys_p->ClockTick();

    if (this->_current_dispatched_cmd_p) {
        const auto is_write = this->_current_dispatched_cmd_p->is_write;
        const auto n_req    = this->_current_dispatched_cmd_p->n_req;
        const auto req_addr = this->_current_dispatched_cmd_p->addr + (this->_current_dispatched_cmd_p->dispatch_progress * this->_transfer_size);

        if (_msys_p->WillAcceptTransaction(req_addr, is_write)) {
            _msys_p->AddTransaction(req_addr, is_write);

            if (!is_write) {
                if (this->_ongoing_rd_req_cmd_map.find(req_addr) == this->_ongoing_rd_req_cmd_map.end())
                    this->_ongoing_rd_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
                this->_ongoing_rd_req_cmd_map[req_addr].push(this->_current_dispatched_cmd_p);
            } else {
                if (this->_ongoing_wr_req_cmd_map.find(req_addr) == this->_ongoing_wr_req_cmd_map.end())
                    this->_ongoing_wr_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
                this->_ongoing_wr_req_cmd_map[req_addr].push(this->_current_dispatched_cmd_p);
            }
            
            this->_current_dispatched_cmd_p->dispatch_progress++;

            if (this->_current_dispatched_cmd_p->dispatch_progress >= n_req) {
                this->_current_dispatched_cmd_p = NULL;
            }
        }
    }
}

void MemorySystemWrapper::read_callback(uint64_t addr) {
    if (this->_ongoing_rd_req_cmd_map.find(addr) == this->_ongoing_rd_req_cmd_map.end())
        return;

    while (this->_ongoing_rd_req_cmd_map[addr].size()) {
        MemorySystemCommand *cmd_p = this->_ongoing_rd_req_cmd_map[addr].front();
        cmd_p->execute_progress++;
        this->_ongoing_rd_req_cmd_map[addr].pop();
    }

    this->_ongoing_rd_req_cmd_map.erase(addr);
}

void MemorySystemWrapper::write_callback(uint64_t addr) {
    if (this->_ongoing_wr_req_cmd_map.find(addr) == this->_ongoing_wr_req_cmd_map.end())
        return;

    while (this->_ongoing_wr_req_cmd_map[addr].size()) {
        MemorySystemCommand *cmd_p = this->_ongoing_wr_req_cmd_map[addr].front();
        cmd_p->execute_progress++;
        this->_ongoing_wr_req_cmd_map[addr].pop();
    }

    this->_ongoing_wr_req_cmd_map.erase(addr);
}

double MemorySystemWrapper::get_tck() const {
    return this->_msys_p->GetTCK();
}

int MemorySystemWrapper::get_bus_bits() const {
    return this->_msys_p->GetBusBits();
}

int MemorySystemWrapper::get_burst_length() const {
    return this->_msys_p->GetBurstLength();
}