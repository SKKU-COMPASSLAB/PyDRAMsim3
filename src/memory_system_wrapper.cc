#include <iostream>
#include <algorithm>
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
    int max_issue_per_cmd_q_per_cycle
): _max_issue_per_cmd_q_per_cycle(std::max(1, max_issue_per_cmd_q_per_cycle)) {
    this->_config_file_raw = strdup(config_file);
    this->_output_dir_raw = strdup(output_dir);

    this->_msys_p = new dramsim3::MemorySystem(
        this->_config_file_raw, this->_output_dir_raw,
        std::bind(&MemorySystemWrapper::read_callback, this, std::placeholders::_1),
        std::bind(&MemorySystemWrapper::write_callback, this, std::placeholders::_1)
    );

    this->_transfer_size = (this->_msys_p->GetBusBits() / 8) * this->_msys_p->GetBurstLength();
    // this->_cmd_queue.resize(this->_cmd_queue_num);
}

MemorySystemWrapper::~MemorySystemWrapper() {
    free(this->_config_file_raw);
    free(this->_output_dir_raw);
    delete this->_msys_p;
}

bool MemorySystemWrapper::dispatch_command(MemorySystemCommand *cmd_p) {
    // auto cmd_q_id = cmd_p->cmd_q_id;
    // if (cmd_q_id < 0 || cmd_q_id >= this->_cmd_queue_num) {
    //     return false;
    // }
    // this->_cmd_queue[cmd_q_id].push_back(cmd_p);
    this->_inst_cmd_queue.push_back(cmd_p);
    
    return true;
}

void MemorySystemWrapper::cycle_step() {
    _msys_p->ClockTick();

    // if (this->_cmd_queue_num <= 0) {
    //     return;
    // }

    // const int start_qid = this->_rr_next_cmd_q_id;
    // this->_rr_next_cmd_q_id = (this->_rr_next_cmd_q_id + 1) % this->_cmd_queue_num;

    // for (int i = 0; i < this->_cmd_queue_num; i++) {
    // const int qid = (start_qid + i) % this->_cmd_queue_num;
    int issued_this_cycle = 0;

    while (!this->_inst_cmd_queue.empty() && issued_this_cycle < this->_max_issue_per_cmd_q_per_cycle) {
        auto cmd_p = this->_inst_cmd_queue.front();
        const auto is_write = cmd_p->is_write;
        const auto n_req    = cmd_p->n_req;
        const auto req_addr = cmd_p->addr + (cmd_p->dispatch_progress * this->_transfer_size);

        if (!_msys_p->WillAcceptTransaction(req_addr, is_write)) {
            break;
        }
        if (!_msys_p->AddTransaction(req_addr, is_write)) {
            break;
        }

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
        issued_this_cycle++;

        if (cmd_p->dispatch_progress >= n_req) {
            if (cmd_p->dispatch_callback) {
                cmd_p->dispatch_callback((void *)cmd_p);
            }
            // this->_cmd_queue[qid].erase(this->_cmd_queue[qid].begin());
            this->_inst_cmd_queue.erase(this->_inst_cmd_queue.begin());
        }
    }
    // }
}

void MemorySystemWrapper::read_callback(uint64_t addr) {
    if (this->_ongoing_rd_req_cmd_map.find(addr) == this->_ongoing_rd_req_cmd_map.end())
        return;

    auto &req_q = this->_ongoing_rd_req_cmd_map[addr];
    MemorySystemCommand *cmd_p = req_q.front();
    cmd_p->execute_progress++;
    if (cmd_p->execute_progress >= cmd_p->n_req && cmd_p->execute_callback) {
        cmd_p->execute_callback((void *)cmd_p);
    }
    req_q.pop();

    if (req_q.empty()) {
        this->_ongoing_rd_req_cmd_map.erase(addr);
    }
}

void MemorySystemWrapper::write_callback(uint64_t addr) {
    if (this->_ongoing_wr_req_cmd_map.find(addr) == this->_ongoing_wr_req_cmd_map.end())
        return;

    auto &req_q = this->_ongoing_wr_req_cmd_map[addr];
    MemorySystemCommand *cmd_p = req_q.front();
    cmd_p->execute_progress++;
    if (cmd_p->execute_progress >= cmd_p->n_req && cmd_p->execute_callback) {
        cmd_p->execute_callback((void *)cmd_p);
    }
    req_q.pop();

    if (req_q.empty()) {
        this->_ongoing_wr_req_cmd_map.erase(addr);
    }
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