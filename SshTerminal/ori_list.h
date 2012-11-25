
#ifndef _ORI_LIST_H
#define _ORI_LIST_H


typedef struct ORI_DATA ORI_DATA;
typedef struct ORI_LIST ORI_LIST;

struct ORI_DATA
{
    ORI_DATA* p_next;
};

struct ORI_LIST
{
    ORI_DATA* p_data;
    int si_count;
    
    void (*push_back)(ORI_LIST*, ORI_DATA*);
    void (*push_front)(ORI_LIST*, ORI_DATA*);
    void (*pop_back)(ORI_LIST*);
    void (*pop_front)(ORI_LIST*);
    void (*list_clear)(ORI_LIST*);
    int (*list_count)(ORI_LIST*);
    ORI_DATA* (*index_search)(ORI_LIST*, int);
    int (*insert_data)(ORI_LIST*, ORI_DATA*, int);
    int (*delete_data)(ORI_LIST*, int);
    
    void (*release_data)(ORI_DATA*);
}; 

extern ORI_LIST* create_ori_list(void);


#endif