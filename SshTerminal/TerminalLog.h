#import <UIKit/UIKit.h>
#import "Queue.h"

@protocol TerminalLogDelegate;

@interface TerminalLog : NSObject
{
    BOOL appMode;
    BOOL wrapAroundMode;
    
    char* log;
    int* index;
    int lineNum;
    int curLogSize;
    int maxLogSize;
    int maxLogLine;
    
    char** appLog;
    int logWidth;
    int logHeight;
    int cursor[2];
    int appLigion[2];
    
    Queue* resultStringQueue;
    CGFloat drawPoint;
    NSInteger drawLineToOutOfVisible;
    
    int lineOffset;
    int rangeOffset;
    
    CGFloat oneLineHeight;
    CGFloat uiVisibleHeight;
    CGSize constrainedToSize;
    UIFont* uiFont;
    
    NSThread* resultAnalysisThread;
    NSThread* drawReadyThread;
    NSCondition* resultStringQueueCondition;
    NSCondition* drawPointCondition;
    
    id<TerminalLogDelegate> terminalLogDelegate;
}

@property (nonatomic, assign)id<TerminalLogDelegate> terminalLogDelegate;

- (id)initWithLogSize:(int)logSize:(int)logLine:(UIFont*)font:(CGFloat)uiWidth:(CGFloat)uiHeight:(id)delegate;
- (void)exitThreadRunning;

- (void)appendLog:(NSString*)string;
- (void)setDrawPoint:(CGFloat)point;
- (void)logReset;

@end

@protocol TerminalLogDelegate

- (void)notifyChangeLogFinish:(NSInteger)line;
- (void)setPrintText:(NSString*)printLog:(CGFloat)startPoint:(CGFloat)visiblePoint;
- (void)notifyChangeAppMode;
- (void)notifyChangeAppModeOff;
- (void)notifyChangeAppLogFinish:(NSString*)appLogString;


@end