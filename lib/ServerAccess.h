#import "TerminalLog.h"

@protocol ServerAccessDelegate;

@interface ServerAccess : NSObject
{
    NSString* hostName;
    NSString* userName;
    NSString* passWord;
    NSString* privateKey;
    NSString* publicKey;
    NSString* port;
    BOOL useKey;
    
    void* ssh_session;
    NSString* currentDir;
    
    id<ServerAccessDelegate> serverAccessDelegate;
}

@property (copy)NSString* hostName;
@property (copy)NSString* userName;
@property (copy)NSString* passWord;
@property (copy)NSString* privateKey;
@property (copy)NSString* publicKey;
@property (copy)NSString* port;
@property (assign)BOOL useKey;
@property (nonatomic, assign)id<ServerAccessDelegate> serverAccessDelegate;

- (BOOL)connetSshServer;
- (void)terminalInit;
- (void)commandExec:(NSString*)command;
- (void)disconnectServer;

@end

@protocol ServerAccessDelegate

- (void)commandResultLoad:(NSString*)result:(const char*)command;

@end