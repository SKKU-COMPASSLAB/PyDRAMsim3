#include <iostream>
#include "memory_system_wrapper.h"
#include "py_interface.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    // MemorySystemCommand *cmd = new MemorySystemCommand{0x1000, 64, false, 0, 0, 0};
    // MemorySystemWrapper ms_wrapper(argv[1], "./.tmp");

    // ms_wrapper.dispatch_command(cmd);

    // for (int i = 0; i < 1000; i++) {
    //     ms_wrapper.cycle_step();

    //     if (check_msys_cmd_executed(cmd)) {
    //         std::cout << "Command executed at cycle " << i << std::endl;
    //         break;
    //     }
    // }

    // delete cmd;

    auto msys_p = pydramsim3_create_msys(argv[1], "./.tmp");
    auto cmd_p  = pydramsim3_create_msys_cmd(0x1000, 64, false);

    pydramsim3_msys_dispatch_cmd(msys_p, cmd_p);

    for (int i = 0; i < 1000; i++) {
        pydramsim3_msys_cycle_step(msys_p);

        if (pydramsim3_check_msys_cmd_executed(cmd_p)) {
            std::cout << "Command executed at cycle " << i << std::endl;
            break;
        }
    }

    pydramsim3_destroy_msys_cmd(cmd_p);
    pydramsim3_destroy_msys(msys_p);

    return 0;
}