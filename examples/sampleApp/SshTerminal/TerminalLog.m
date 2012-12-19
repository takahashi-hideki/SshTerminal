#import "TerminalLog.h"
#import "memoryDebug.h"

#include "string_misc.h"
#include "escapesequence.h"
#include "ssh_typedef.h"
#include "ssh_terminal_typedef.h"

@implementation TerminalLog
@synthesize terminalLogDelegate;

- (id)initWithLogSize:(int)logSize :(int)logLine :(UIFont *)font :(CGFloat)uiWidth :(CGFloat)uiHeight :(id)delegate
{
    self = [super init];
    resultStringQueue = [[Queue alloc] init];
    drawPoint = -1000.0;
    appMode = NO;
    wrapAroundMode = NO;
    maxLogSize = logSize;
    maxLogLine = logLine;
    log = (char*)malloc(sizeof(char)*maxLogSize);
    index = (int*)malloc(sizeof(int)*maxLogLine);
    log[0] = '_';
    log[1] = '\0';
    for(int i = 0; i < maxLogLine; i++)
    {
        index[i] = 0;
    }
    appLog = NULL;
    logWidth = DEFAULT_PTY_SIZE_WIDTH;
    logHeight = DEFAULT_PTY_SIZE_HEIGHT;
    curLogSize = strlen(log);
    lineNum = 1;
    uiFont = font;
    constrainedToSize = CGSizeMake(uiWidth, CGFLOAT_MAX);
    CGSize size = [@"a" sizeWithFont:uiFont constrainedToSize:constrainedToSize lineBreakMode:UILineBreakModeCharacterWrap];
    oneLineHeight = size.height;
    uiVisibleHeight = uiHeight;
    terminalLogDelegate = delegate;
    resultStringQueueCondition = [[NSCondition alloc] init];
    resultAnalysisThread = [[NSThread alloc] initWithTarget:self selector:@selector(resultAnalysisThreadRun) object:nil];
    drawReadyThread = [[NSThread alloc] initWithTarget:self selector:@selector(drawReadyThreadRun) object:nil];
    [resultAnalysisThread start];
    [drawReadyThread start];
    
    return self;
}

- (void)dealloc
{
    free(log);
    free(index);
    [self freeAppLog];
    [resultStringQueue release];
    [resultAnalysisThread release];
    [drawReadyThread release];
    [resultStringQueueCondition release];
    [super dealloc];
}

- (void)exitThreadRunning
{
    [resultAnalysisThread cancel];
    [resultStringQueueCondition lock];
    [resultStringQueueCondition signal];
    [resultStringQueueCondition unlock];
    
    [drawReadyThread cancel];
    [drawPointCondition lock];
    [drawPointCondition signal];
    [drawPointCondition unlock];
    
    while(![resultAnalysisThread isFinished] || ![drawReadyThread isFinished])
    {
        [NSThread sleepForTimeInterval:0.1f];
    }
}

- (void)appendLog:(NSString*)result
{
    [resultStringQueueCondition lock];
    [resultStringQueue enqueue:result];
    [resultStringQueueCondition signal];
    [resultStringQueueCondition unlock];
}

- (void)backLog:(NSString*)backString
{
    [resultStringQueueCondition lock];
    [resultStringQueue push:backString];
    [resultStringQueueCondition signal];
    [resultStringQueueCondition unlock];
}

- (void)resultAnalysisThreadRun
{
    @autoreleasepool {
        NSString* analysisString;
        NSString* appString;
        while(![[NSThread currentThread] isCancelled])
        {
           
            [resultStringQueueCondition lock];
            while([resultStringQueue count] == 0)
            {
                [resultStringQueueCondition wait];
                if([[NSThread currentThread] isCancelled])
                {
                    [NSThread exit];
                }
            }
            if(appMode)
            {
                analysisString = [resultStringQueue dequeue];
                [resultStringQueueCondition unlock];
                appString = [self appModeResultAnalysis:analysisString];
                [terminalLogDelegate notifyChangeAppLogFinish:appString];
            }
            else
            {
                analysisString = [[resultStringQueue dequeue] stringByAppendingString:@"_"];
                [resultStringQueueCondition unlock];
                [self addLineIndex:analysisString];
                [terminalLogDelegate notifyChangeLogFinish :lineNum];
            }
            
        }
        [NSThread exit];
    }
}

- (void)addLineIndex:(NSString*)analysisString
{
    const char* string = [analysisString cStringUsingEncoding:NSUTF8StringEncoding];
    char* curLocation;
    char* nextLocation;
    char* lastLine;
    char* event_position;
    int event_code;
    int lastLineIndex = index[lineNum - 1];
    int lineIndex;
    int preLastIndex;
    NSString* newString;
    NSString* appString;
    CGSize size;
    CGFloat newStringHeight;
    NSInteger newLineNum;
    
    lastLine = log + lastLineIndex;
    log[strlen(log) - 1] = '\0';  //delete '_'
    preLastIndex = strlen(log);
    strcat(log, string);
    while((event_code = analysis_escapesequence(log, preLastIndex, &event_position)) != 0)
    {
        preLastIndex = event_position - log;
        if(event_code == WRAP_AROUND_MODE)
        {
            //wrap around mode
            wrapAroundMode = YES;
        }
        else
        if(event_code == APP_MODE_ON)
        {
            //app mode
            appMode = YES;
            event_position[strlen(event_position) - 1] = '\0';
            appString = [NSString stringWithCString:event_position encoding:NSASCIIStringEncoding];
            log[preLastIndex] = '_';
            log[preLastIndex + 1] = '\0';
            [self backLog:appString];
            [self mallocAppLog];
            [terminalLogDelegate notifyChangeAppMode];
            break;
        }
    }
    newString = [NSString stringWithCString:lastLine encoding:NSUTF8StringEncoding];
    size = [newString sizeWithFont:uiFont constrainedToSize:constrainedToSize lineBreakMode:UILineBreakModeCharacterWrap];
    newStringHeight = size.height;
    newLineNum = (NSInteger)(newStringHeight / oneLineHeight);
    if(newLineNum == 1)
    {
        return;
    }
    curLocation = lastLine;
    for(int i = 1; i < newLineNum; i++)
    {
        nextLocation = [self getNextLocation:curLocation];
        lineIndex = nextLocation - log;
        index[lineNum] = lineIndex;
        lineNum++;
        curLocation = nextLocation;
    }
}

- (char*)getNextLocation:(char*)startLocation
{
    @autoreleasepool {
    char* lineEnd;
    char* nextLocation = NULL;
    char* chSubLine;
    int subLineLength;
    NSString* subString;
    CGFloat height;
    NSInteger subStringLineNum;
    int i;
    CGSize size;
    
    
    lineEnd = get_a_line(startLocation);
    subLineLength = lineEnd - startLocation + 1;
    chSubLine = (char*)malloc(sizeof(char)*subLineLength + 1);
    strncpy(chSubLine, startLocation, subLineLength);
    chSubLine[subLineLength] = '\0';
    subString = [NSString stringWithCString:chSubLine encoding:NSUTF8StringEncoding];
    free(chSubLine);
    size = [subString sizeWithFont:uiFont constrainedToSize:constrainedToSize lineBreakMode:UILineBreakModeCharacterWrap];
    height = size.height;
    subStringLineNum = (NSInteger)(height / oneLineHeight);
    if(subStringLineNum == 1)
    {
        nextLocation = lineEnd + 1;
    }
    else
    {
        for(i = (subString.length / subStringLineNum) + 1; i < subString.length; i++)
        {
            size = [[subString substringWithRange:NSMakeRange(0, i)] sizeWithFont:uiFont constrainedToSize:constrainedToSize lineBreakMode:UILineBreakModeCharacterWrap];
            height = size.height;
            if(height != oneLineHeight)
            {
                nextLocation = startLocation + i - 1;
                break;
            }
        }
    }
    
    return nextLocation;
    }
}

- (void)setDrawPoint:(CGFloat)point
{
    [drawPointCondition lock];
    drawPoint = point;
    [drawPointCondition signal];
    [drawPointCondition unlock];
}

- (void)drawReadyThreadRun
{
    @autoreleasepool {
        CGFloat point;
        CGFloat startPoint;
        CGFloat endPoint;
        NSInteger startLine;
        NSInteger endLine;
        NSString* printLog;
        while(![[NSThread currentThread] isCancelled])
        {
            [drawPointCondition lock];
            while(drawPoint < -500.0)
            {
                [drawPointCondition wait];
                if([[NSThread currentThread] isCancelled])
                {
                    [NSThread exit];
                }
            }
            point = drawPoint;
            drawPoint = -1000.0;
            [drawPointCondition unlock];
            
            startPoint = point - uiVisibleHeight;
            endPoint = point + (uiVisibleHeight * 2);
            
            if(startPoint < 0.0)
            {
                startPoint = 0.0;
            }
            if(endPoint > lineNum * oneLineHeight)
            {
                endPoint = lineNum * oneLineHeight;
            }
            
            startLine = [self pointToLine:startPoint];
            endLine = [self pointToLine:endPoint];
            printLog = [self lineToString:startLine :endLine];
            [drawPointCondition lock];
            if(drawPoint < -500.0)
            {
                [terminalLogDelegate setPrintText:printLog :startPoint :point];
            }
            [drawPointCondition unlock];
        }
        [NSThread exit];
    }
}

- (NSInteger)pointToLine:(CGFloat)point
{
    NSInteger line;
    if(point < 0.0)
    {
        line = 0;
    }
    else
    if(point == lineNum * oneLineHeight)
    {
        line = lineNum - 1;
    }
    else
    {
        line = (NSInteger)(point / oneLineHeight);
    }
    
    return line;
}

- (NSString*)lineToString:(NSInteger)startLine:(NSInteger)endLine
{
    NSInteger startLocation;
    NSInteger endLocation;
    NSString* printLog;
    char* chPrintLog;
    char* startLog;
    if(startLine < 0 || endLine < 0)
    {
        printLog = NULL;
    }
    else
    if(startLine > endLine)
    {
        printLog = NULL;
    }
    else
    {
        startLocation = index[startLine];
        if(endLine == (lineNum - 1))
        {
            endLocation = strlen(log);
        }
        else
        {
            endLocation = index[endLine + 1];
        }
        chPrintLog = (char*)malloc(sizeof(char) * (endLocation - startLocation + 1));
        startLog = log + startLocation;
        strncpy(chPrintLog, startLog, (endLocation - startLocation));
        chPrintLog[(endLocation - startLocation)] = '\0';
        printLog = [NSString stringWithCString:chPrintLog encoding:NSASCIIStringEncoding];
        free(chPrintLog);
    }
    
    return printLog;
}

- (void)mallocAppLog
{
    int i;
    if(appLog != NULL)
    {
        [self freeAppLog];
    }
    
    appLog = (char**)malloc(sizeof(char*) * logHeight);
    appLog[0] = (char*)malloc(sizeof(char) * logHeight * (logWidth + 1));
    
    for(i = 0; i < logHeight; i++)
    {
        appLog[i] = appLog[0] + i * (logWidth + 1);
        memset(appLog[i], '\0', logWidth + 1);
    }
    appLog[0][0] = ' ';
    cursor[0] = 0;
    cursor[1] = 0;
    appLigion[0] = 0;
    appLigion[1] = DEFAULT_PTY_SIZE_HEIGHT - 1;
}

- (void)freeAppLog
{
    if(appLog == NULL)
    {
        return;
    }
    free(appLog[0]);
    free(appLog);
    appLog = NULL;
}

- (NSString*)appModeResultAnalysis:(NSString*)analysisString
{
    const char* string = [analysisString cStringUsingEncoding:NSUTF8StringEncoding];
    int event_code;
    const char* event_position;
    const char* cur_position = string;
    NSString* appLogString;
    NSMutableString* appLogMutableString;
    NSString* logString;
    int i;
    
    while((event_code = analysis_escapesequence_app_mode(cur_position, appLog, cursor, &event_position, logWidth, logHeight, appLigion)) != 0)
    {
        if(event_code == WRAP_AROUND_MODE)
        {
            wrapAroundMode = YES;
        }
        else
        if(event_code == APP_MODE_OFF)
        {
            appMode = NO;
            logString = [NSString stringWithCString:event_position encoding:NSASCIIStringEncoding];
            [self backLog:logString];
            [terminalLogDelegate notifyChangeAppModeOff];
            break;
        }
        cur_position = event_position;
    }
    
    appLogMutableString = [NSMutableString stringWithCString:appLog[0] encoding:NSASCIIStringEncoding];
    for(i = 1; i < logHeight; i++)
    {
        if(strlen(appLog[i]) == 0)
        {
            break;
        }
        [appLogMutableString appendFormat:@"\n%s", appLog[i]];
    }
    
    appLogString = appLogMutableString;
    
    
    return appLogString;
}

- (void)logReset
{
    [self textLogClear];
}

- (void)textLogClear
{
    char* tmpLastLine;
    char* lastLine;
    int lastLineIndex;
    
    lastLineIndex = index[lineNum - 1];
    lastLine = log + lastLineIndex;
    
    tmpLastLine = (char*)malloc(sizeof(char) * (strlen(lastLine) + 1));
    strcpy(tmpLastLine, lastLine);
    memset(log, 0, maxLogSize);
    for(int i = 0; i < maxLogLine; i++)
    {
        index[i] = 0;
    }
    strcpy(log, tmpLastLine);
    lineNum = 1;
    curLogSize = strlen(log);
}

@end