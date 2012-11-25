#import "VirtualTextView.h"
#import "NSStringLineAddition.h"
#import "memoryDebug.h"

#define OVER_DRAW_LINE 60

@implementation VirtualTextView
@synthesize changeLogFlag;
@synthesize preDrawPoint;

- (id)init
{
    self = [super init];
    textView = [[TerminalTextView alloc] initWithFrame:CGRectMake(0, 0, self.bounds.size.width, 0)];
    textView.backgroundColor = [UIColor blackColor];
    textView.delegate = self;
    drawPoint = 0.0;
    preBottomPoint = 0.0;
    changeLogFlag = TRUE;
    preDrawPoint = 0.0;
    self.contentSize = CGSizeMake(self.bounds.size.width, 0);
    [self addSubview:textView];
    return self;
}

- (void)dealloc
{
    [textView release];
    [super dealloc];
}

//set color print text
- (void)setTextColor:(UIColor *)color
{
    textView.textColor = color;
}

//set font print text
- (void)setFont:(UIFont *)uiFont
{
    textView.font = uiFont;
    CGSize size;
    size = [@"a" sizeWithFont:textView.font];
    oneLineHeight = size.height;
    [self textViewSizeChange];
}

//set text and parameter for draw
- (void)setTextForDraw:(NSString *)printText:(CGFloat)startPoint
{
    CGFloat startLinePoint;
    CGFloat linepoint;
    
    drawPoint = startPoint;
    if(startPoint <= 0.0)
    {
        startLinePoint = 0.0;
    }
    else
    {
        linepoint = startPoint / oneLineHeight;
        if(linepoint == floor(linepoint))
        {
            linepoint = linepoint - 1.0;
        }
        startLinePoint = floor(linepoint) * oneLineHeight;
    }
    textView.printString = printText;
    textView.printSize = [textView.printString sizeWithFont:textView.font constrainedToSize:CGSizeMake(self.bounds.size.width, CGFLOAT_MAX) lineBreakMode:UILineBreakModeCharacterWrap];
    textView.point = startLinePoint;
    [self performSelectorOnMainThread:@selector(drawText) withObject:nil waitUntilDone:NO];
}


- (void)drawText
{
    [self textViewSizeChange];
    [textView preDraw];
}

//set log line number
- (void)setLineNum:(NSInteger)line
{
    lineNum = line;
}

- (void)textViewSizeChange
{
    preBottomPoint = self.contentSize.height;
    self.contentSize = CGSizeMake(self.bounds.size.width, (oneLineHeight * lineNum));
    
}

- (CGFloat)getPreDrawPoint
{
    return textView.point;
}

//text log view scroll to bottom
- (void)scrollToBottom
{
    if(changeLogFlag)
    {
        [self scrollRectToVisible:CGRectMake(0, self.contentSize.height - 1.0, self.bounds.size.width, 1.0) animated:YES];
    }
}

@end