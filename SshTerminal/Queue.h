@interface Queue : NSObject
{
    NSMutableArray* queue;
}

- (id)dequeue;
- (void)enqueue:(id)object;
- (void)push:(id)object;
- (NSInteger)count;


@end