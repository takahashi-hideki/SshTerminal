#import <UIKit/UIKit.h>

@protocol TerminalTextViewDelegate;

@interface TerminalTextView : UIView
{
    NSMutableString* text;
    UIFont* font;
    UIColor* textColor;
    
    NSString* printString;
    CGFloat point;
    
    NSMutableArray* lineIndex;
    CGSize printSize;
    
    id<TerminalTextViewDelegate> delegate;
}

@property(nonatomic, retain)NSMutableString* text;
@property(nonatomic, retain)UIFont* font;
@property(nonatomic, retain)UIColor* textColor;
@property(nonatomic, retain)NSString* printString;
@property(nonatomic, assign)CGFloat point;
@property(nonatomic, assign)CGSize printSize;
@property(nonatomic, assign)id<TerminalTextViewDelegate> delegate;


- (void)preDraw;

@end

@protocol TerminalTextViewDelegate

- (void)scrollToBottom;

@end