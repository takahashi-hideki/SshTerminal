#ifndef _SSH_CONNECTION_H
#define _SSH_CONNECTION_H

#include "libssh2.h"
#include "ori_list.h"
#include "ori_thread.h"
#include "ssh_typedef.h"

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
void* server_login(const char* host_name, const char* user_name, Boolean use_key, const char* password, const char* private_key, const char* public_key, const char* port);

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
int ssh_exec_channel_open(void* ssh_session, Boolean async_mode);

/*******************************************************************************************/
/**
 * @brief   channel open for shell request
 *
 * @param   ssh_session ssh session parameter structure
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int ssh_shell_channel_open(void* ssh_session);

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
int ssh_exec_command(void* ssh_session, const char* command, int (*callback)(char*, const char*, void*), void* obj);

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
int ssh_exec_channel_async_init(void* ssh_session, int (*callback)(char*, const char*, void*), void* obj);

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
int async_send_a_command_exec_channel(void* ssh_session, const char* command);

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
int async_send_loop_command_exec_channel(void* ssh_session, const char* command, int interval);

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
int change_shell_channel_pty_size(void* ssh_session, int pty_width, int pty_height);

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
int ssh_shell_channel_init(void* ssh_session, int (*callback)(char*, const char*, void*), void* obj);

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
int ssh_shell_channel_send(void* ssh_session, const char* write_buffer);

/*******************************************************************************************/
/**
 * @brief   disconnet ssh server
 *
 * @param   ssh_session ssh session parameter structure
 *
 * @return  successful:0, fail:else
 */
/*******************************************************************************************/
int close_ssh_connection(void* ssh_session);

#endif