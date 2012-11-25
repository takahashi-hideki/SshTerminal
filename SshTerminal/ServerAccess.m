#import "ServerAccess.h"

#include "ssh_connection.h"

@implementation ServerAccess

@synthesize hostName;
@synthesize userName;
@synthesize passWord;
@synthesize privateKey;
@synthesize publicKey;
@synthesize port;
@synthesize useKey;
@synthesize serverAccessDelegate;

- (id)init
{
    self = [super init];
    hostName = [[NSString alloc] init];
    userName = [[NSString alloc] init];
    passWord = [[NSString alloc] init];
    privateKey = [[NSString alloc] init];
    publicKey = [[NSString alloc] init];
    useKey = FALSE;
    
    return self;
}

- (void)dealloc
{
    [hostName release];
    [userName release];
    [passWord release];
    [privateKey release];
    [publicKey release];
    [super dealloc];
}

//callback function for print server response
int command_result_print(char* result, const char* command, void* obj)
{
    @autoreleasepool {
        ServerAccess* self_obj = (ServerAccess*)obj;
        NSString* strResult = [NSString stringWithCString:result encoding:NSASCIIStringEncoding];
        [self_obj.serverAccessDelegate commandResultLoad:strResult :command];
        return 0;
    }
}

//connect to ssh server
- (BOOL)connetSshServer
{
    Boolean use_key;
    NSString* keyDir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];
    NSString* privateKeyPath = [keyDir stringByAppendingPathComponent:self.privateKey];
    NSString* publicKeyPath = [keyDir stringByAppendingPathComponent:self.publicKey];
    
    const char* host_name = [self.hostName cStringUsingEncoding:NSUTF8StringEncoding];
    const char* user_name = [self.userName cStringUsingEncoding:NSUTF8StringEncoding];
    const char* pass_word = [self.passWord cStringUsingEncoding:NSUTF8StringEncoding];
    const char* private_key = [privateKeyPath cStringUsingEncoding:NSUTF8StringEncoding];
    const char* public_key = [publicKeyPath cStringUsingEncoding:NSUTF8StringEncoding];
    const char* ch_port = [self.port cStringUsingEncoding:NSUTF8StringEncoding];
    
    if(useKey)
    {
        use_key = TRUE;
    }
    else
    {
        use_key = FALSE;
    }
    
    ssh_session = server_login(host_name, user_name, use_key, pass_word, private_key, public_key, ch_port);
    if(ssh_session == NULL)
    {
        return FALSE;
    }
    ssh_shell_channel_open(ssh_session);
    
    return YES;
}

//setting ssh shell parameter
- (void)terminalInit
{
    void* this_obj = (void*)self;
    ssh_shell_channel_init(ssh_session, command_result_print, this_obj);
    
    return;
}

//send to server user input string
- (void)commandExec:(NSString *)command
{
    const char* ch_command = [command cStringUsingEncoding:NSUTF8StringEncoding];
    ssh_shell_channel_send(ssh_session, ch_command);
    
    return;
}

//disconnect ssh server
- (void)disconnectServer
{
    close_ssh_connection(ssh_session);
    return;
}

@end