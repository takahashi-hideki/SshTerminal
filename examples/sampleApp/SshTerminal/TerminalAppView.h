#import <UIKit/UIKit.h>
#import "AppTextView.h"

@interface TerminalAppView : UIScrollView
{
    AppTextView* textView;
    CGSize textViewSize;
}

- (id)initWithTextFont:(UIFont*)font color:(UIColor*)color;
- (void)setAppText:(NSString*)appText;
- (void)setTextFont:(UIFont*)font;
- (void)setTextColor:(UIColor*)color;


@end