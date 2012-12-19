#import <UIKit/UIKit.h>
#import "TerminalTextView.h"

@interface VirtualTextView : UIScrollView <TerminalTextViewDelegate>
{
    TerminalTextView* textView;
    
    CGFloat oneLineHeight;
    NSInteger lineNum;
    CGFloat drawPoint;
    BOOL changeLogFlag;
    CGFloat preBottomPoint;
}

@property(nonatomic, assign)BOOL changeLogFlag;
@property(nonatomic, assign)CGFloat preDrawPoint;

- (void)setFont:(UIFont*)uiFont;
- (void)setTextColor:(UIColor*)color;
- (void)setTextForDraw:(NSString*)printText:(CGFloat)startPoint;
- (void)setLineNum:(NSInteger)line;
- (CGFloat)getPreDrawPoint;

@end