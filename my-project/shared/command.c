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

// Test function
extern FILE *fp;
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
//	char* arg_data;
	char* enter_ptr;
	int i;
	uint8_t len;

	enter_ptr = strchr(cmd_string, '\n');
	if(enter_ptr != NULL) *enter_ptr = '\0'; // Remove Enter character
	len = strlen(cmd_string);
	printf("String: %s\n", cmd_string);
	for(i = 0; i < command_index; i++){
		if(strstr(cmd_string, command[i].command_string) == cmd_string){
			char tmp = cmd_string[len];
			if(tmp == ' ' || tmp == '\0'){
				//Split to multi string
				char* current_pos;
				char* offset;
				char* other_offset;
				char* next_pos;
//				arg_data = malloc(len + 1);
//				strcpy(arg_data, cmd_string);
//				current_pos = arg_data;
				current_pos = cmd_string;

				for (argc = 0; (current_pos = strchr(current_pos, ' '))!= NULL; argc++){

					*current_pos = '\0';
					arg[argc] = current_pos + 1;

					// Find "'" character
					offset = strchr((current_pos), '\'');
					// Find '"' character
					other_offset = strchr((current_pos), '\"');

					next_pos = strchr(current_pos, ' ');

					if (offset != NULL && other_offset != NULL)
						if(other_offset < offset){
							goto Double_quotes;
						}

					if(offset != NULL) {
						// Don't ignore the spaces before single quotes
						if( next_pos > offset) {
							if(offset != arg[argc])
								goto Syntax_error;
							arg[argc] = offset + 1;
							// Find the next "'" character
							offset = strchr((offset + 1), '\'');
							if(offset != NULL) {
								// Not include single quotes
								*offset = '\0';
								// Ignore the spaces in single quotes
								current_pos = offset + 1;
							}
							else {
								goto Syntax_error;
							}
						}
					}
					else if (other_offset != NULL) {

						Double_quotes:

						// Don't ignore the spaces before double quotes
						if(next_pos > other_offset) {

							if(other_offset != arg[argc])
								goto Syntax_error;
							arg[argc] = other_offset + 1;
							// Find the next '"' character
							other_offset = strchr((other_offset + 1), '\"');
							if(other_offset != NULL) {
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
					}
				}
				// Execution the command
				command[i].execution(argc, arg);
//				free(arg_data);
				return 1; // Notify that this string is a command
			}
		}
	}

// Restore the String when string is not a command
	Restore_the_string:

	for(i = 0; i < len; i++){
		if(*(cmd_string + i) == '\0')
			*(cmd_string + i) = ' ';
	}

	if(enter_ptr != NULL)
	 	*enter_ptr = '\n';

	return 0;
}