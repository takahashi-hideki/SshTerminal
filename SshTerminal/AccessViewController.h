#import <UIKit/UIKit.h>
#import "ServerAccess.h"
#import "TerminalViewController.h"

@interface AccessViewController : UIViewController <UITableViewDelegate, UITableViewDataSource, UITextFieldDelegate>
{
    ServerAccess* _serverAccess;
    UITextField* activeField;
    TerminalViewController* _terminalViewController;
}

@end