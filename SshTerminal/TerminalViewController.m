#import "TerminalViewController.h"
#import "NSStringLineAddition.h"

#define TERMINAL_FONT_SIZE_LARGE 14.0
#define TERMINAL_FONT_SIZE_NORMAL 12.0
#define TERMINAL_FONT_SIZE_SMALL 11.0

#define TERMINAL_SCROLL_BUFFER_LINE 5000
#define TERMINAL_LOG_BUFFER 500000

@implementation TerminalViewController

- (id)initWithServerAccess:(ServerAccess*)serverAccess
{
    self = [super init];
    _serverAccess = serverAccess;
    _serverAccess.serverAccessDelegate = self;
    lineCount = 1;
    mainScrollView = [[UIScrollView alloc] init];
    textLogView = [[VirtualTextView alloc] init];
    buttonLabel = [[UILabel alloc] init];
    inputLabel = [[UILabel alloc] init];
    inputField = [[UITextField alloc] init];
    keyboardButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    upKeyButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    downKeyButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    escapeKeyButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    ctrlKeyButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    tabKeyButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    logClearButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    keyboardObserving = NO;
    ctrlLock = NO;
    appMode = NO;
    commandMode = YES;

    self.title = [NSString stringWithFormat:@"%@@%@", serverAccess.userName, serverAccess.hostName];
    
    return self;
}

- (void)dealloc
{
    [_serverAccess release];
    [mainScrollView release];
    [textLogView release];
    [buttonLabel release];
    [inputLabel release];
    [inputField release];
    
    [keyboardButton release];
    [upKeyButton release];
    [downKeyButton release];
    [escapeKeyButton release];
    [ctrlKeyButton release];
    [tabKeyButton release];
    [logClearButton release];
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    mainScrollView.frame = self.view.bounds;
    mainScrollView.backgroundColor = [UIColor blackColor];
    mainScrollView.scrollEnabled = NO;
    
    textLogView.frame = CGRectMake(0, 10, self.view.bounds.size.width, self.view.bounds.size.height - 60);
    [textLogView setFont:[UIFont fontWithName:@"Courier" size:TERMINAL_FONT_SIZE_NORMAL]];
    textLogView.backgroundColor = [UIColor blackColor];
    [textLogView setTextColor:[UIColor whiteColor]];
    textLogView.tag = 300;
    textLogView.delegate = self;
    [mainScrollView addSubview:textLogView];
    
    inputField.backgroundColor = [UIColor clearColor];
    inputField.keyboardType = UIKeyboardTypeASCIICapable;
    inputField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    inputField.autocorrectionType = UITextAutocorrectionTypeNo;
    inputField.returnKeyType = UIReturnKeyDone;
    inputField.delegate = self;
    inputField.hidden = YES;
    inputField.text = @" ";
    [mainScrollView addSubview:inputField];
    [inputField becomeFirstResponder];

    buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 95, self.view.bounds.size.width, 60);
    buttonLabel.backgroundColor = [UIColor grayColor];
    buttonLabel.userInteractionEnabled = YES;
    [mainScrollView addSubview:buttonLabel];
    
    
    keyboardButton.frame = CGRectMake(20, 5, 80, 50);
    [keyboardButton setTitle:@"keyboard" forState:UIControlStateNormal];
    [buttonLabel addSubview:keyboardButton];
    
    upKeyButton.frame = CGRectMake(120, 5, 50, 50);
    [upKeyButton setTitle:@"↑" forState:UIControlStateNormal];
    [upKeyButton addTarget:self action:@selector(inputUpKey:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:upKeyButton];
    
    downKeyButton.frame = CGRectMake(190, 5, 50, 50);
    [downKeyButton setTitle:@"↓" forState:UIControlStateNormal];
    [downKeyButton addTarget:self action:@selector(inputDownKey:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:downKeyButton];
    
    escapeKeyButton.frame = CGRectMake(260, 5, 50, 50);
    [escapeKeyButton setTitle:@"ESC" forState:UIControlStateNormal];
    [escapeKeyButton addTarget:self action:@selector(inputEscKey:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:escapeKeyButton];
    
    ctrlKeyButton.frame = CGRectMake(330, 5, 50, 50);
    [ctrlKeyButton setTitle:@"ctrl" forState:UIControlStateNormal];
    [ctrlKeyButton addTarget:self action:@selector(onCtrlKey:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:ctrlKeyButton];
    
    tabKeyButton.frame = CGRectMake(400, 5, 50, 50);
    [tabKeyButton setTitle:@"tab" forState:UIControlStateNormal];
    [tabKeyButton addTarget:self action:@selector(inputTabKey:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:tabKeyButton];
    
    logClearButton.frame = CGRectMake(470, 5, 50, 50);
    [logClearButton setTitle:@"clear" forState:UIControlStateNormal];
    [logClearButton addTarget:self action:@selector(clearLog:) forControlEvents:UIControlEventTouchDown];
    [buttonLabel addSubview:logClearButton];

    mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    
    [self.view addSubview:mainScrollView];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    return NO;
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    if(toInterfaceOrientation == UIInterfaceOrientationPortrait){
        mainScrollView.frame = self.view.bounds;
        buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
        inputLabel.frame = CGRectMake(10, self.view.bounds.size.height - 85, 30, 30);
        inputField.frame = CGRectMake(45, self.view.bounds.size.height - 80, self.view.bounds.size.width - 55, 30);
        mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    }
    else
    if(toInterfaceOrientation == UIInterfaceOrientationPortraitUpsideDown){
        mainScrollView.frame = self.view.bounds;
        buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
        inputLabel.frame = CGRectMake(10, self.view.bounds.size.height - 85, 30, 30);
        inputField.frame = CGRectMake(45, self.view.bounds.size.height - 80, self.view.bounds.size.width - 55, 30);
        mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
        
    }
    else
    if(toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft){
        mainScrollView.frame = self.view.bounds;
        buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
        inputLabel.frame = CGRectMake(10, self.view.bounds.size.height - 85, 30, 30);
        inputField.frame = CGRectMake(45, self.view.bounds.size.height - 80, self.view.bounds.size.width - 55, 30);
        mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    }
    else
    if(toInterfaceOrientation == UIInterfaceOrientationLandscapeRight){
        mainScrollView.frame = self.view.bounds;
        buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
        inputLabel.frame = CGRectMake(10, self.view.bounds.size.height - 85, 30, 30);
        inputField.frame = CGRectMake(45, self.view.bounds.size.height - 80, self.view.bounds.size.width - 55, 30);
        mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if(appMode || commandMode)
    {
        [_serverAccess commandExec:@"\n"];
        textField.text = @" ";
        commandMode = NO;
    }
    return NO;
}

- (void)commandResultLoad:(NSString*)result:(const char*)command
{
    [_terminalLog appendLog:result];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    if(!keyboardObserving)
    {
        NSNotificationCenter* center;
        center = [NSNotificationCenter defaultCenter];
        [center addObserver:self
                   selector:@selector(keyboardWillAppear:)
                       name:UIKeyboardWillShowNotification
                     object:nil];
        [center addObserver:self
                   selector:@selector(keyboardWillDisappear:)
                       name:UIKeyboardWillHideNotification
                     object:nil];
        
        keyboardObserving = TRUE;
    }
    mainScrollView.frame = self.view.bounds;
    buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
    textLogView.frame = CGRectMake(0, 10, self.view.bounds.size.width, self.view.bounds.size.height - 60);
    
    UIFont* font = [UIFont fontWithName:@"Courier" size:TERMINAL_FONT_SIZE_LARGE];
    [textLogView setFont:font];
    terminalAppView = [[TerminalAppView alloc] initWithTextFont:font color:[UIColor greenColor]];
    _terminalLog = [[TerminalLog alloc] initWithLogSize:TERMINAL_LOG_BUFFER :TERMINAL_SCROLL_BUFFER_LINE :font :self.view.bounds.size.width :textLogView.frame.size.height :self];
    [_serverAccess terminalInit];
}


- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [_terminalLog exitThreadRunning];
    [_serverAccess disconnectServer];
    
    if(keyboardObserving)
    {
        NSNotificationCenter* center;
        center = [NSNotificationCenter defaultCenter];
        [center removeObserver:self
                          name:UIKeyboardWillShowNotification
                        object:nil];
        [center removeObserver:self
                          name:UIKeyboardWillHideNotification
                        object:nil];
        
        keyboardObserving = FALSE;
    }
}

- (void)keyboardWillAppear:(NSNotification*)notification
{
    NSDictionary* userInfo;
    userInfo = [notification userInfo];
    
    mainScrollView.frame = self.view.bounds;
    mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    
    CGFloat overlap;
    CGRect keyboardFrame;
    CGRect textViewFrame;
    keyboardFrame = [[userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    keyboardFrame = [mainScrollView.superview convertRect:keyboardFrame fromView:nil];
    textViewFrame = mainScrollView.frame;
    overlap = MAX(0.0f, CGRectGetMaxY(textViewFrame) - CGRectGetMinY(keyboardFrame));
    
    UIEdgeInsets insets;
    insets = UIEdgeInsetsMake(0.0f, 0.0f, overlap, 0.0f);
    
    NSTimeInterval duration;
    UIViewAnimationCurve animationCurve;
    void (^animations)(void);
    duration = [[userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    animationCurve = [[userInfo objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue];
    animations = ^(void) {
        mainScrollView.contentInset = insets;
        mainScrollView.scrollIndicatorInsets = insets;
    };
    
    [UIView
     animateWithDuration:duration
     delay:0.0
     options:(animationCurve << 16)
     animations:animations
     completion:nil];
    
    CGRect rect;
    rect.origin.x = 0.0f;
    rect.origin.y = mainScrollView.contentSize.height - 1.0f;
    rect.size.width = CGRectGetWidth(mainScrollView.frame);
    rect.size.height = 1.0f;
    [mainScrollView scrollRectToVisible:rect animated:YES];
    
    buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
    textLogView.frame = CGRectMake(0, overlap + 10, self.view.bounds.size.width, self.view.bounds.size.height - 60 - overlap);
    CGRect rect2;
    rect2.origin.x = 0.0f;
    rect2.origin.y = textLogView.contentSize.height - 1.0f;
    rect2.size.width = CGRectGetWidth(textLogView.frame);
    rect2.size.height = 1.0f;
    [textLogView scrollRectToVisible:rect2 animated:YES];
    textLogView.changeLogFlag = FALSE;
    [_terminalLog setDrawPoint:textLogView.contentOffset.y];
    [keyboardButton removeTarget:self action:@selector(keyboardUp:) forControlEvents:UIControlEventTouchDown];
    [keyboardButton addTarget:self action:@selector(keyboardDown:) forControlEvents:UIControlEventTouchDown];
}

- (void)keyboardWillDisappear:(NSNotification*)notification
{
    NSDictionary* userInfo;
    userInfo = [notification userInfo];
    
    NSTimeInterval duration;
    UIViewAnimationCurve animationCurve;
    void (^animations)(void);
    duration = [[userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    animationCurve = [[userInfo objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue];
    animations = ^(void) {
        mainScrollView.contentInset = UIEdgeInsetsZero;
        mainScrollView.scrollIndicatorInsets = UIEdgeInsetsZero;
    };
    [UIView
     animateWithDuration:duration
     delay:0.0
     options:(animationCurve << 16)
     animations:animations
     completion:nil];
    
    mainScrollView.frame = self.view.bounds;
    buttonLabel.frame = CGRectMake(0, self.view.bounds.size.height - 50, self.view.bounds.size.width, 60);
    textLogView.frame = CGRectMake(0, 10, self.view.bounds.size.width, self.view.bounds.size.height - 60);
    mainScrollView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 10);
    textLogView.changeLogFlag = FALSE;
    [_terminalLog setDrawPoint:textLogView.contentOffset.y];
    [keyboardButton removeTarget:self action:@selector(keyboardDown:) forControlEvents:UIControlEventTouchDown];
    [keyboardButton addTarget:self action:@selector(keyboardUp:) forControlEvents:UIControlEventTouchDown];
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    VirtualTextView* textView = (VirtualTextView*)scrollView;
    if(textView.tag == 300)
    {
        if(fabs(textView.preDrawPoint - textView.contentOffset.y) > textView.frame.size.height)
        {
            textView.changeLogFlag = FALSE;
            [_terminalLog setDrawPoint:textLogView.contentOffset.y];
        }
    }
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    if(commandMode || appMode){
        if(![string length])
        {
            [_serverAccess commandExec:@"\b"];
        }
        else
        if(ctrlLock)
        {
            ctrlLock = NO;
            [ctrlKeyButton setTitleColor:[escapeKeyButton currentTitleColor] forState:UIControlStateNormal];
            [ctrlKeyButton removeTarget:self action:@selector(offCtrlKey:) forControlEvents:UIControlEventTouchDown];
            [ctrlKeyButton addTarget:self action:@selector(onCtrlKey:) forControlEvents:UIControlEventTouchDown];
            char ch = [string characterAtIndex:0];
            char ctrCh = ch - 0x60;
            [_serverAccess commandExec:[NSString stringWithCString:&ctrCh encoding:NSASCIIStringEncoding]];
        }
        else
        {
            [_serverAccess commandExec:string];
        }
    }
    return NO;
}

- (void)keyboardDown:(UIButton*)btn
{
    [inputField resignFirstResponder];
}

- (void)keyboardUp:(UIButton*)btn
{
    [inputField becomeFirstResponder];
}

- (void)inputUpKey:(UIButton*)btn
{
    if(appMode || commandMode)
    {
        [_serverAccess commandExec:@"\e[A"];
    }
}

- (void)inputDownKey:(UIButton*)btn
{
    if(appMode || commandMode)
    {
        [_serverAccess commandExec:@"\e[B"];
    }
}

- (void)inputEscKey:(UIButton*)btn
{
    if(appMode || commandMode)
    {
        [_serverAccess commandExec:@"\e"];
    }
}

- (void)onCtrlKey:(UIButton*)btn
{
    if(appMode || commandMode)
    {
        ctrlLock = YES;
        [ctrlKeyButton setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
        [ctrlKeyButton removeTarget:self action:@selector(onCtrlKey:) forControlEvents:UIControlEventTouchDown];
        [ctrlKeyButton addTarget:self action:@selector(offCtrlKey:) forControlEvents:UIControlEventTouchDown];
    }
}

- (void)offCtrlKey:(UIButton*)btn
{
    ctrlLock = NO;
    [ctrlKeyButton setTitleColor:[escapeKeyButton currentTitleColor] forState:UIControlStateNormal];
    [ctrlKeyButton removeTarget:self action:@selector(offCtrlKey:) forControlEvents:UIControlEventTouchDown];
    [ctrlKeyButton addTarget:self action:@selector(onCtrlKey:) forControlEvents:UIControlEventTouchDown];
}

- (void)inputTabKey:(UIButton*)btn
{
    if(appMode || commandMode)
    {
        [_serverAccess commandExec:@"\t"];
    }
}

- (void)clearLog:(UIButton*)btn
{
    if(!appMode && !commandMode)
    {
        [_terminalLog logReset];
        commandMode = YES;
        [textLogView scrollsToTop];
        [_terminalLog setDrawPoint:textLogView.contentOffset.y];
    }
}

- (void)notifyChangeLogFinish:(NSInteger)line
{
    [textLogView setLineNum:line];
    textLogView.changeLogFlag = TRUE;
    [_terminalLog setDrawPoint:textLogView.contentOffset.y];
}

- (void)setPrintText:(NSString *)printLog:(CGFloat)startPoint:(CGFloat)point
{
    textLogView.preDrawPoint = point;
    [textLogView setTextForDraw:printLog :startPoint];
}

- (void)notifyChangeAppMode
{
    appMode = YES;
    [self performSelectorOnMainThread:@selector(appViewShow) withObject:nil waitUntilDone:NO];
}

- (void)appViewShow
{
    terminalAppView.frame = textLogView.frame;
    terminalAppView.backgroundColor = [UIColor blackColor];
    [textLogView removeFromSuperview];
    [mainScrollView addSubview:terminalAppView];
}

- (void)notifyChangeAppLogFinish:(NSString*)appLogString
{
    [terminalAppView setAppText:appLogString];
}

- (void)notifyChangeAppModeOff
{
    appMode = NO;
    [self performSelectorOnMainThread:@selector(appViewRemove) withObject:nil waitUntilDone:NO];
}

- (void)appViewRemove
{
    textLogView.frame = terminalAppView.frame;
    [terminalAppView removeFromSuperview];
    [mainScrollView addSubview:textLogView];
}

@end