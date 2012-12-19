#import <UIKit/UIKit.h>
#import "ServerAccess.h"
#import "VirtualTextView.h"
#import "TerminalLog.h"
#import "TerminalAppView.h"

@interface TerminalViewController : UIViewController <UITextFieldDelegate, UITextViewDelegate, ServerAccessDelegate, TerminalLogDelegate>
{
    ServerAccess* _serverAccess;
    TerminalLog* _terminalLog;
    int lineCount;
    
    UIScrollView* mainScrollView;
    VirtualTextView* textLogView;
    UILabel* buttonLabel;
    UILabel* inputLabel;
    UITextField* inputField;
    BOOL keyboardObserving;
    TerminalAppView* terminalAppView;
    
    
    UIButton* keyboardButton;
    UIButton* upKeyButton;
    UIButton* downKeyButton;
    UIButton* escapeKeyButton;
    UIButton* ctrlKeyButton;
    UIButton* tabKeyButton;
    
    UIButton* logClearButton;
    
    BOOL ctrlLock;
    
    BOOL appMode;
    BOOL commandMode;
}

- (id)initWithServerAccess:(ServerAccess*)serverAccess;

@end