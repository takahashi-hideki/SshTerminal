#include "data_access_controller.h"

#include <stdlib.h>
#include <string.h>

void safety_queue_push_back(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list)
{
    ori_thread_mutex_lock(mutex);
    p_list->push_back(p_list, p_data);
    ori_thread_mutex_unlock(mutex);
    return;
}

void safety_queue_push_back_with_condition(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, void* condition)
{
    ori_thread_mutex_lock(mutex);
    p_list->push_back(p_list, p_data);
    ori_thread_condition_signal(condition);
    ori_thread_mutex_unlock(mutex);
}

void safety_add_list_insert(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, int interval)
{
    int i;
    ADD_LIST* search_data;
    ADD_LIST* add_data;
    ori_thread_mutex_lock(mutex);
    for(i = 0; i < p_list->si_count; i++)
    {
        search_data = (ADD_LIST*)p_list->index_search(p_list, i);
        if(search_data->interval == interval)
        {
            search_data->p_list->push_back(search_data->p_list, p_data);
            break;
        }
        else
        if(search_data->interval > interval)
        {
            add_data = create_add_data(interval);
            add_data->p_list->push_back(add_data->p_list, p_data);
            p_list->insert_data(p_list, (ORI_DATA*)add_data, i);
            break;
        }
    }
    if(i == p_list->si_count)
    {
        add_data = create_add_data(interval);
        add_data->p_list->push_back(add_data->p_list, p_data);
        p_list->push_back(p_list, (ORI_DATA*)add_data);
    }
    ori_thread_mutex_unlock(mutex);
}

void safety_add_list_insert_with_condition(void* mutex, ORI_DATA* p_data, ORI_LIST* p_list, int interval, void* condition)
{
    int i;
    ADD_LIST* search_data;
    ADD_LIST* add_data;
    ori_thread_mutex_lock(mutex);
    for(i = 0; i < p_list->si_count; i++)
    {
        search_data = (ADD_LIST*)p_list->index_search(p_list, i);
        if(search_data->interval == interval)
        {
            search_data->p_list->push_back(search_data->p_list, p_data);
            break;
        }
        else
        if(search_data->interval > interval)
        {
            add_data = create_add_data(interval);
            add_data->p_list->push_back(add_data->p_list, p_data);
            p_list->insert_data(p_list, (ORI_DATA*)add_data, i);
            break;
        }
    }
    if(i == p_list->si_count)
    {
        add_data = create_add_data(interval);
        add_data->p_list->push_back(add_data->p_list, p_data);
        p_list->push_back(p_list, (ORI_DATA*)add_data);
    }
    ori_thread_condition_signal(condition);
    ori_thread_mutex_unlock(mutex);
}

void safety_queue_push_back_and_list(void* queue_mutex, void* list_mutex, ORI_LIST* add_list, ORI_LIST* queue, int timecount)
{
    int i, j;
    ADD_LIST* add_data;
    COMMAND_LIST* command_data;
    COMMAND_LIST* add_command_data;
    
    ori_thread_mutex_lock(queue_mutex);
    ori_thread_mutex_lock(list_mutex);
    for(i = 0; i < add_list->si_count; i++)
    {
        add_data = (ADD_LIST*)add_list->index_search(add_list, i);
        if(timecount % add_data->interval == 0)
        {
            for(j = 0; j < add_data->p_list->si_count; j++){
                command_data = (COMMAND_LIST*)add_data->p_list->p_data;
                add_command_data = create_command_data(command_data->command);
                queue->push_back(queue, (ORI_DATA*)add_command_data);
            }
        }
    }
    ori_thread_mutex_unlock(list_mutex);
    ori_thread_mutex_unlock(queue_mutex);
}

void safety_queue_push_back_and_list_with_condition(void* queue_mutex, void* list_mutex, ORI_LIST* add_list, ORI_LIST* queue, int timecount, void* queue_condition, void* list_condition, void* thread)
{
    int i, j;
    ADD_LIST* add_list_data;
    COMMAND_LIST* command_data;
    COMMAND_LIST* add_command_data;
    ORI_LIST* push_queue_list;
    
    ori_thread_mutex_lock(list_mutex);
    while(add_list->si_count == 0)
    {
        ori_thread_condition_wait(list_condition, list_mutex);
        if(ori_thread_is_cancelled(thread))
        {
            ori_thread_mutex_unlock(list_mutex);
            return;
        }
    }
    push_queue_list = create_ori_list();
    push_queue_list->release_data = release_command_list_data;
    for(i = 0; i < add_list->si_count; i++)
    {
        add_list_data = (ADD_LIST*)add_list->index_search(add_list, i);
        if(timecount % add_list_data->interval == 0)
        {
            for(j = 0; j < add_list_data->p_list->si_count; j++)
            {
                command_data = (COMMAND_LIST*)add_list_data->p_list->index_search(add_list_data->p_list, j);
                add_command_data = create_command_data(command_data->command);
                push_queue_list->push_back(push_queue_list, (ORI_DATA*)add_command_data);
            }
        }
    }
    ori_thread_mutex_unlock(list_mutex);
    if(push_queue_list->si_count != 0)
    {
        ori_thread_mutex_lock(queue_mutex);
        for(i = 0; i < push_queue_list->si_count; i++)
        {
            command_data = (COMMAND_LIST*)push_queue_list->index_search(push_queue_list, i);
            add_command_data = create_command_data(command_data->command);
            queue->push_back(queue, (ORI_DATA*)add_command_data);
        }
        ori_thread_condition_signal(queue_condition);
        ori_thread_mutex_unlock(queue_mutex);
    }
    push_queue_list->list_clear(push_queue_list);
    free(push_queue_list);
}

int safety_queue_pop_front(void* mutex, ORI_LIST* queue, COMMAND_LIST* p_data)
{
    COMMAND_LIST* pop_data;
    int rc;
    
    ori_thread_mutex_lock(mutex);
    
    if(queue->si_count != 0)
    {
        pop_data = (COMMAND_LIST*)(queue->p_data);
        p_data->command = (char*)malloc(sizeof(char) * (strlen(pop_data->command) + 1));
        strcpy(p_data->command, pop_data->command);
        queue->pop_front(queue);
        rc = 0;
    }
    else
    {
        rc = 1;
    }
    
    ori_thread_mutex_unlock(mutex);
    return rc;
}

COMMAND_LIST* safety_queue_pop_front_with_condition(void* mutex, ORI_LIST* queue, void* condition, void* thread)
{
    COMMAND_LIST* pop_data;
    COMMAND_LIST* p_data;
    
    ori_thread_mutex_lock(mutex);
    while(queue->si_count == 0)
    {
        ori_thread_condition_wait(condition, mutex);
        if(ori_thread_is_cancelled(thread))
        {
            ori_thread_mutex_unlock(mutex);
            return NULL;
        }
    }
    pop_data = (COMMAND_LIST*)queue->index_search(queue, 0);
    p_data = create_command_data(pop_data->command);
    queue->pop_front(queue);
    ori_thread_mutex_unlock(mutex);
    return p_data;
}

void wakeup_condition(void* mutex, void* condition)
{
    ori_thread_mutex_lock(mutex);
    ori_thread_condition_signal(condition);
    ori_thread_mutex_unlock(mutex);
}

void release_add_list_data(ORI_DATA* p_data)
{
    ADD_LIST* add_data;
    add_data = (ADD_LIST*)p_data;
    add_data->p_list->list_clear(add_data->p_list);
    free(add_data->p_list);
    free(add_data);
    return;
}


void release_command_list_data(ORI_DATA* p_data)
{
    COMMAND_LIST* comannd_data;
    comannd_data = (COMMAND_LIST*)p_data;
    free(comannd_data->command);
    free(comannd_data);
    return;
}


void command_queue_init(SSH_SESSION* ssh_session)
{
    if(ssh_session->command_queue == NULL)
    {
        ssh_session->command_queue = create_ori_list();
        ssh_session->command_queue->release_data = release_command_list_data;
    }
    return;
}

void add_list_init(SSH_SESSION* ssh_session)
{
    if(ssh_session->add_list == NULL)
    {
        ssh_session->add_list = create_ori_list();
        ssh_session->add_list->release_data = release_add_list_data;
    }
    return;
}

void command_list_init(ADD_LIST* add_list)
{
    if(add_list->p_list == NULL)
    {
        add_list->p_list = create_ori_list();
        add_list->p_list->release_data = release_command_list_data;
    }
}

COMMAND_LIST* create_command_data(const char* command)
{
    COMMAND_LIST* command_data;
    char* add_command;
    command_data = (COMMAND_LIST*)malloc(sizeof(COMMAND_LIST));
    add_command = (char*)malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(add_command, command);
    command_data->p_next = NULL;
    command_data->command = add_command;
    
    return command_data;
}

ADD_LIST* create_add_data(int interval)
{
    ADD_LIST* add_data;
    add_data = (ADD_LIST*)malloc(sizeof(ADD_LIST));
    add_data->p_next = NULL;
    add_data->interval = interval;
    add_data->p_list = NULL;
    command_list_init(add_data);
    
    return add_data;
}
