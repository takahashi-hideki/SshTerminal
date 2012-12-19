#import "AppTextView.h"

@implementation AppTextView
@synthesize appString;
@synthesize textFont;
@synthesize textColor;


- (id)init
{
    self = [super init];
    appString = [[NSString alloc] init];
    self.clearsContextBeforeDrawing = TRUE;
    self.opaque = NO;
    return self;
}

- (void)dealloc
{
    [appString release];
    [super dealloc];
}

- (void)drawRect:(CGRect)rect
{
    [textColor set];
    [self.appString drawInRect:self.frame withFont:textFont lineBreakMode:NSLineBreakByWordWrapping];
}

@end