#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#define COMMAND_LINE_MAX 64U
/* Debug Task only, non-reentrant. Input must be NUL-terminated; not modified.
 * Space/tab separated tokens, case-sensitive; quoting is not supported. */
void CommandParser_Parse(const char *line);
#endif
