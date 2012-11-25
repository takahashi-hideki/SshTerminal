#include "ori_thread.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>

#else
#include <pthread.h>
#endif

typedef struct ORI_THREAD_PARAM ORI_THREAD_PARAM;

struct ORI_THREAD_PARAM
{
    Boolean cancel_flag;
    
#ifdef _WIN32
    
#else
    pthread_t* t_thread;
#endif
};

/*******************************************************************************************/
/**
 * @brief   thread create, and start
 *
 * @param   thread_function function run thread
 *          arg             function argument
 *
 * @return  thread parameter structure
 */
/*******************************************************************************************/
void* ori_thread_create(void* (*thread_function)(void* arg), void *arg)
{
    void* p_thread = NULL;
#ifdef _WIN32
    
#else
    pthread_t* t_thread = (pthread_t*)malloc(sizeof(pthread_t));
    ORI_THREAD_PARAM* thread_param = (ORI_THREAD_PARAM*)malloc(sizeof(ORI_THREAD_PARAM));
    thread_param->cancel_flag = FALSE;
    thread_param->t_thread = t_thread;
    pthread_create(t_thread, NULL, thread_function, arg);
    p_thread = (void*)thread_param;
#endif
    return p_thread;
}

/*******************************************************************************************/
/**
 * @brief   initialize mutex object
 *
 * @param   nothing
 *
 * @return  mutex object
 */
/*******************************************************************************************/
void* ori_thread_mutex_init(void)
{
    void* mutex;
#ifdef _WIN32
    
#else
    pthread_mutex_t* t_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(t_mutex, NULL);
    mutex = (void*)t_mutex;
#endif
    return mutex;
}

/*******************************************************************************************/
/**
 * @brief   lock mutex object
 *
 * @param   mutex   mutex object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_mutex_lock(void* mutex)
{
#ifdef _WIN32
    
#else
    pthread_mutex_t *t_mutex = (pthread_mutex_t*)mutex;
    pthread_mutex_lock(t_mutex);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   unlock mutex object
 *
 * @param   mutex   mutex object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_mutex_unlock(void* mutex)
{
#ifdef _WIN32
    
#else
    pthread_mutex_t *t_mutex = (pthread_mutex_t*)mutex;
    pthread_mutex_unlock(t_mutex);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   initialize condition object
 *
 * @param   nothing
 *
 * @return  condition object
 */
/*******************************************************************************************/
void* ori_thread_codition_init(void)
{
    void* condition;
#ifdef _WIN32
    
#else
    pthread_cond_t *t_condition = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(t_condition, NULL);
    condition = (void*)t_condition;
#endif
    return condition;
}

/*******************************************************************************************/
/**
 * @brief   suspend until receive wakeup signal
 *
 * @param   condition   condition object
 *          mutex       mutex object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_condition_wait(void* condition, void* mutex)
{
#ifdef _WIN32
    
#else
    pthread_cond_t *t_condition = (pthread_cond_t*)condition;
    pthread_mutex_t *t_mutex = (pthread_mutex_t*)mutex;
    pthread_cond_wait(t_condition, t_mutex);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   send wakeup signal
 *
 * @param   condition   condition object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_condition_signal(void* condition)
{
#ifdef _WIN32
    
#else
    pthread_cond_t *t_condition = (pthread_cond_t*)condition;
    pthread_cond_signal(t_condition);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   cancel thread
 *
 * @param   thread      thread parameter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_cancel(void* thread)
{
    ORI_THREAD_PARAM* thread_param = (ORI_THREAD_PARAM*)thread;
    thread_param->cancel_flag = TRUE;
    return;
}

/*******************************************************************************************/
/**
 * @brief   get cancel status
 *
 * @param   thread      thread parameter structure
 *
 * @return  status cancel: true
 */
/*******************************************************************************************/
Boolean ori_thread_is_cancelled(void* thread)
{
    ORI_THREAD_PARAM* thread_param = (ORI_THREAD_PARAM*)thread;
    if(thread_param->cancel_flag == TRUE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*******************************************************************************************/
/**
 * @brief   wait until finish thread
 *
 * @param   thread      thread parameter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_join(void* thread)
{
    ORI_THREAD_PARAM* thread_param = (ORI_THREAD_PARAM*)thread;
#ifdef _WIN32
    
#else
    pthread_join(*(thread_param->t_thread), NULL);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   release thread parameter structure
 *
 * @param   thread      thread parameter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_free(void* thread)
{
    ORI_THREAD_PARAM* thread_param = (ORI_THREAD_PARAM*)thread;
#ifdef _WIN32
#else
    free(thread_param->t_thread);
    free(thread_param);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   release mutex object
 *
 * @param   mutex   mutex object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_mutex_free(void* mutex)
{
#ifdef _WIN32
    
#else
    pthread_mutex_t* t_mutex = (pthread_mutex_t*)mutex;
    free(t_mutex);
#endif
    return;
}

/*******************************************************************************************/
/**
 * @brief   release condition object
 *
 * @param   condition   condition object
 *
 * @return  nothing
 */
/*******************************************************************************************/
void ori_thread_condition_free(void* condition)
{
#ifdef _WIN32
    
#else
    pthread_cond_t* t_condition = (pthread_cond_t*)condition;
    free(t_condition);
#endif
    return;
}