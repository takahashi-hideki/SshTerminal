
@interface NSString (LineCount)

- (NSInteger)lineCount;
- (NSInteger)lineCountWithFont:(UIFont*)font:(CGSize)constrainedToSize:(NSLineBreakMode)lineBreakMode;
- (NSString*)substringWithLine:(NSInteger)startLine:(NSInteger)numLine;
- (NSString*)substringWithLineAndFont:(NSInteger)startLine:(NSInteger)numLine:(UIFont*)font:(CGSize)constrainedToSize:(NSLineBreakMode)lineBreakMode;
- (NSRange)rangeOfStringWithLine:(NSInteger)startLine:(NSInteger)numLine;
- (NSString*)removeLine:(NSInteger)startLine:(NSInteger)numLine;
- (NSInteger)getNextLineLocation:(NSInteger)lineLocation:(UIFont*)font:(CGSize)constrainedToSize:(NSLineBreakMode)lineBreakMode;
+ (NSInteger)getCountNewLine:(NSString*)string:(NSString*)appendString:(UIFont*)font:(CGSize)constrainedToSize:(NSLineBreakMode)lineBreakMode;

@end