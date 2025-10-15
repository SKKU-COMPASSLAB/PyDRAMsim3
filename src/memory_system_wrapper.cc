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
    char *output_dir,
    int cmd_queue_num
): _cmd_queue_num(cmd_queue_num) {
    this->_config_file_raw = strdup(config_file);
    this->_output_dir_raw = strdup(output_dir);

    this->_msys_p = new dramsim3::MemorySystem(
        this->_config_file_raw, this->_output_dir_raw,
        std::bind(&MemorySystemWrapper::read_callback, this, std::placeholders::_1),
        std::bind(&MemorySystemWrapper::write_callback, this, std::placeholders::_1)
    );

    this->_transfer_size = (this->_msys_p->GetBusBits() / 8) * this->_msys_p->GetBurstLength();
    this->_cmd_queue.resize(this->_cmd_queue_num);
}

MemorySystemWrapper::~MemorySystemWrapper() {
    free(this->_config_file_raw);
    free(this->_output_dir_raw);
    delete this->_msys_p;
}

bool MemorySystemWrapper::dispatch_command(MemorySystemCommand *cmd_p) {
    // if (this->_current_dispatched_cmd_p != NULL) {
    //     this->_suspended_cmds.push_back(cmd_p);
    //     return false;
    // }
    
    // this->_current_dispatched_cmd_p = cmd_p;
    // this->_current_dispatched_cmd_p->n_req = (this->_current_dispatched_cmd_p->size + this->_transfer_size - 1) / this->_transfer_size;
    // this->_current_dispatched_cmd_p->dispatch_progress = 0;
    // this->_current_dispatched_cmd_p->execute_progress = 0;

    // if (this->_current_dispatched_cmd_p->dispatch_callback) {
    //     this->_current_dispatched_cmd_p->dispatch_callback((void *)cmd_p);  // call dispatch callback immediately
    // }

    auto cmd_q_id = cmd_p->cmd_q_id;
    this->_cmd_queue[cmd_q_id].push_back(cmd_p);
    
    return true;
}

void MemorySystemWrapper::cycle_step() {
    _msys_p->ClockTick();

    for (int qid = 0; qid < this->_cmd_queue_num; qid++) {
        if (this->_cmd_queue[qid].empty())
            continue;

        auto cmd_p = this->_cmd_queue[qid].front();
        const auto is_write = cmd_p->is_write;
        const auto n_req    = cmd_p->n_req;
        const auto req_addr = cmd_p->addr + (cmd_p->dispatch_progress * this->_transfer_size);

        if (_msys_p->WillAcceptTransaction(req_addr, is_write)) {
            _msys_p->AddTransaction(req_addr, is_write);

            if (!is_write) {
                if (this->_ongoing_rd_req_cmd_map.find(req_addr) == this->_ongoing_rd_req_cmd_map.end())
                    this->_ongoing_rd_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
                this->_ongoing_rd_req_cmd_map[req_addr].push(cmd_p);
            } else {
                if (this->_ongoing_wr_req_cmd_map.find(req_addr) == this->_ongoing_wr_req_cmd_map.end())
                    this->_ongoing_wr_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
                this->_ongoing_wr_req_cmd_map[req_addr].push(cmd_p);
            }
            
            cmd_p->dispatch_progress++;

            if (cmd_p->dispatch_progress >= n_req) {
                if (cmd_p->dispatch_callback) {
                    cmd_p->dispatch_callback((void *)cmd_p);
                }
                this->_cmd_queue[qid].erase(this->_cmd_queue[qid].begin());
            }
        }
    }

    // if (this->_current_dispatched_cmd_p) {
    //     const auto is_write = this->_current_dispatched_cmd_p->is_write;
    //     const auto n_req    = this->_current_dispatched_cmd_p->n_req;
    //     const auto req_addr = this->_current_dispatched_cmd_p->addr + (this->_current_dispatched_cmd_p->dispatch_progress * this->_transfer_size);

    //     if (_msys_p->WillAcceptTransaction(req_addr, is_write)) {
    //         _msys_p->AddTransaction(req_addr, is_write);

    //         if (!is_write) {
    //             if (this->_ongoing_rd_req_cmd_map.find(req_addr) == this->_ongoing_rd_req_cmd_map.end())
    //                 this->_ongoing_rd_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
    //             this->_ongoing_rd_req_cmd_map[req_addr].push(this->_current_dispatched_cmd_p);
    //         } else {
    //             if (this->_ongoing_wr_req_cmd_map.find(req_addr) == this->_ongoing_wr_req_cmd_map.end())
    //                 this->_ongoing_wr_req_cmd_map[req_addr] = std::queue<MemorySystemCommand *>();
    //             this->_ongoing_wr_req_cmd_map[req_addr].push(this->_current_dispatched_cmd_p);
    //         }
            
    //         this->_current_dispatched_cmd_p->dispatch_progress++;

    //         if (this->_current_dispatched_cmd_p->dispatch_progress >= n_req) {
    //             this->_current_dispatched_cmd_p = NULL;
    //         }
    //     }
    // }

    // if (this->_current_dispatched_cmd_p == NULL && !this->_suspended_cmds.empty()) {
    //     auto next_cmd_p = this->_suspended_cmds.front();
    //     this->_suspended_cmds.erase(this->_suspended_cmds.begin());
    //     this->dispatch_command(next_cmd_p);
    // }
}

void MemorySystemWrapper::read_callback(uint64_t addr) {
    if (this->_ongoing_rd_req_cmd_map.find(addr) == this->_ongoing_rd_req_cmd_map.end())
        return;

    while (this->_ongoing_rd_req_cmd_map[addr].size()) {
        MemorySystemCommand *cmd_p = this->_ongoing_rd_req_cmd_map[addr].front();
        cmd_p->execute_progress++;
        if (cmd_p->execute_progress >= cmd_p->n_req && cmd_p->execute_callback) {
            cmd_p->execute_callback((void *)cmd_p);
        }
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
        if (cmd_p->execute_progress >= cmd_p->n_req && cmd_p->execute_callback) {
            cmd_p->execute_callback((void *)cmd_p);
        }
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