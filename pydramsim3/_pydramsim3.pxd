cdef extern from "py_interface.h":
    ctypedef unsigned long long msys_data_t
    ctypedef void (* callback_t)(void *cmd_p)

    void *pydramsim3_create_msys(char *config_file, char *output_dir, int cmd_queue_num) nogil
    void  pydramsim3_destroy_msys(void *msys) nogil

    void *pydramsim3_create_msys_cmd(int cmd_q_id, msys_data_t addr, msys_data_t size, bint is_write) nogil
    void  pydramsim3_destroy_msys_cmd(void *cmd_p) nogil

    bint  pydramsim3_check_msys_cmd_dispatched(void *cmd_p) nogil
    bint  pydramsim3_check_msys_cmd_executed(void *cmd_p) nogil
    int   pydramsim3_get_expected_cmd_cycles(void *msys_p, void *cmd_p) nogil

    bint  pydramsim3_msys_dispatch_cmd(void *msys, void *cmd_p, callback_t dispatch_callback, callback_t execute_callback) nogil
    void  pydramsim3_msys_cycle_step(void *msys) nogil
    double pydramsim3_msys_get_tck(void *msys) nogil
