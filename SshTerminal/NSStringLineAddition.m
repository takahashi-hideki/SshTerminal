#import "NSStringLineAddition.h"

@implementation NSString (LineCount)

- (NSInteger)lineCount
{
    NSInteger count = 0;
    NSRange range, subRange;
    NSString* lastLineString;
    NSString* lastCharacter;
    
    range = NSMakeRange(0, self.length);
    subRange = NSMakeRange(0, 0);
    
    while(range.length > 0)
    {
        subRange = [self lineRangeForRange:NSMakeRange(range.location, 0)];
        range.location = NSMaxRange(subRange);
        range.length -= subRange.length;
        count++;
    }
    
    lastLineString = [self substringWithRange:subRange];
    lastCharacter = [lastLineString substringWithRange:NSMakeRange([lastLineString length] - 1, 1)];
    if([lastCharacter isEqualToString:@"\r"] || [lastCharacter isEqualToString:@"\n"])
    {
        count++;
    }
    
    return count;
}

- (NSInteger)lineCountWithFont:(UIFont *)font :(CGSize)constrainedToSize :(NSLineBreakMode)lineBreakMode
{
    CGSize size;
    CGFloat oneLineHeight;
    CGFloat stringHeight;
    NSInteger lineNum;
    
    NSLog(@"oneline");
    size = [@"a" sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
    oneLineHeight = size.height;
    
    NSLog(@"allline");
    size = [self sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
    stringHeight = size.height;
    lineNum = (NSInteger)(stringHeight / oneLineHeight);
    
    NSLog(@"linecount end");
    return lineNum;
}

- (NSString*)substringWithLine:(NSInteger)startLine :(NSInteger)numLine
{
    NSString* subString;
    NSInteger count = 1;
    NSInteger startLocation = -1;
    NSInteger subLength = -1;
    NSRange range, subRange;
    
    if(numLine <= 0 || startLine <= 0)
    {
        return NULL;
    }
    
    range = NSMakeRange(0, self.length);
    
    while(range.length > 0)
    {
        if(count == startLine)
        {
            startLocation = range.location;
        }
        subRange = [self lineRangeForRange:NSMakeRange(range.location, 0)];
        
        range.location = NSMaxRange(subRange);
        range.length -= subRange.length;
        
        if(count == (startLine + numLine - 1))
        {
            subLength = range.location - startLocation;
            break;
        }
        count++;
    }
    if(startLocation < 0 || subLength < 0)
    {
        return NULL;
    }
    
    subString = [self substringWithRange:NSMakeRange(startLocation, subLength)];
    
    return subString;
}

- (NSString*)substringWithLineAndFont:(NSInteger)startLine :(NSInteger)numLine :(UIFont *)font :(CGSize)constrainedToSize :(NSLineBreakMode)lineBreakMode
{
    NSString* subString;
    NSInteger subLength = 0;
    NSInteger lineNum;
    NSInteger subStringLineNum;
    NSInteger startStringLineNum;
    NSInteger endStringLineNum;
    NSInteger count = 1;
    NSInteger preCount = 0;
    NSInteger preLocation;
    NSInteger startLocation = 0;
    NSInteger endLocation;
    CGFloat oneLineHeight;
    CGFloat stringHeight;
    CGFloat subStringHeight;
    CGFloat startStringHeight;
    CGFloat endStringHeight;
    CGSize size;
    NSRange range, subRange;
    BOOL startFlag = NO;
    
    int i;
    
    if(startLine <= 0 || numLine <= 0)
    {
        return NULL;
    }
    
    size = [@"a" sizeWithFont:font];
    oneLineHeight = size.height;
    size = [self sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
    stringHeight = size.height;
    
    lineNum = (NSInteger)(stringHeight / oneLineHeight);
    NSLog(@"line : %d %f %f", lineNum, stringHeight, oneLineHeight);
    NSLog(@"subline : %d", startLine + numLine -1);
    if(lineNum < (startLine + numLine - 1))
    {
        return NULL;
    }
    
    range = NSMakeRange(0, self.length);
    preLocation = 0;
    while(range.length > 0)
    {
        if(count >= startLine && !startFlag)
        {
            if(count == startLine)
            {
                startLocation = range.location;
            }
            else
            {
                for(i = 1; i < subString.length; i++)
                {
                    size = [[self substringWithRange:NSMakeRange(preLocation, i)] sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
                    startStringHeight = size.height;
                    startStringLineNum = (NSInteger)(startStringHeight / oneLineHeight);
                    if(count - startStringLineNum + 1 == startLine)
                    {
                        startLocation = preLocation + i - 1;
                    }
                }
            }
            startFlag = YES;
        }
        
        preLocation = range.location;
        
        subRange = [self lineRangeForRange:NSMakeRange(range.location, 0)];
        subString = [self substringWithRange:subRange];
        size = [subString sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
        subStringHeight = size.height;
        subStringLineNum = (NSInteger)(subStringHeight / oneLineHeight);
        
        range.location = NSMaxRange(subRange);
        range.length -= subRange.length;
        
        if(count >= (startLine + numLine - 1) && startFlag)
        {
            if(count == (startLine + numLine - 1))
            {
                if(subStringLineNum == 1)
                {
                    subLength = range.location - startLocation;
                }
                else
                {
                    for(i = 1; i < subString.length; i++)
                    {
                        size = [[self substringWithRange:NSMakeRange(preLocation, i)] sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
                        endStringHeight = size.height;
                        endStringLineNum = (NSInteger)(endStringHeight / oneLineHeight);
                        if(endStringLineNum > 1)
                        {
                            endLocation = preLocation + i - 1;
                            subLength = endLocation - startLocation;
                            break;
                        }
                    }
                }
            }
            else
            {
                for(i = 1; i < subString.length + 1; i++)
                {
                    size = [[self substringWithRange:NSMakeRange(preLocation, i)] sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
                    endStringHeight = size.height;
                    endStringLineNum = (NSInteger)(endStringHeight / oneLineHeight);
                    if((preCount + endStringLineNum - 1) == (startLine + numLine))
                    {
                        endLocation = preLocation + i - 1;
                        subLength = endLocation - startLocation;
                        break;
                    }
                }
            }
            break;
        }
        preCount = count;
        count += subStringLineNum;
    }
    
    
    NSLog(@"start : %d length : %d, self : %d", startLocation, subLength, self.length);
    if(subLength > self.length - startLocation)
    {
        subLength = self.length - startLocation;
        NSLog(@"sublength_bug");
    }
    subString = [self substringWithRange:NSMakeRange(startLocation, subLength)];
    
    return subString;
}

- (NSRange)rangeOfStringWithLine:(NSInteger)startLine :(NSInteger)numLine
{
    NSRange resultRange;
    NSInteger count = 1;
    NSInteger startLocation = -1;
    NSInteger subLength = -1;
    NSRange range, subRange;
    
    if(numLine <= 0 || startLine <= 0)
    {
        resultRange = NSMakeRange(NSNotFound, 0);
        return resultRange;
    }
    
    
    range = NSMakeRange(0, self.length);
    
    while(range.length > 0)
    {
        if(count == startLine)
        {
            startLocation = range.location;
        }
        subRange = [self lineRangeForRange:NSMakeRange(range.location, 0)];
        
        range.location = NSMaxRange(subRange);
        range.length -= subRange.length;
        
        if(count == (startLine + numLine - 1))
        {
            subLength = range.location - startLocation;
            break;
        }
        count++;
    }
    if(startLocation < 0 || subLength < 0)
    {
        resultRange = NSMakeRange(NSNotFound, 0);
        return resultRange;
    }
    resultRange = NSMakeRange(startLocation, subLength);
    
    return resultRange;

}

- (NSString*)removeLine:(NSInteger)startLine :(NSInteger)numLine
{
    NSMutableString* string;
    NSString* resultString;
    NSRange subRange;
    
    subRange = [self rangeOfStringWithLine:startLine :numLine];
    if(subRange.location == NSNotFound)
    {
        return self;
    }
    string = [NSMutableString stringWithString:self];
    [string deleteCharactersInRange:subRange];
    
    resultString = string;
    
    return resultString;
}

- (NSInteger)getNextLineLocation:(NSInteger)lineLocation :(UIFont *)font :(CGSize)constrainedToSize :(NSLineBreakMode)lineBreakMode
{
    NSRange range, subRange;
    NSString* subString;
    CGSize size;
    CGFloat oneLineHeight;
    CGFloat lineHeight;
    CGFloat subStringHeight;
    NSInteger nextLineLocation = 0;
    NSInteger lineOffset;
    
    range = NSMakeRange(lineLocation, self.length - lineLocation);
    subRange = [self lineRangeForRange:NSMakeRange(range.location, 0)];
    lineOffset = lineLocation - subRange.location;
    subRange.location += lineOffset;
    subRange.length -= lineOffset;
    subString = [self substringWithRange:subRange];
    size = [@"a" sizeWithFont:font];
    oneLineHeight = size.height;
    size = [subString sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
    subStringHeight = size.height;
    if(subStringHeight != oneLineHeight)
    {
        for(int i = 1; i < subString.length; i++)
        {
            size = [[subString substringWithRange:NSMakeRange(0, i)] sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
            lineHeight = size.height;
            if(lineHeight != oneLineHeight)
            {
                nextLineLocation = lineLocation + i - 1;
                break;
            }
        }
    }
    else
    {
        nextLineLocation = NSMaxRange(subRange);
    }
    return nextLineLocation;
}

+ (NSInteger)getCountNewLine:(NSString *)string :(NSString *)appendString :(UIFont *)font :(CGSize)constrainedToSize :(NSLineBreakMode)lineBreakMode
{
    NSString* newString;
    CGSize size;
    CGFloat oneLineHight;
    CGFloat newStringHight;
    NSInteger lineNum;
    
    newString = [string stringByAppendingString:appendString];
    size = [@"a" sizeWithFont:font];
    oneLineHight = size.height;
    size = [newString sizeWithFont:font constrainedToSize:constrainedToSize lineBreakMode:lineBreakMode];
    newStringHight = size.height;
    lineNum = (NSInteger)(newStringHight / oneLineHight);
    
    return lineNum;
}

@end