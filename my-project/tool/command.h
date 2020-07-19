#include <stdint.h>

/* For register_command in other module
extern struct {char* command_string; void (*execution)(int, char **);} command [10];
extern uint8_t command_index;

#define register_command(cmd, func) \
command[command_index].command_string = #cmd; \
command[command_index].execution = &func; \
command_index++
*/

int run_command(char* cmd_string);

void int_command(void);