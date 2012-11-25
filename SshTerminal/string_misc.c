#include "string_misc.h"
#include <string.h>
#include <stdlib.h>

char* get_a_line(char* string)
{
    char* new_line_code;
    char* last_char_of_line;
    
    new_line_code = strchr(string, '\r');
    if(new_line_code == NULL)
    {
        last_char_of_line = string;
        last_char_of_line += (strlen(string) - 1);
    }
    else
    {
        last_char_of_line = new_line_code;
        if(*(last_char_of_line + 1) == '\n')
        {
            last_char_of_line++;
        }
    }
    return last_char_of_line;
}

void remove_char(char* string, char c)
{
    int i = 0;
    int move = 0;
    while(!(string[i] == '\0'))
    {
        if(string[i] == c)
        {
            move++;
        }
        else
        {
            string[i - move] = string[i];
        }
        i++;
    }
    string[i - move] = '\0';
}

void remove_head_char(char* string)
{
    int i = 0;
    while(!(string[i] == '\0'))
    {
        string[i] = string[i+1];
        i++;
    }
}

void remove_n_char(char* string, int n)
{
    if(strlen(string) < n)
    {
        return;
    }
    
    int i = 0;
    while(!(string[i+n-1] == '\0'))
    {
        string[i] = string[i+n];
        i++;
    }
}