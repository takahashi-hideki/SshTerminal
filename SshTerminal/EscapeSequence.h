#ifndef _ESCAPESEQUENCE_H
#define _ESCAPESEQUENCE_H

int analysis_escapesequence(char* string, int start_index, char** event_position);
int analysis_escapesequence_app_mode(const char* string, char** buffer, int* cursor, const char** event_position, int line_width, int line_height, int* app_region);

#endif