#ifndef _DATA_ACCESS_CONTROLLER_H
#define _DATA_ACCESS_CONTROLLER_H

#include "ori_list.h"
#include "ori_thread.h"
#include "libssh2.h"

typedef struct SSH_SESSION SSH_SESSION;
typedef struct SSH_SESSION_PARAM SSH_SESSION_PARAM;
typedef struct ADD_LIST ADD_LIST;
typedef struct COMMAND_LIST COMMAND_LIST;
typedef struct THREAD_PARAM THREAD_PARAM;

struct ADD_LIST
{
    ORI_DATA* p_next;
    int interval;
    ORI_LIST* p_list;
};

struct COMMAND_LIST
{
    ORI_DATA* p_next;
    char* command;
};

struct SSH_SESSION_PARAM
{
    int sock;
    LIBSSH2_SESSION* session;
    LIBSSH2_CHANNEL* channel;
    Boolean login_flag;
    Boolean async_mode;
    int channel_type;
};

struct THREAD_PARAM
{
    void* exec_command_thread;
    void* command_timer_thread;
    void* command_queue_mutex;
    void* add_list_mutex;
    void* command_queue_condition;
    void* add_list_condition;
    
    Boolean exec_command_thread_flag;
    Boolean command_timer_thread_flag;
};

struct SSH_SESSION
{
    SSH_SESSION_PARAM* session_param;
    ORI_LIST* add_list;
    ORI_LIST* command_queue;
    int (*callback)(char*, const char*, void*);
    void* obj;
    
    THREAD_PARAM* thread_param;
};

void safety_queue_push_back(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list);
void safety_queue_push_back_with_condition(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, void* condition);
void safety_add_list_insert(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, int interval);
void safety_add_list_insert_with_condition(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, int interval, void* condition);
void safety_queue_push_back_and_list(void* queue_mutex, void* list_mutex, ORI_LIST* add_list, ORI_LIST* queue, int timecount);
void safety_queue_push_back_and_list_with_condition(void* queue_mutex, void* list_mutex, ORI_LIST* add_list, ORI_LIST* queue, int timecount, void* queue_condition, void* list_condition, void* thread);
int safety_queue_pop_front(void* mutex, ORI_LIST* queue, COMMAND_LIST* p_data);
COMMAND_LIST* safety_queue_pop_front_with_condition(void* mutex, ORI_LIST* queue, void* condition, void* thread);

void wakeup_condition(void* mutex, void* condition);

void release_add_list_data(ORI_DATA* p_data);
void release_command_list_data(ORI_DATA* p_data);
void command_queue_init(SSH_SESSION* ssh_session);
void add_list_init(SSH_SESSION* ssh_session);
void command_list_init(ADD_LIST* add_list);
COMMAND_LIST* create_command_data(const char* command);
ADD_LIST* create_add_data(int interval);

#endif