# cython: language_level=3
import cython
from cpython.bytes cimport PyBytes_AsString
from cpython.pycapsule cimport PyCapsule_New, PyCapsule_GetPointer, PyCapsule_Destructor
from ._pydramsim3 cimport (
    msys_data_t,
    pydramsim3_create_msys, pydramsim3_destroy_msys,
    pydramsim3_create_msys_cmd, pydramsim3_destroy_msys_cmd,
    pydramsim3_check_msys_cmd_dispatched, pydramsim3_check_msys_cmd_executed, pydramsim3_get_expected_cmd_cycles,
    pydramsim3_msys_dispatch_cmd, pydramsim3_msys_cycle_step, pydramsim3_msys_get_tck
)


cdef bytes MSYS_CAPSULE_NAME = b"pydramsim3.msys"
cdef bytes CMD_CAPSULE_NAME  = b"pydramsim3.cmd"

cdef void msys_capsule_destructor(capsule) except *:
    cdef void* p = PyCapsule_GetPointer(capsule, MSYS_CAPSULE_NAME)
    if p != NULL:
        pydramsim3_destroy_msys(p)

cdef void cmd_capsule_destructor(capsule) except *:
    cdef void* p = PyCapsule_GetPointer(capsule, CMD_CAPSULE_NAME)
    if p != NULL:
        pydramsim3_destroy_msys_cmd(p)


@cython.boundscheck(False)
@cython.wraparound(False)
def create_msys(config_file, output_dir):
    cdef char *config_file_p = PyBytes_AsString(config_file.encode("ascii"))
    cdef char *output_dir_p  = PyBytes_AsString(output_dir.encode("ascii"))
    cdef void *msys_p = pydramsim3_create_msys(config_file_p, output_dir_p)

    cap = PyCapsule_New(msys_p, MSYS_CAPSULE_NAME, <PyCapsule_Destructor> msys_capsule_destructor)
    return cap

@cython.boundscheck(False)
@cython.wraparound(False)
def create_msys_cmd(msys_data_t addr, msys_data_t size, bint is_write):
    cdef void *cmd_p = pydramsim3_create_msys_cmd(addr, size, is_write)

    cap = PyCapsule_New(cmd_p, CMD_CAPSULE_NAME, <PyCapsule_Destructor> cmd_capsule_destructor)
    return cap


@cython.boundscheck(False)
@cython.wraparound(False)
def check_msys_cmd_dispatched(cmd):
    cdef void *cmd_p = PyCapsule_GetPointer(cmd, CMD_CAPSULE_NAME)
    cdef bint flag = pydramsim3_check_msys_cmd_dispatched(cmd_p)
    return flag

@cython.boundscheck(False)
@cython.wraparound(False)
def check_msys_cmd_executed(cmd):
    cdef void *cmd_p = PyCapsule_GetPointer(cmd, CMD_CAPSULE_NAME)
    cdef bint flag = pydramsim3_check_msys_cmd_executed(cmd_p)
    return flag

@cython.boundscheck(False)
@cython.wraparound(False)
def get_expected_cmd_cycles(msys, cmd):
    cdef void *msys_p = PyCapsule_GetPointer(msys, MSYS_CAPSULE_NAME)
    cdef void *cmd_p = PyCapsule_GetPointer(cmd, CMD_CAPSULE_NAME)
    cdef int cycles = pydramsim3_get_expected_cmd_cycles(msys_p, cmd_p)
    return cycles
    


@cython.boundscheck(False)
@cython.wraparound(False)
def msys_dispatch_cmd(msys, cmd):
    cdef void *msys_p = PyCapsule_GetPointer(msys, MSYS_CAPSULE_NAME)
    cdef void *cmd_p = PyCapsule_GetPointer(cmd, CMD_CAPSULE_NAME)
    cdef bint flag = pydramsim3_msys_dispatch_cmd(msys_p, cmd_p)
    return flag

@cython.boundscheck(False)
@cython.wraparound(False)
def msys_cycle_step(msys, int cycles):
    cdef void *msys_p = PyCapsule_GetPointer(msys, MSYS_CAPSULE_NAME)
    for _ in range(cycles):
        pydramsim3_msys_cycle_step(msys_p)

@cython.boundscheck(False)
@cython.wraparound(False)
def msys_get_tck(msys):
    cdef void *msys_p = PyCapsule_GetPointer(msys, MSYS_CAPSULE_NAME)
    return pydramsim3_msys_get_tck(msys_p)
