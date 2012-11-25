#ifndef _SSH_TERMINAL_TYPEDEF_H
#define _SSH_TERMINAL_TYPEDEF_H

#ifndef Boolean
#define Boolean int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//escapesequence event code
#define WRAP_AROUND_MODE 1
#define APPLICATION_CURSOR_MODE 2
#define LET_OFF_KEYBOARD_MODE_ON 3
#define CURSOR_POSITION_MOVE_TO_HOME 4
#define APP_MODE_ON 5
#define LET_OFF_KEYBOARD_MODE_OFF 6
#define APP_MODE_OFF 7

#endif