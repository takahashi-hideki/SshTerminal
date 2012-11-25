#import "AccessViewController.h"


#define HOST_TAG 200
#define USER_TAG 201
#define PASS_TAG 202
#define PORT_TAG 203
#define PRIVATE_TAG 204
#define PUBLIC_TAG 205

@implementation AccessViewController

- (id)init
{
    self = [super init];
    self.title = @"SERVER CONNECTION";
    _serverAccess = [[ServerAccess alloc] init];
    activeField = nil;
    [self test];
    return self;
}

- (void)dealloc
{
    [_serverAccess release];
    [_terminalViewController release];
    [super dealloc];
}

- (void)test
{
    
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIScrollView* roginView = [[[UIScrollView alloc] initWithFrame:self.view.bounds] autorelease];
    roginView.scrollEnabled = YES;
    roginView.backgroundColor = [UIColor whiteColor];
    
    //login form
    UITableView* roginFormTable = [[[UITableView alloc] initWithFrame:CGRectMake(0, 0, self.view.bounds.size.width, 550) style:UITableViewStyleGrouped] autorelease];
    roginFormTable.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleLeftMargin;
    roginFormTable.backgroundView = nil;
    roginFormTable.backgroundColor = [UIColor whiteColor];
    roginFormTable.scrollEnabled = NO;
    roginFormTable.delegate = self;
    roginFormTable.dataSource = self;
    [roginView addSubview:roginFormTable];
    
    //usekey button
    NSArray* useKeyArray = [NSArray arrayWithObjects:@"password", @"key", nil];
    UISegmentedControl* useKeyControl = [[[UISegmentedControl alloc] initWithItems:useKeyArray] autorelease];
    useKeyControl.frame = CGRectMake(175, 580, 400, 50);
    useKeyControl.segmentedControlStyle = UISegmentedControlStylePlain;
    useKeyControl.selectedSegmentIndex = 0;
    [useKeyControl addTarget:self action:@selector(changeAuthenticationMethod:) forControlEvents:UIControlEventValueChanged];
    [roginView addSubview:useKeyControl];
    
    //connection button
    UIButton* connectionButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    connectionButton.frame = CGRectMake(250, 650, 250, 50);
    [connectionButton setTitle:@"connect" forState:UIControlStateNormal];
    [connectionButton addTarget:self action:@selector(connectServer:) forControlEvents:UIControlEventTouchUpInside];
    [roginView addSubview:connectionButton];
    
    roginView.contentSize = CGSizeMake(self.view.bounds.size.width, self.view.bounds.size.height + 1.0);
    [self.view addSubview:roginView];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 6;
}

- (NSString*)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    switch (section) {
        case 0:
            return @"host name";
            break;
        case 1:
            return @"user name";
            break;
        case 2:
            return @"password / passcode";
            break;
        case 3:
            return @"port";
            break;
        case 4:
            return @"private key";
            break;
        case 5:
            return @"public key";
            break;
        default:
            return @"";
            break;
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString* identifier = @"Cell";
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:identifier];
    
    if(cell == nil)
    {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:identifier] autorelease];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        UITextField* hostTextField;
        UITextField* userTextField;
        UITextField* passTextField;
        UITextField* portTextField;
        UITextField* privateTextField;
        UITextField* publicTextField;
        switch(indexPath.section)
        {
            case 0:
                hostTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                hostTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                hostTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                hostTextField.keyboardType = UIKeyboardTypeURL;
                hostTextField.returnKeyType = UIReturnKeyDone;
                hostTextField.delegate = self;
                hostTextField.tag = HOST_TAG;
                
                [cell.contentView addSubview:hostTextField];
                break;
            case 1:
                userTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                userTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                userTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                userTextField.keyboardType = UIKeyboardTypeURL;
                userTextField.returnKeyType = UIReturnKeyDone;
                userTextField.delegate = self;
                userTextField.tag = USER_TAG;
                
                [cell.contentView addSubview:userTextField];
                break;
            case 2:
                passTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                passTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                passTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                passTextField.keyboardType = UIKeyboardTypeURL;
                passTextField.returnKeyType = UIReturnKeyDone;
                passTextField.secureTextEntry = YES;
                passTextField.delegate = self;
                passTextField.tag = PASS_TAG;
                
                [cell.contentView addSubview:passTextField];
                break;
            case 3:
                portTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                portTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                portTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                portTextField.keyboardType = UIKeyboardTypeNumberPad;
                portTextField.returnKeyType = UIReturnKeyDone;
                portTextField.delegate = self;
                portTextField.tag = PORT_TAG;
                
                [cell.contentView addSubview:portTextField];
                break;
            case 4:
                privateTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                privateTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                privateTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                privateTextField.keyboardType = UIKeyboardTypeURL;
                privateTextField.returnKeyType = UIReturnKeyDone;
                privateTextField.delegate = self;
                privateTextField.tag = PRIVATE_TAG;
                
                [cell.contentView addSubview:privateTextField];
                break;
            case 5:
                publicTextField = [[[UITextField alloc] initWithFrame:CGRectMake(10.0, 10.0, 650, 24.0)] autorelease];
                publicTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
                publicTextField.autocorrectionType = UITextAutocorrectionTypeNo;
                publicTextField.keyboardType = UIKeyboardTypeURL;
                publicTextField.returnKeyType = UIReturnKeyDone;
                publicTextField.delegate = self;
                publicTextField.tag = PUBLIC_TAG;
                
                [cell.contentView addSubview:publicTextField];
                break;
            default:
                break;
        }
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}


- (void)textFieldDidBeginEditing:(UITextField *)textField
{
    activeField = textField;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    NSInteger textTag = textField.tag;
    switch(textTag)
    {
        case HOST_TAG:
            _serverAccess.hostName = textField.text;
            break;
        case USER_TAG:
            _serverAccess.userName = textField.text;
            break;
        case PASS_TAG:
            _serverAccess.passWord = textField.text;
            break;
        case PORT_TAG:
            _serverAccess.port = textField.text;
            break;
        case PRIVATE_TAG:
            _serverAccess.privateKey = textField.text;
            break;
        case PUBLIC_TAG:
            _serverAccess.publicKey = textField.text;
            break;
        default:
            break;
    }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return YES;
}

- (void)changeAuthenticationMethod:(UISegmentedControl*)seg
{
    if(seg.selectedSegmentIndex == 0)
    {
        _serverAccess.useKey = FALSE;
    }
    else
    {
        _serverAccess.useKey = TRUE;
    }
}

- (void)connectServer:(UIButton*)btn
{
    UIAlertView* alert;
    if(activeField != nil){
        [activeField resignFirstResponder];
    }
    
    if([_serverAccess connetSshServer])
    {
        _terminalViewController = [[TerminalViewController alloc] initWithServerAccess:_serverAccess];
        [self.navigationController pushViewController:_terminalViewController animated:YES];
        [_terminalViewController release];
    }
    else
    {
        alert = [[[UIAlertView alloc]
                 initWithTitle:@"connection failed" 
                 message:[NSString stringWithFormat:@"failed connect to %@@%@", _serverAccess.userName, _serverAccess.hostName]
                 delegate:nil
                 cancelButtonTitle:nil
                 otherButtonTitles:@"OK", nil] autorelease];
        [alert show];
    }
}

@end