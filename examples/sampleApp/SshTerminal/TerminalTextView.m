#import "TerminalTextView.h"
#import "NSStringLineAddition.h"
#import "memoryDebug.h"

@implementation TerminalTextView

@synthesize text;
@synthesize font;
@synthesize textColor;
@synthesize printString;
@synthesize point;
@synthesize printSize;
@synthesize delegate;

- (id)init
{
    self = [super init];
    text = [[NSMutableString alloc] init];
    printString = [[NSString alloc] init];
    lineIndex = [[NSMutableArray alloc] init];
    self.opaque = NO;
    [text setString:@"_"];
    [lineIndex addObject:[NSNumber numberWithInteger:0]];
    
    return self;
}

- (void)dealloc
{
    [text release];
    [font release];
    [textColor release];
    [printString release];
    [lineIndex release];
    [super dealloc];
}

- (void)preDraw
{
    [memoryDebug reportMemory];
    self.frame = CGRectMake(0, point, printSize.width, printSize.height);
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
    [textColor set];
    [self.printString drawInRect:CGRectMake(0, 0, printSize.width, printSize.height) withFont:font lineBreakMode:UILineBreakModeCharacterWrap];
    [delegate scrollToBottom];
}

@end