#import <UIKit/UIKit.h>

@interface AppTextView : UIView
{
    NSString* appString;
    UIColor* textColor;
    UIFont* textFont;
}

@property(nonatomic, retain)NSString* appString;
@property(nonatomic, retain)UIFont* textFont;
@property(nonatomic, retain)UIColor* textColor;

@end