#import "TerminalAppView.h"
#include "ssh_typedef.h"

@implementation TerminalAppView

- (id)initWithTextFont:(UIFont *)font color:(UIColor *)color
{
    self = [super init];
    textView = [[AppTextView alloc] init];
    textView.textFont = font;
    textView.textColor = color;
 
    textView.frame = CGRectMake(0, 0, self.bounds.size.width, 0);
    self.contentSize = textView.frame.size;
    [self addSubview:textView];
    
    return self;
}

- (void)dealloc
{
    [textView release];
    [super dealloc];
}

- (void)setAppText:(NSString *)appText
{
    textView.appString = appText;
    textViewSize = [appText sizeWithFont:textView.textFont constrainedToSize:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX) lineBreakMode:NSLineBreakByWordWrapping];
    [self performSelectorOnMainThread:@selector(appTextDraw) withObject:nil waitUntilDone:NO];
}

- (void)setTextFont:(UIFont *)font
{
    textView.textFont = font;
}

- (void)setTextColor:(UIColor *)color
{
    textView.textColor = color;
}

- (void)appTextDraw
{
    textView.frame = CGRectMake(0, 0, textViewSize.width, textViewSize.height);
    self.contentSize = textViewSize;
    [textView setNeedsDisplay];
}

@end