cdef extern from "py_interface.h":
    ctypedef unsigned long long msys_data_t

    void *pydramsim3_create_msys(char *config_file, char *output_dir) nogil
    void  pydramsim3_destroy_msys(void *msys) nogil

    void *pydramsim3_create_msys_cmd(msys_data_t addr, msys_data_t size, bint is_write) nogil
    void  pydramsim3_destroy_msys_cmd(void *cmd_p) nogil

    bint  pydramsim3_check_msys_cmd_dispatched(void *cmd_p) nogil
    bint  pydramsim3_check_msys_cmd_executed(void *cmd_p) nogil
    int   pydramsim3_get_expected_cmd_cycles(void *msys_p, void *cmd_p) nogil

    bint  pydramsim3_msys_dispatch_cmd(void *msys, void *cmd_p) nogil
    void  pydramsim3_msys_cycle_step(void *msys) nogil
    double pydramsim3_msys_get_tck(void *msys) nogil
