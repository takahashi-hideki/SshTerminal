#include "ORI_LIST.h"

#include <stdlib.h>

static ORI_DATA* search_end(ORI_LIST* p_list)
{
    ORI_DATA* p_data = p_list->p_data;
    if(p_data == NULL)
    {
        return NULL;
    }
    while(p_data->p_next != NULL)
    {
        p_data = p_data->p_next;
    }
    
    return p_data;
}

void push_back(ORI_LIST* p_list, ORI_DATA* p_data)
{
    ORI_DATA* p_data_end = search_end(p_list);
    if(p_data_end == NULL)
    {
        p_list->p_data = p_data;
    }
    else
    {
        p_data_end->p_next = p_data;
    }
    (p_list->si_count)++;
    
    return;
}

void push_front(ORI_LIST* p_list, ORI_DATA* p_data)
{
    p_data->p_next = p_list->p_data;
    p_list->p_data = p_data;
    (p_list->si_count)++;
    
    return;
}

void pop_back(ORI_LIST* p_list)
{
    ORI_DATA* p_data = p_list->p_data;
    ORI_DATA* p_data_pre_end;
    
    
    if(p_data == NULL)
    {
        return;
    }
    
    if(p_list->si_count == 1)
    {
        p_list->release_data(p_data);
        p_list->p_data = NULL;
        p_list->si_count = 0;
        return;
    }
    
    p_data_pre_end = p_list->index_search(p_list, (p_list->si_count - 2));
    p_data = p_data_pre_end->p_next;
    p_data_pre_end->p_next = NULL;
    p_list->release_data(p_data);
    (p_list->si_count)--;
    return;
}

void pop_front(ORI_LIST* p_list)
{
    ORI_DATA* p_data = p_list->p_data;
    
    if(p_data == NULL)
    {
        return;
    }
    
    p_list->p_data = p_data->p_next;
    p_list->release_data(p_data);
    (p_list->si_count)--;
    
    return;
}

void list_clear(ORI_LIST* p_list)
{
    while(p_list->si_count > 0)
    {
        p_list->pop_front(p_list);
    }
    return;
}

int list_count(ORI_LIST* p_list)
{
    return p_list->si_count;
}

ORI_DATA* index_search(ORI_LIST* p_list, int si_position)
{
    ORI_DATA* p_data = p_list->p_data;
    int si_position_count;
    if(p_data == NULL)
    {
        return NULL;
    }
    
    if(si_position >= p_list->si_count)
    {
        return NULL;
    }
    else
    if(si_position >= 0)
    {
        for(si_position_count = 0; si_position_count < si_position; si_position_count++)
        {
            p_data = p_data->p_next;
            if(p_data == NULL)
            {
                break;
            }
        }
        return p_data;
    }
    else
    if(si_position < 0)
    {
        p_data = search_end(p_list);
        return p_data;
    }
    else
    {
        return NULL;
    }
}

int insert_data(ORI_LIST* p_list, ORI_DATA* p_data, int si_position)
{
    ORI_DATA* p_position_data;
    ORI_DATA* p_pre_position_data;
    if(si_position < 0)
    {
        return -1;
    }
    else
    if(si_position > p_list->si_count)
    {
        return -1;
    }
    else
    if(si_position == p_list->si_count)
    {
        p_list->push_back(p_list, p_data);
        return 0;
    }
    else
    if(si_position == 0)
    {
        p_list->push_front(p_list, p_data);
        return 0;
    }
    else
    {
        p_pre_position_data = p_list->index_search(p_list, (si_position - 1));
        p_position_data = p_list->index_search(p_list, si_position);
        p_pre_position_data->p_next = p_data;
        p_data->p_next = p_position_data;
        (p_list->si_count)++;
        return 0;
    }
}

int delete_data(ORI_LIST* p_list, int si_position)
{
    ORI_DATA* p_position_data;
    ORI_DATA* p_pre_position_data;
    
    if(si_position < 0)
    {
        return -1;
    }
    else
    if(si_position >= p_list->si_count)
    {
        return -1;
    }
    else
    if(si_position == 0)
    {
        p_list->pop_front(p_list);
        return 0;
    }
    else
    if(si_position == (p_list->si_count - 1))
    {
        p_list->pop_back(p_list);
        return 0;
    }
    else
    {
        p_position_data = p_list->index_search(p_list, si_position);
        p_pre_position_data = p_list->index_search(p_list, (si_position - 1));
        p_pre_position_data->p_next = p_position_data->p_next;
        p_list->release_data(p_position_data);
        (p_list->si_count)--;
        return 0;
    }
}

ORI_LIST* create_ori_list(void)
{
    ORI_LIST* p_list;
    
    p_list = (ORI_LIST*)malloc(sizeof(ORI_LIST));
    p_list->p_data = NULL;
    p_list-> si_count = 0;
    p_list->push_back = push_back;
    p_list->push_front = push_front;
    p_list->pop_back = pop_back;
    p_list->pop_front = pop_front;
    p_list->list_clear = list_clear;
    p_list->list_count = list_count;
    p_list->index_search = index_search;
    p_list->insert_data = insert_data;
    p_list->delete_data = delete_data;
    
    return p_list;
}
