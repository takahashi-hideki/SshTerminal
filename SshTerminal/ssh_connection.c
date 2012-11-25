#include "ssh_connection.h"
#include "ssh_debug.h"
#include "data_access_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>

void* exec_channel_command_timer_thread(void* arg);
void* exec_channel_command_exec_thread(void* arg);
void* shell_channel_read_write_thread(void* arg);
static Boolean login_check(SSH_SESSION* p_session);
static int exec_channel_command_exec(SSH_SESSION* p_session, const char* command, int (*callback)(char*, const char*, void*), void* obj);
static int exit_thread(SSH_SESSION* p_session);
static int exec_channel_thread_exit(SSH_SESSION* p_session);
static int shell_channel_thread_exit(SSH_SESSION* p_session);

/*******************************************************************************************/
/**
 * @brief   wait for socket I/O ready status
 *
 * @param   socket_id   socket
 *          ssh_session ssh session parameter structure
 *          timeout_sec time for timeout
 *
 * @return  number of ready status socket, negative number:error code
 */
/*******************************************************************************************/
static int waitsocket(int socket_fd, LIBSSH2_SESSION* ssh_session, time_t timeout_sec)
{
    int ec;
    struct timeval timeout;
    
    fd_set fd;
    fd_set* writefd = NULL;
    fd_set* readfd = NULL;
    int dir;
    
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    FD_ZERO(&fd);
    FD_SET(socket_fd, &fd);
    
    dir = libssh2_session_block_directions(ssh_session);
    
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
    {
        readfd = &fd;
    }
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
    {
        writefd = &fd;
    }
    
    ec = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
    
    return ec;
}

/*******************************************************************************************/
/**
 * @brief   check login status
 * 
 * @param   p_session   ssh session parameter structure
 *
 * @return  logged on:true, else:false
 */
/*******************************************************************************************/
static Boolean login_check(SSH_SESSION* p_session)
{
    if(p_session == NULL)
    {
        return FALSE;
    }
    if(!(p_session->session_param->login_flag))
    {
        return FALSE;
    }
    return TRUE;
}

/*******************************************************************************************/
/**
 * @brief   connect to ssh server, and user authentication
 *
 * @param   host_name   ssh server name
 *          user_name   login user name
 *          use_key     true:authentication by privatekey, false:authentication by password
 *          password    login user password, or privatekey passcode
 *          private_key privatekey path
 *          public_key  publickey path
 *          port        connection port
 *
 * @return  ssh session parameter structure
 */
/*******************************************************************************************/
void* server_login(const char* host_name, const char* user_name, Boolean use_key, const char* password, const char* private_key, const char* public_key, const char* port)
{
    struct addrinfo hints;
    struct addrinfo *res, *ai;
    int ec;
    int sock = -1;
    LIBSSH2_SESSION* session;
    SSH_SESSION* ssh_session;
    SSH_SESSION_PARAM* session_param;
    THREAD_PARAM* thread_param;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    
    
    if(getaddrinfo(host_name, port, &hints, &res) != 0)
    {
        debug_log(SSH_TERMINAL_GETADDRINFO_ERROR, "server_login : login failed.");
        return NULL;
    }
    
    for(ai = res; ai != NULL; ai = ai->ai_next)
    {
        sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if(sock < 0)
        {
            debug_log(SSH_TERMINAL_SOCKET_ERROR, "server_login : login failed.");
            return NULL;
        }
        if(connect(sock, ai->ai_addr, ai->ai_addrlen) < 0)
        {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    
    session = libssh2_session_init();
    if(session == NULL)
    {
        close(sock);
        debug_log(SSH_TERMINAL_SESSION_INIT_ERROR, "server_login : login failed.");
        return NULL;
    }
    
    libssh2_session_set_blocking(session, 0);
    while((ec = libssh2_session_startup(session, sock)) == LIBSSH2_ERROR_EAGAIN)
    {
        ;
    }
    if(ec < 0)
    {
        libssh2_session_disconnect(session, "error");
        libssh2_session_free(session);
        close(sock);
        debug_log(SSH_TERMINAL_SESSION_STARTUP_ERROR, "server_login : login failed.");
        return NULL;
    }

    if(use_key)
    {
        while((ec = libssh2_userauth_publickey_fromfile(session, user_name, public_key, private_key, password)) == LIBSSH2_ERROR_EAGAIN)
        {
            ;
        }
        if(ec < 0)
        {
            libssh2_session_disconnect(session, "error");
            libssh2_session_free(session);
            close(sock);
            debug_log(SSH_TERMINAL_USERAUTH_ERROR, "server_login : publicckey userauth failed.");
            return NULL;
        }
    }
    else
    {
        while((ec = libssh2_userauth_password(session, user_name, password)) == LIBSSH2_ERROR_EAGAIN)
        {
            ;
        }
        if(ec < 0)
        {
            libssh2_session_disconnect(session, "error");
            libssh2_session_free(session);
            close(sock);
            debug_log(SSH_TERMINAL_USERAUTH_ERROR, "server_login : password userauth failed.");
            return NULL;
        }
    }
    
    ssh_session = (SSH_SESSION*)malloc(sizeof(SSH_SESSION));
    
    session_param = (SSH_SESSION_PARAM*)malloc(sizeof(SSH_SESSION_PARAM));
    session_param->sock = sock;
    session_param->session = session;
    session_param->channel = NULL;
    session_param->login_flag = TRUE;
    session_param->async_mode = TRUE;
    session_param->channel_type = -1;
    
    thread_param = (THREAD_PARAM*)malloc(sizeof(THREAD_PARAM));
    thread_param->exec_command_thread = NULL;
    thread_param->command_timer_thread = NULL;
    thread_param->command_queue_mutex = ori_thread_mutex_init();
    thread_param->add_list_mutex = ori_thread_mutex_init();
    thread_param->command_queue_condition = ori_thread_codition_init();
    thread_param->add_list_condition = ori_thread_codition_init();
    thread_param->exec_command_thread_flag = FALSE;
    thread_param->command_timer_thread_flag = FALSE;
    
    ssh_session->session_param = session_param;
    ssh_session->add_list = NULL;
    ssh_session->command_queue = NULL;
    ssh_session->thread_param = thread_param;
    
    return (void*)ssh_session;
}

/*******************************************************************************************/
/**
 * @brief   channel open for exec request
 *
 * @param   ssh_session ssh session parameter structure
 *          async_mode  true:perform background execute command and read result
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_exec_channel_open(void* ssh_session, Boolean async_mode)
{
    SSH_SESSION* p_session;
    LIBSSH2_CHANNEL* channel;
    
    p_session = (SSH_SESSION*)ssh_session;
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_exec_channel_open : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel != NULL)
    {
        debug_log(SSH_TERMINAL_ALREADY_CHANNEL_OPEN, "ssh_exec_channel_open : already channel open.");
        return SSH_TERMINAL_ALREADY_CHANNEL_OPEN;
    }

    while((channel = libssh2_channel_open_session(p_session->session_param->session)) == NULL &&
          libssh2_session_last_error(p_session->session_param->session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    
    if(channel == NULL)
    {
        debug_log(SSH_TERMINAL_CHANNEL_OPEN_ERROR, "ssh_exec_channel_open : channel open failed.");
        return SSH_TERMINAL_CHANNEL_OPEN_ERROR;
    }
    
    p_session->session_param->channel = channel;
    p_session->session_param->channel_type = CHANNEL_TYPE_EXEC;
    p_session->session_param->async_mode = async_mode;

    return 0;
}

/*******************************************************************************************/
/**
 * @brief   channel open for shell request
 *
 * @param   ssh_session ssh session parameter structure
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/

int ssh_shell_channel_open(void* ssh_session)
{
    SSH_SESSION* p_session;
    LIBSSH2_CHANNEL* channel;
    int ec;
    
    p_session = (SSH_SESSION*)ssh_session;
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_shell_channel_open : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel != NULL)
    {
        debug_log(SSH_TERMINAL_ALREADY_CHANNEL_OPEN, "ssh_shell_channel_open : already channel open.");
        return SSH_TERMINAL_ALREADY_CHANNEL_OPEN;
    }
    
    while((channel = libssh2_channel_open_session(p_session->session_param->session)) == NULL &&
          libssh2_session_last_error(p_session->session_param->session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    
    if(channel == NULL)
    {
        debug_log(SSH_TERMINAL_CHANNEL_OPEN_ERROR, "ssh_shell_channel_open : channel open failed.");
        return SSH_TERMINAL_CHANNEL_OPEN_ERROR;
    }
    
    while((ec = libssh2_channel_request_pty(channel, "vt100")) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);;
    }
    if(ec < 0){
        debug_log(SSH_TERMINAL_REQUEST_PTY_ERROR, "ssh_shell_channel_open : request pty failed.");
        while(libssh2_channel_close(channel) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
        }
        libssh2_channel_free(channel);
        return SSH_TERMINAL_REQUEST_PTY_ERROR;
    }
    
    while((ec = libssh2_channel_request_pty_size(channel, DEFAULT_PTY_SIZE_WIDTH, DEFAULT_PTY_SIZE_HEIGHT)) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(ec < 0){
        debug_log(SSH_TERMINAL_REQUEST_PTY_ERROR, "ssh_shell_channel_open : request pty failed.");
        while(libssh2_channel_close(channel) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
        }
        libssh2_channel_free(channel);
        return SSH_TERMINAL_REQUEST_PTY_ERROR;
    }
    
    while((ec = libssh2_channel_shell(channel)) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(ec < 0)
    {
        debug_log(SSH_TERMINAL_SHELL_ERROR, "ssh_shell_channel_open : channel shell failed.");
        while(libssh2_channel_close(channel) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
        }
        libssh2_channel_free(channel);
        return SSH_TERMINAL_SHELL_ERROR;
    }
    
    p_session->session_param->channel = channel;
    p_session->session_param->channel_type = CHANNEL_TYPE_SHELL;
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   execute command synchronous in exec request
 *
 * @param   ssh_session ssh session parameter structure
 *          command     command string
 *          callback    function for result string
 *          obj         object for call callback function
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_exec_command(void* ssh_session, const char* command, int (*callback)(char*, const char*, void*), void* obj)
{
    int ec;
    SSH_SESSION* p_session;
    p_session = (SSH_SESSION*)ssh_session;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_exec_command : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_EXEC)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "ssh_exec_command : channel type is not exec.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(p_session->session_param->async_mode)
    {
        debug_log(SSH_TERMINAL_ASYNC_MODE_ERROR, "ssh_exec_command : async mode must be false.");
        return SSH_TERMINAL_ASYNC_MODE_ERROR;
    }
    
    if(p_session->session_param->channel == NULL)
    {
        debug_log(SSH_TERMINAL_NOT_OPEN_CHANNEL, "ssh_exec_command : don't open channel.");
        return SSH_TERMINAL_NOT_OPEN_CHANNEL;
    }
    
    ec = exec_channel_command_exec(p_session, command, callback, obj);

    return ec;
}

/*******************************************************************************************/
/**
 * @brief   execute command in exec request and read result string
 *
 * @param   p_session   ssh session parameter structure
 *          command     command string
 *          callback    function for result string
 *          obj         object call callback function
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
static int exec_channel_command_exec(SSH_SESSION* p_session, const char* command, int (*callback)(char*, const char*, void*), void* obj)
{
    char* buffer;
    int buffer_length = COMMAND_RESULT_BUFFER_LENGTH;
    int ec;
    int result_code;
    LIBSSH2_CHANNEL* channel;
    
    while((ec = libssh2_channel_exec(p_session->session_param->channel, command)) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(ec < 0)
    {
        debug_log(SSH_TERMINAL_COMMAND_EXEC_ERROR, "sync_command_exec : channel exec failed.");
        return SSH_TERMINAL_COMMAND_EXEC_ERROR;
    }
    
    buffer = (char*)malloc(sizeof(char) * (buffer_length + 1));
    
    while(1)
    {
        do{
            memset(buffer, 0, buffer_length + 1);
            ec = libssh2_channel_read(p_session->session_param->channel, buffer, buffer_length);
            if(ec == 0)
            {
                ec = libssh2_channel_read_stderr(p_session->session_param->channel, buffer, buffer_length);
            }
            if(ec > 0)
            {
                callback(buffer, command, obj);
            }
        }
        while(ec > 0);
        
        if(ec == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
        }
        else
        {
            break;
        }
    }
    free(buffer);
    if(ec < 0)
    {
        debug_log(SSH_TERMINAL_CHANNEL_READ_ERROR, "sync_command_exec : channel read failed.");
        result_code = SSH_TERMINAL_CHANNEL_READ_ERROR;
    }
    else
    {
        result_code = 0;
    }
    
    while(libssh2_channel_close(p_session->session_param->channel) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    
    libssh2_channel_free(p_session->session_param->channel);
    
    while((channel = libssh2_channel_open_session(p_session->session_param->session)) == NULL &&
          libssh2_session_last_error(p_session->session_param->session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(channel == NULL)
    {
        result_code = SSH_TERMINAL_CHANNEL_OPEN_ERROR;
    }
    
    p_session->session_param->channel = channel;
    
    return result_code;
}

/*******************************************************************************************/
/**
 * @brief   initialize async parameter for exec request
 *
 * @param   ssh_session ssh session parameter structure
 *          callback    function for command result string
 *          obj         object for call callback function
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_exec_channel_async_init(void* ssh_session, int (*callback)(char*, const char*, void*), void* obj)
{
    SSH_SESSION* p_session;
    p_session = (SSH_SESSION*)ssh_session;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_exec_channel_async_init : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_EXEC)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "ssh_exec_channel_async_init : channel type is not exec.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(!(p_session->session_param->async_mode))
    {
        debug_log(SSH_TERMINAL_ASYNC_MODE_ERROR, "ssh_exec_channel_async_init : async mode must be true.");
        return SSH_TERMINAL_ASYNC_MODE_ERROR;
    }

    if(p_session->thread_param->command_timer_thread_flag)
    {
        debug_log(SSH_TERMINAL_ALREADY_RUN_THREAD, "ssh_exec_channel_async_init : already run thread.");
        return SSH_TERMINAL_ALREADY_RUN_THREAD;
    }
    
    command_queue_init(p_session);
    add_list_init(p_session);
    p_session->thread_param->command_timer_thread_flag = TRUE;
    p_session->thread_param->command_timer_thread = ori_thread_create(exec_channel_command_timer_thread, (void*)p_session);
    p_session->callback = callback;
    p_session->obj = obj;

    return 0;
}

/*******************************************************************************************/
/**
 * @brief   function thread run for loop command
 *
 * @param   arg     ssh session parameter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void* exec_channel_command_timer_thread(void* arg)
{
    SSH_SESSION* p_session;
    int timecount = 0;
    p_session = (SSH_SESSION*)arg;
    
    p_session->thread_param->exec_command_thread_flag = TRUE;
    p_session->thread_param->command_timer_thread = ori_thread_create(exec_channel_command_exec_thread, (void*)p_session);
    
    while(!(ori_thread_is_cancelled(p_session->thread_param->command_timer_thread)))
    {
        safety_queue_push_back_and_list_with_condition(p_session->thread_param->command_queue_mutex, p_session->thread_param->add_list_mutex,
                                                       p_session->add_list, p_session->command_queue, timecount,
                                                       p_session->thread_param->command_queue_condition, p_session->thread_param->add_list_condition,
                                                       p_session->thread_param->command_timer_thread);
        sleep(1);
        timecount++;
    }
    
    ori_thread_cancel(p_session->thread_param->exec_command_thread);
    ori_thread_join(p_session->thread_param->exec_command_thread);
    ori_thread_free(p_session->thread_param->exec_command_thread);
    p_session->thread_param->exec_command_thread = NULL;
    
    p_session->thread_param->command_timer_thread_flag = FALSE;
    
    return NULL;
}

/*******************************************************************************************/
/**
 * @brief   function thread run for command execute
 *
 * @param   arg     ssh session patemeter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void* exec_channel_command_exec_thread(void *arg)
{
    SSH_SESSION* p_session;
    COMMAND_LIST* p_data;
    p_session = (SSH_SESSION*)arg;
    
    while(!(ori_thread_is_cancelled(p_session->thread_param->exec_command_thread)))
    {
        p_data = safety_queue_pop_front_with_condition(p_session->thread_param->command_queue_mutex, p_session->command_queue, p_session->thread_param->command_queue_condition, p_session->thread_param->exec_command_thread);
        if(p_data == NULL)
        {
            break;
        }
        exec_channel_command_exec(p_session, p_data->command, p_session->callback, p_session->obj);
        release_command_list_data((ORI_DATA*)p_data);
    }
    
    p_session->thread_param->exec_command_thread_flag = FALSE;
    return NULL;
}

/*******************************************************************************************/
/**
 * @brief   send a command for background perform in exec request
 *
 * @param   ssh_session ssh session parameter structure
 *          command     command string
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int async_send_a_command_exec_channel(void* ssh_session, const char* command)
{
    SSH_SESSION* p_session;
    COMMAND_LIST* send_command_data;
    p_session = (SSH_SESSION*)ssh_session;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "async_send_a_command_exec_channel : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_EXEC)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "async_send_a_command_exec_channel : channel type is not exec.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(!(p_session->session_param->async_mode))
    {
        debug_log(SSH_TERMINAL_ASYNC_MODE_ERROR, "async_send_a_command_exec_channel : async mode must be true.");
        return SSH_TERMINAL_ASYNC_MODE_ERROR;
    }

    if(!(p_session->thread_param->command_timer_thread_flag))
    {
        debug_log(SSH_TERMINAL_NOT_INIT_THREAD, "async_send_a_command_exec_channel : not init exec channel thread.");
        return SSH_TERMINAL_NOT_INIT_THREAD;
    }
    
    send_command_data = create_command_data(command);
    safety_queue_push_back_with_condition(p_session->thread_param->command_queue_mutex, (ORI_DATA*)send_command_data, p_session->command_queue, p_session->thread_param->command_queue_condition);

    return 0;
}

/*******************************************************************************************/
/**
 * @brief   send command each interval times for background perform in exec request
 *
 * @param   ssh_session ssh session parameter structure
 *          command     command string
 *          interval    interval time execute command
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int async_send_loop_command_exec_channel(void* ssh_session, const char* command, int interval)
{
    SSH_SESSION* p_session;
    COMMAND_LIST* add_command;
    p_session = (SSH_SESSION*)ssh_session;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "async_send_loop_command_exec_channel : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_EXEC)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "async_send_loop_command_exec_channel : channel type is not exec.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(!(p_session->session_param->async_mode))
    {
        debug_log(SSH_TERMINAL_ASYNC_MODE_ERROR, "async_send_loop_command_exec_channel : async mode must be true.");
        return SSH_TERMINAL_ASYNC_MODE_ERROR;
    }
    
    if(!(p_session->thread_param->command_timer_thread_flag))
    {
        debug_log(SSH_TERMINAL_NOT_INIT_THREAD, "async_send_loop_command_exec_channel : not init exec channel thread.");
        return SSH_TERMINAL_NOT_INIT_THREAD;
    }
    
    add_command = create_command_data(command);
    safety_add_list_insert_with_condition(p_session->thread_param->add_list_mutex, (ORI_DATA*)add_command, p_session->add_list,
                                          interval, p_session->thread_param->add_list_condition);
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   change pty size for shell request
 *
 * @param   ssh_session ssh session parameter structure
 *          pty_width   pty column size
 *          pty_height  pty row size
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int change_shell_channel_pty_size(void* ssh_session, int pty_width, int pty_heigth)
{
    SSH_SESSION* p_session;
    int ec;
    
    p_session = (SSH_SESSION*)ssh_session;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "change_shell_channel_pty_size : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_SHELL)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "change_shell_channel_pty_size : channel type is not shell.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    while((ec = libssh2_channel_request_pty_size(p_session->session_param->channel, pty_width, pty_heigth)) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(ec < 0)
    {
        debug_log(SSH_TERMINAL_CHANGE_PTY_SIZE_ERROR, "change_shell_channel_pty_size : channel request pty size failed.");
        return SSH_TERMINAL_CHANGE_PTY_SIZE_ERROR;
    }
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   initialize parameter for shell request
 *
 * @param   ssh_session ssh session parameter structure
 *          callback    function for server response string
 *          obj         object for call callback funtion
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_shell_channel_init(void* ssh_session, int (*callback)(char*, const char*, void*), void* obj)
{
    SSH_SESSION* p_session = (SSH_SESSION*)ssh_session;
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_channel_init : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_SHELL)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "ssh_shell_channel_init : channel type is not shell.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(p_session->thread_param->exec_command_thread_flag)
    {
        debug_log(SSH_TERMINAL_ALREADY_RUN_THREAD, "ssh_channel_init : already run thread.");
        return SSH_TERMINAL_ALREADY_RUN_THREAD;
    }
    
    command_queue_init(p_session);
    p_session->thread_param->exec_command_thread_flag = TRUE;
    p_session->thread_param->exec_command_thread = ori_thread_create(shell_channel_read_write_thread, (void*)p_session);
    p_session->callback = callback;
    p_session->obj = obj;
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   functtion thread run for shell read and write
 *
 * @param   arg     ssh session parameter structure
 *
 * @return  nothing
 */
/*******************************************************************************************/
void* shell_channel_read_write_thread(void* arg)
{
    SSH_SESSION* p_session = (SSH_SESSION*)arg;
    COMMAND_LIST* p_data;
    int rc;
    char* buffer;
    int buffer_length = COMMAND_RESULT_BUFFER_LENGTH;
    buffer = (char*)malloc(sizeof(char) * buffer_length + 1);
    //usleep(50000);
    while(!(ori_thread_is_cancelled(p_session->thread_param->exec_command_thread)))
    {
        //read
        memset(buffer, 0, buffer_length + 1);
        rc = libssh2_channel_read(p_session->session_param->channel, buffer, buffer_length);
        if(rc > 0){
            p_session->callback(buffer, NULL, p_session->obj);
        }
        else
        {
            usleep(100000);
        }
        
        //write
        p_data = (COMMAND_LIST*)malloc(sizeof(COMMAND_LIST));
        rc = safety_queue_pop_front(p_session->thread_param->command_queue_mutex, p_session->command_queue, p_data);
        if(rc == 0)
        {
            libssh2_channel_write(p_session->session_param->channel, p_data->command, strlen(p_data->command));
            free(p_data->command);
        }
        free(p_data);
    }
    
    free(buffer);
    
    if(p_session->thread_param->command_timer_thread_flag)
    {
        ori_thread_cancel(p_session->thread_param->command_timer_thread);
        ori_thread_join(p_session->thread_param->command_timer_thread);
        ori_thread_free(p_session->thread_param->command_timer_thread);
        p_session->thread_param->command_timer_thread = NULL;
    }
    p_session->thread_param->exec_command_thread_flag = FALSE;
    return NULL;
}

/*******************************************************************************************/
/**
 * @brief   send write string on shell request
 *
 * @param   ssh_session ssh session parameter structure
 *          write_buffer    string send to server
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_shell_channel_send(void* ssh_session, const char* write_buffer)
{
    SSH_SESSION* p_session;
    p_session = (SSH_SESSION*)ssh_session;
    COMMAND_LIST* add_command_data;
    
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "ssh_shell_channel_send : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel_type != CHANNEL_TYPE_SHELL)
    {
        debug_log(SSH_TERMINAL_INVALID_CHANNEL_TYPE, "ssh_shell_channel_send : channel type is not shell.");
        return SSH_TERMINAL_INVALID_CHANNEL_TYPE;
    }
    
    if(!(p_session->thread_param->exec_command_thread_flag))
    {
        debug_log(SSH_TERMINAL_NOT_INIT_THREAD, "ssh_shell_channel_send : not init channel r/w thread.");
        return SSH_TERMINAL_NOT_INIT_THREAD;
    }
    
    add_command_data = create_command_data(write_buffer);
    
    safety_queue_push_back(p_session->thread_param->command_queue_mutex, (ORI_DATA*)add_command_data, p_session->command_queue);
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   disconnet ssh server
 *
 * @param   ssh_session ssh session parameter structure
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int close_ssh_connection(void* ssh_session)
{
    int ec;
    SSH_SESSION* p_session;
    
    p_session = (SSH_SESSION*)ssh_session;
    if(!(login_check(p_session)))
    {
        debug_log(SSH_TERMINAL_NO_SESSION_ERROR, "close_ssh_connection : no session.");
        return SSH_TERMINAL_NO_SESSION_ERROR;
    }
    
    if(p_session->session_param->channel != NULL)
    {
        exit_thread(ssh_session);
    
        while((ec = libssh2_channel_close(p_session->session_param->channel)) == LIBSSH2_ERROR_EAGAIN)
        {
            waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
        }
        libssh2_channel_free(p_session->session_param->channel);
        p_session->session_param->channel = NULL;
    }
    
    while((ec = libssh2_session_disconnect(p_session->session_param->session, "normal")) == LIBSSH2_ERROR_EAGAIN)
    {
        waitsocket(p_session->session_param->sock, p_session->session_param->session, TIMEOUT_SEC);
    }
    if(ec < 0)
    {
        debug_log(SSH_TERMINAL_SESSION_DISCONNECT_ERROR, "close_ssh_connection : disconnect failed.");
        return SSH_TERMINAL_SESSION_DISCONNECT_ERROR;
    }
    libssh2_session_free(p_session->session_param->session);
    close(p_session->session_param->sock);
    free(p_session->session_param);
    ori_thread_mutex_free(p_session->thread_param->command_queue_mutex);
    ori_thread_mutex_free(p_session->thread_param->add_list_mutex);
    ori_thread_condition_free(p_session->thread_param->command_queue_condition);
    ori_thread_condition_free(p_session->thread_param->add_list_condition);
    free(p_session->thread_param);
    free(p_session);
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   exit thread
 *
 * @param   p_session   ssh session parameter structure
 *
 * @return  successful:0, else:fail
 */
/*******************************************************************************************/
static int exit_thread(SSH_SESSION* p_session)
{
    if(p_session->session_param->channel_type == CHANNEL_TYPE_SHELL)
    {
        shell_channel_thread_exit(p_session);
    }
    else
    if(p_session->session_param->channel_type == CHANNEL_TYPE_EXEC)
    {
        if(p_session->session_param->async_mode)
        {
            exec_channel_thread_exit(p_session);
        }
    }
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   exit thread for exec request channel
 *
 * @param   p_session   ssh session parameter structure
 *
 * @return  successful:0, else:fail
 */
/*******************************************************************************************/
static int exec_channel_thread_exit(SSH_SESSION* p_session)
{
    ori_thread_cancel(p_session->thread_param->command_timer_thread);
    ori_thread_join(p_session->thread_param->command_timer_thread);
    ori_thread_free(p_session->thread_param->command_timer_thread);
    p_session->thread_param->command_timer_thread = NULL;
    
    if(p_session->command_queue != NULL)
    {
        p_session->command_queue->list_clear(p_session->command_queue);
        free(p_session->command_queue);
        p_session->command_queue = NULL;
    }
    
    if(p_session->add_list != NULL)
    {
        p_session->add_list->list_clear(p_session->add_list);
        free(p_session->add_list);
        p_session->add_list = NULL;
    }
    
    return 0;
}

/*******************************************************************************************/
/**
 * @brief   exit thread for shell request channel
 *
 * @param   p_session   ssh session parameter structure
 *
 * @return successful:0, else:fail
 */
/*******************************************************************************************/
static int shell_channel_thread_exit(SSH_SESSION* p_session)
{
    
    ori_thread_cancel(p_session->thread_param->exec_command_thread);
    ori_thread_join(p_session->thread_param->exec_command_thread);
    ori_thread_free(p_session->thread_param->exec_command_thread);
    p_session->thread_param->exec_command_thread = NULL;
    
    if(p_session->command_queue != NULL)
    {
        p_session->command_queue->list_clear(p_session->command_queue);
        free(p_session->command_queue);
        p_session->command_queue = NULL;
    }
    
    return 0;
}
