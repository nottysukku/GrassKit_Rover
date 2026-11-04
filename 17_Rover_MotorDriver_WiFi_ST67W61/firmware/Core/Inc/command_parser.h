/**
  ******************************************************************************
  * @file           : command_parser.h
  * @brief          : Human-readable text protocol parser for phone and PC control.
  ******************************************************************************
  */

#ifndef __COMMAND_PARSER_H
#define __COMMAND_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void Command_Init(void);
void Command_Parse(char *cmd_str);
void Command_SendStatus(void);
void Command_SendWebStatus(void);
void Command_SendHelp(void);

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_PARSER_H */
