#include "escapesequence.h"

#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "string_misc.h"
#include "ssh_terminal_typedef.h"
#include <stdio.h>

void delete_back_char(char* string);
int analysis_vt100_escapesequence(char* string, int* event_code);
int analysis_vt100_escapesequence_app_mode(const char* string, char** buffer, int* cursor, int* event_code, int line_width, int line_height, int* app_region);
int cursor_move_app_mode(char** buffer, int* cursor, int line_width, int line_height, int cursor_x, int cursor_y);

void delete_back_char(char* string)
{
    char* backward;
    backward = string - 1;
    remove_n_char(backward, 2);
    return;
}

int analysis_vt100_escapesequence(char* string, int* event_code)
{
    int pointer_move;
    regex_t regst;
    regmatch_t pmatch[1];
    Boolean match = FALSE;
    *event_code = 0;
    
    remove_head_char(string);  //delete '\e'
    if(strncmp(string, "[K", 2) == 0){  //clear at pointer to line end
        remove_n_char(string, 2);
        pointer_move = 0;
    }
    else
    if(strncmp(string, "(B\e)0", 5) == 0){  //enable another character code set
        remove_n_char(string, 5);
        pointer_move = 0;
    }
    else
    if(strncmp(string, "[?7h", 4) == 0){  //wrap around mode on
        remove_n_char(string, 4);
        pointer_move = 0;
        *event_code = WRAP_AROUND_MODE;
    }
    else
    if(strncmp(string, "[?1h\e=", 6) == 0){  //let off keyboard mode on
        remove_n_char(string, 6);
        pointer_move = 0;
        //*event_code = LET_OFF_KEYBOARD_MODE_ON;
        *event_code = APP_MODE_ON;
    }
    else
    if(strncmp(string, "[?1l\e>", 6) == 0){  //let off keyboard mode off
        remove_n_char(string, 6);
        pointer_move = 0;
        //*event_code = LET_OFF_KEYBOARD_MODE_OFF;
        *event_code = APP_MODE_OFF;
    }
    else
    if(strncmp(string, "[J", 2) == 0){  //clear view
        remove_n_char(string, 2);
        pointer_move = 0;
    }
    else
    if(strncmp(string, "[0J", 3) == 0){  //cler view
        remove_n_char(string, 3);
        pointer_move = 0;
    }
    else
    if(strncmp(string, "[H", 2) == 0){  //move cursor to home position
        remove_n_char(string, 2);
        pointer_move = 0;
        *event_code = CURSOR_POSITION_MOVE_TO_HOME;
    }
    else
    {
        pointer_move = 0;
        //[m;nr
        //set region line m to n
        regcomp(&regst, "\[[0-9]+;[0-9]+r", REG_EXTENDED);
        if(regexec(&regst, string, 1, pmatch, 0) == 0)
        {
            if(pmatch[0].rm_so == 0)
            {
                remove_n_char(string, (pmatch[0].rm_eo - pmatch[0].rm_so));
                pointer_move = 0;
                match = TRUE;
                //*event_code = APP_MODE_ON;
            }
        }
        regfree(&regst);
        
        if(!match)
        {
            //[m or [nm
            //set character attribute
            regcomp(&regst, "\[[0-9]*m", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    remove_n_char(string, (pmatch[0].rm_eo - pmatch[0].rm_so));
                    pointer_move = 0;
                    match = TRUE;
                }
            }
            
            regfree(&regst);
        }
        
        if(!match)
        {
            //[m;nH
            //move cursor to [m][n]
            regcomp(&regst, "\[[0-9]+;[0-9]+H", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    remove_n_char(string, (pmatch[0].rm_eo - pmatch[0].rm_so));
                    pointer_move = 0;
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        
        if(!match)
        {
            //[nG
            //node
            regcomp(&regst, "\[[0-9]+G", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    remove_n_char(string, (pmatch[0].rm_eo - pmatch[0].rm_so));
                    pointer_move = 0;
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        
        if(!match)
        {
            fprintf(stderr, "not match!\n");
        }
    }
    
    return pointer_move;
}

int analysis_vt100_escapesequence_app_mode(const char* string, char** buffer, int* cursor, int* event_code, int line_width, int line_height, int* app_region)
{
    int pointer_move;
    regex_t regst;
    regmatch_t pmatch[2];
    Boolean match = FALSE;
    int digit;
    int cursor_x;
    int cursor_y;
    int region_start;
    int region_end;
    int cur_digit;
    *event_code = 0;
    char* line;
    char* cur_position;
    char number_char[10] = "";
    int i, j;

    if(strncmp(string, "[K", 2) == 0){  //clear at cursor position to line end
        pointer_move = 2;
        line = &(buffer[cursor[0]][cursor[1]]);
        memset(line, '\0', strlen(line));
        *line = ' ';
    }
    else
    if(strncmp(string, "(B\e)0", 5) == 0){  //enable another character code set
        pointer_move = 5;
    }
    else
    if(strncmp(string, "[H", 2) == 0){  //cursor move to home position([0][0])
        pointer_move = 2;
        cursor[0] = 0;
        cursor[1] = 0;
    }
    else
    if(strncmp(string, "[?7h", 4) == 0){  //wrap around mode on
        pointer_move = 4;
        *event_code = WRAP_AROUND_MODE;
    }
    else
    if(strncmp(string, "[?1h\e=", 6) == 0){  //let off keyboard mode on
        pointer_move = 6;
        //*event_code = LET_OFF_KEYBOARD_MODE_ON;
        *event_code = APP_MODE_ON;
    }
    else
    if(strncmp(string, "[?1l\e>", 6) == 0){  //let off keyboard mode off
        pointer_move = 6;
        //*event_code = LET_OFF_KEYBOARD_MODE_OFF;
        *event_code = APP_MODE_OFF;
    }
    else
    if(strncmp(string, "[J", 2) == 0){  //clear log to end
        pointer_move = 2;
        cur_position = &(buffer[cursor[0]][cursor[1]]);
        memset(cur_position, '\0', strlen(cur_position));
        
        for(i = cursor[0] + 1; i < line_height; i++)
        {
            memset(buffer[i], '\0', line_width);
        }
        *cur_position = ' ';
    }
    else
    if(strncmp(string, "M", 1) == 0){  //scroll down text
        pointer_move = 1;
        if(cursor[0] != app_region[0])
        {
            cursor[0]--;
            while(buffer[cursor[0]][cursor[1]] == '\0')
            {
                cursor[1]--;
            }
        }
        else
        {
            //regionのデータを１行ずつ下にずらす
            for(i = app_region[1]; i > app_region[0]; i--)
            {
                memset(buffer[i], '\0', strlen(buffer[i]));
                strcpy(buffer[i], buffer[i-1]);
            }
            memset(buffer[app_region[0]], '\0', strlen(buffer[app_region[0]]));
            buffer[app_region[0]][0] = ' ';
        }
    }
    else
    if(strncmp(string, "7", 1) == 0){  //save current cursor position
        pointer_move = 1;
    }
    else
    if(strncmp(string, "8", 1) == 0){  //load cursor position from save cursor
        pointer_move = 1;
    }
    else
    {
        pointer_move = 0;
        //[m;nH
        //move cursor to [m][n]
        regcomp(&regst, "\[[0-9]+;[0-9]+H", REG_EXTENDED);
        if(regexec(&regst, string, 1, pmatch, 0) == 0)
        {
            if(pmatch[0].rm_so == 0)
            {
                pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                for(i = 1; i < pointer_move; i++)
                {
                    if(string[i] != ';')
                    {
                        number_char[i-1] = string[i];
                    }
                    else
                    {
                        number_char[i-1] = '\0';
                        break;
                    }
                }
                cursor_y = atoi(number_char) - 1;
                
                for(j = 1; i + j < pointer_move; j++)
                {
                    if(string[i+j] != 'H')
                    {
                        number_char[j-1] = string[i+j];
                    }
                    else
                    {
                        number_char[j-1] = '\0';
                        break;
                    }
                }
                cursor_x = atoi(number_char) - 1;
                cursor_move_app_mode(buffer, cursor, line_width, line_height, cursor_x, cursor_y);
                match = TRUE;
            }
        }
        regfree(&regst);
        
        if(!match)
        {
            //[m or [nm
            //set character attribute
            regcomp(&regst, "\[[0-9]*m", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        
        if(!match)
        {
            //[nA
            //cursor positon up n line
            regcomp(&regst, "\[[0-9]+A", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    digit = pointer_move - 2;
                    for(i = 1; i < digit + 1; i++)
                    {
                        number_char[i-1] = string[i];
                    }
                    number_char[i-1] = '\0';
                    
                    cursor_y = cursor[0] - atoi(number_char);
                    if(cursor_y < 0)
                    {
                        cursor[0] = cursor_y;
                    }
                    else
                    {
                        cursor[0] = 0;
                    }
                    while(buffer[cursor[0]][cursor[1]] == '\0')
                    {
                        cursor[1]--;
                    }
                    match = TRUE;
                }
            }
            regfree(&regst);
        }

        
        if(!match)
        {
            //[nB
            //cursor positon down n line
            regcomp(&regst, "\[[0-9]+B", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    digit = pointer_move - 2;
                    for(i = 1; i < digit + 1; i++)
                    {
                        number_char[i-1] = string[i];
                    }
                    number_char[i-1] = '\0';
                    
                    cursor_y = atoi(number_char) + cursor[0];
                    cur_digit = cursor[1];
                    while(cursor[0] != cursor_y && cursor[0] < line_height - 1)
                    {
                        cursor[0]++;
                        cur_digit = cursor[1];
                        while(buffer[cursor[0]][cur_digit] == '\0')
                        {
                            if(cur_digit == 0)
                            {
                                buffer[cursor[0]][cur_digit] = ' ';
                                break;
                            }
                            cur_digit--;
                        }
                    }
                    cursor[1] = cur_digit;
                    //cursor[1] = 0;
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        
        if(!match)
        {
            //[nC
            //cursor positon move right n digit
            regcomp(&regst, "\[[0-9]+C", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    digit = pointer_move - 2;
                    for(i = 1; i < digit + 1; i++)
                    {
                        number_char[i-1] = string[i];
                    }
                    number_char[i-1] = '\0';
                    
                    cursor_x = atoi(number_char) + cursor[1];
                    if(cursor_x < line_width)
                    {
                        cursor[1] = cursor_x;
                    }
                    else
                    {
                        cursor[1] = line_width - 1;
                    }
                    cur_digit = cursor[1];
                    while(buffer[cursor[0]][cur_digit] == '\0')
                    {
                        buffer[cursor[0]][cur_digit] = ' ';
                        cur_digit--;
                    }
                    
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        
        if(!match)
        {
            //[nD
            //cursor positon move left n digit
            regcomp(&regst, "\[[0-9]+D", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    digit = pointer_move - 2;
                    for(i = 1; i < digit + 1; i++)
                    {
                        number_char[i-1] = string[i];
                    }
                    number_char[i-1] = '\0';
                    cursor_x = cursor[1] - atoi(number_char);
                    if(cursor_x >= 0)
                    {
                        cursor[1] = cursor_x;
                    }
                    else
                    {
                        cursor[1] = 0;
                    }
                    
                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        if(!match)
        {
            //あとで実装?
            //[m;nr
            //set region line m to n
            regcomp(&regst, "\[[0-9]+;[0-9]+r", REG_EXTENDED);
            if(regexec(&regst, string, 1, pmatch, 0) == 0)
            {
                if(pmatch[0].rm_so == 0)
                {
                    pointer_move = pmatch[0].rm_eo - pmatch[0].rm_so;
                    for(i = 1; i < pointer_move; i++)
                    {
                        if(string[i] != ';')
                        {
                            number_char[i-1] = string[i];
                        }
                        else
                        {
                            number_char[i-1] = '\0';
                            break;
                        }
                    }
                    region_start = atoi(number_char) - 1;
                    
                    for(j = 1; i + j < pointer_move; j++)
                    {
                        if(string[i+j] != 'H')
                        {
                            number_char[j-1] = string[i+j];
                        }
                        else
                        {
                            number_char[j-1] = '\0';
                            break;
                        }
                    }
                    region_end = atoi(number_char) - 1;
                    app_region[0] = region_start;
                    app_region[1] = region_end;

                    match = TRUE;
                }
            }
            regfree(&regst);
        }
        if(!match)
        {
            fprintf(stderr, "yaaa\n");
        }
    }
    return pointer_move;
}

int analysis_escapesequence(char* string, int start_index, char** event_position)
{
    char* start = string + start_index;
    char* cur;
    int event_code;
    *event_position = NULL;
    
    int i = 0;
    int pointer_move;
    
    while(!(start[i] == '\0'))
    {
        if(start[i] == '\a'){ //bell code
            cur = start + i;
            remove_head_char(cur);
        }
        else
        if(start[i] == '\b'){ //delete backward code
            cur = start + i;
            delete_back_char(cur);
            i--;
        }
        else
        if(start[i] == '\x0f'){ //CTRL -o
            cur = start + i;
            remove_head_char(cur);
        }
        else
        if(start[i] == '\e'){ //vt100 escape sequence header
            cur = start + i;
            pointer_move = analysis_vt100_escapesequence(cur, &event_code);
            if(event_code != 0)
            {
                *event_position = cur;
                return event_code;
            }
            i += pointer_move;
        }
        else
        {
            i++;
        }
    }
    
    return 0;
}

int analysis_escapesequence_app_mode(const char* string, char** buffer, int* cursor, const char** event_position, int line_width, int line_height, int* app_region)
{
    int event_code;
    const char* cur;
    *event_position = NULL;
    int i = 0;
    int j;
    int pointer_move;
    
    while(!(string[i] == '\0'))
    {
        if(string[i] == '\a'){  //bell code
            i++;
        }
        else
        if(string[i] == '\b'){  //cursor back code
            i++;
            if(cursor[1] > 0)
            {
                cursor[1]--;
            }
            else
            if(cursor[0] > 0)
            {
                cursor[0]--;
                cursor[1] = line_width - 1;
                while(buffer[cursor[0]][cursor[1]] == '\0')
                {
                    cursor[1]--;
                }
            }
        }
        else
        if(string[i] == '\x0f'){  //CTRL -o
            i++;
        }
        else
        if(string[i] == '\r'){  //new line code
            i++;
            if(string[i] == '\n')
            {
                i++;
                if(cursor[0] != app_region[1])
                {
                    cursor[0]++;
                    cursor[1] = 0;
                    if(buffer[cursor[0]][cursor[1]] == '\0')
                    {
                        buffer[cursor[0]][cursor[1]] = ' ';
                    }
                }
                else
                {
                    //regionのデータを1行ずつ上にずらす
                    for(j = app_region[0]; j < app_region[1]; j++)
                    {
                        memset(buffer[j], '\0', strlen(buffer[j]));
                        strcpy(buffer[j], buffer[j+1]);
                    }
                    memset(buffer[app_region[1]], '\0', strlen(buffer[app_region[1]]));
                    cursor[1] = 0;
                    buffer[cursor[0]][cursor[1]] = ' ';

                }
            }
        }
        else
        if(string[i] == '\n'){  //new line code
            i++;
        }
        else
        if(string[i] == '\e'){  //vt100 escape sequence header
            i++;
            cur = string + i;
            pointer_move = analysis_vt100_escapesequence_app_mode(cur, buffer, cursor, &event_code, line_width, line_height, app_region);
            i += pointer_move;
            if(event_code != 0)
            {
                *event_position = string + i;
                return event_code;
            }
        }
        else{  //print character
            buffer[cursor[0]][cursor[1]] = string[i];
            if(cursor[1] < line_width - 1)
            {
                cursor[1]++;
                if(buffer[cursor[0]][cursor[1]] == '\0')
                {
                    buffer[cursor[0]][cursor[1]] = ' ';
                }
            }
            else
            if(cursor[0] < line_height - 1)
            {
                cursor[1] = 0;
                cursor[0]++;
                if(buffer[cursor[0]][cursor[1]] == '\0')
                {
                    buffer[cursor[0]][cursor[1]] = ' ';
                }
            }
            i++;
        }
    }
    
    return 0;
}

int cursor_move_app_mode(char** buffer, int* cursor, int line_width, int line_height, int cursor_x, int cursor_y)
{
    int cursor_move_y = cursor_y - cursor[0];
    int cur_x;
    
    if(cursor_x >= line_width || cursor_y >= line_height || cursor_x < 0 || cursor_y < 0)
    {
        return -1;
    }
    
    if(cursor_move_y < 0)
    {
        cursor[0] = cursor_y;
        cursor[1] = cursor_x;
        cur_x = cursor[1];
        while(buffer[cursor[0]][cur_x] == '\0')
        {
            buffer[cursor[0]][cur_x] = ' ';
            cur_x--;
        }
    }
    else
    if(cursor_move_y == 0)
    {
        cursor[1] = cursor_x;
        cur_x = cursor[1];
        while(buffer[cursor[0]][cur_x] == '\0')
        {
            buffer[cursor[0]][cur_x] = ' ';
            cur_x--;
        }
    }
    else
    {
        while(cursor[0] != cursor_y)
        {
            cursor[0]++;
            if(buffer[cursor[0]][0] == '\0')
            {
                buffer[cursor[0]][0] = ' ';
            }
        }
        cursor[1] = cursor_x;
        cur_x = cursor[1];
        while(buffer[cursor[0]][cur_x] == '\0')
        {
            buffer[cursor[0]][cur_x] = ' ';
            cur_x--;
            if(cur_x < 0)
            {
                break;
            }
        }
    }
    
    return 0;
    
}
