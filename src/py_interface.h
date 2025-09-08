#ifndef __PYDRAMSIM3_INTERFACE_H
#define __PYDRAMSIM3_INTERFACE_H

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long msys_data_t;


void *pydramsim3_create_msys(char *config_file, char *output_dir);
void  pydramsim3_destroy_msys(void *msys);

void *pydramsim3_create_msys_cmd(msys_data_t addr, msys_data_t size, char is_write);
void  pydramsim3_destroy_msys_cmd(void *cmd_p);

char  pydramsim3_check_msys_cmd_dispatched(void *cmd_p);
char  pydramsim3_check_msys_cmd_executed(void *cmd_p);
int   pydramsim3_get_expected_cmd_cycles(void *msys_p, void *cmd_p);

char  pydramsim3_msys_dispatch_cmd(void *msys, void *cmd_p);
void  pydramsim3_msys_cycle_step(void *msys);
double pydramsim3_msys_get_tck(void *msys);


#ifdef __cplusplus
}
#endif

#endif