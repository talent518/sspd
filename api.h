#ifndef API_H
#define API_H

#include <stdio.h>

char *gad(const char *argv0);

int execi(const char *cmd);
#define tput_cols() execi("tput cols")
#define tput_lines() execi("tput lines")

char *str_repeat(const char *str,size_t str_len,size_t repeat);
void strnprint(const char *str,size_t repeat);

#endif