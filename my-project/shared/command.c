#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define register_command(cmd, func) \
command[command_index].command_string = cmd; \
command[command_index].execution = &func; \
command_index++

// Support up to 10 command
struct {char* command_string; void (*execution)(int, char **);} command [10];
uint8_t command_index = 0;
void int_command(void);
int run_command(char* cmd_string);

#define test_code 0
#if (test_code == 1)
#define fp stdout
#else
extern FILE *fp;
#endif

void multiplication(int a, int b);
void multiplication(int a, int b){
	fprintf(fp, "Multiplication result: %hi\n\r",a*b);
}

static void test(int argc, char **argv);
static void test(int argc, char **argv)
{
	int i;
	for( i = 0; i < argc; i++)
		printf("In Func Ptr: argv%hi = %s\n", i, argv[i]);
}

static void abc(int argc, char **argv);
static void abc(int argc, char **argv){
	printf("In command name: %s\n", argv[0]);
	multiplication(atoi(argv[1]), atoi(argv[2]));
	if(argc > 3 || argc < 2) fprintf(fp, "Warning: Only support 2 arguments\n\r");
}

void int_command(void){
	/* register_command("module-name command-name", func-name) */
	register_command("test cmd", test);
	register_command("blabla mul", abc);
}

int run_command(char* cmd_string)
{
	char *arg[10]; // Support up to 9 argument not included command-name
	uint8_t argc;
	char* arg_data;
	char* enter_ptr;
	int i;
	uint8_t len;
	uint8_t strcmp_result;

	enter_ptr = strchr(cmd_string, '\n');
	if(enter_ptr != NULL) *enter_ptr = '\0'; // Remove Enter character
	while(cmd_string[0] == ' ')
		cmd_string++;
	len = strlen(cmd_string);
	while(cmd_string[len - 1] == ' ') {
		cmd_string[len - 1] = '\0';
		len = strlen(cmd_string);
	}
	printf("String: %s\n", cmd_string);
	for(i = 0; i < command_index; i++){
		strcmp_result = strcmp(cmd_string, command[i].command_string);
		strcmp_result = (strcmp_result > 0) ? !(cmd_string[strlen(command[i].command_string)] == ' ') : strcmp_result;
		printf("strcmp_result: %hu\n", strcmp_result);

		if(!strcmp_result) {
			//Split to multi string
			char* current_pos;
			char* offset;
			char* other_offset;
			arg_data = malloc(len + 1);
			strcpy(arg_data, cmd_string);
			current_pos = arg_data;

			for (argc = 0; (current_pos = strchr(current_pos, ' '))!= NULL; argc++){

				do {
					*current_pos = '\0';
				} while(*(++current_pos) == ' ');

				// Find "'" character
				offset = strchr((current_pos), '\'');
				// Find '"' character
				other_offset = strchr((current_pos), '\"');

				if (offset != NULL && other_offset != NULL)
					if(other_offset < offset){
						goto Double_quotes;
					}

				if(offset != NULL) {
					if(offset == current_pos) {
						arg[argc] = ++current_pos;
						// Find the next "'" character
						offset = strchr(current_pos, '\'');
						if(offset != NULL) {
							if(*(offset + 1) != ' ' && *(offset + 1) != '\0')
								goto Syntax_error;
							// Not include single quotes
							*offset = '\0';
							// Ignore the spaces in single quotes
							current_pos = offset + 1;
						}
						else {
							goto Syntax_error;
						}
					}
					else
						goto Normal_arg;
				}
				else if (other_offset != NULL) {

					Double_quotes:

					if(other_offset == current_pos) {
						arg[argc] = ++current_pos;
						// Find the next '"' character
						other_offset = strchr(current_pos, '\"');
						if(other_offset != NULL) {
							if(*(other_offset + 1) != ' ' && *(other_offset + 1) != '\0')
								goto Syntax_error;
							// Not include double quotes
							*other_offset = '\0';
							// Ignore the spaces in double quotes
							current_pos = other_offset + 1;
						}
						else {

							Syntax_error:

							fprintf(fp, "Command syntax error\r\n");
							goto Restore_the_string;
						}
					}
					else
						goto Normal_arg;
				}
				else

					Normal_arg:

					arg[argc] = current_pos;
			}

			// Execution the command
			command[i].execution(argc, arg);
			free(arg_data);
			return 1; // Notify that this string is a command
		}
	}

// Restore the String when string is not a command
	Restore_the_string:
// Can't restore the quotes
	// for(char* tmp_ptr = cmd_string; tmp_ptr < enter_ptr; tmp_ptr++){
	// 	if(*tmp_ptr == '\0')
	// 		*tmp_ptr = ' ';
	// }

	// if(enter_ptr != NULL)
	//  	*enter_ptr = '\n';

	return 0;
}

#if (test_code == 1)
int main(int argc, char *argv[])
{
	char local_buf[100];
	char* local_ptr;
	int_command();

	printf("For testting ...\nPlease type some thing: ");
	rewind(stdin);
	local_ptr = fgets(local_buf, 100, stdin);
	if(!run_command(local_buf)) printf("Not a command: %s\n", local_ptr);
}
#endif