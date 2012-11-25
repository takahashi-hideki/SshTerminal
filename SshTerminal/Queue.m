#import "Queue.h"

@implementation Queue

- (id)init
{
    self = [super init];
    queue = [[NSMutableArray alloc] init];
    return self;
}

- (void)dealloc
{
    [queue release];
    [super dealloc];
}

- (id)dequeue
{
    id head;
    if([queue count] == 0)
    {
        return nil;
    }
    head = [queue objectAtIndex:0];
    if(head != nil)
    {
        [[head retain] autorelease];
        [queue removeObjectAtIndex:0];
    }
    return head;
}

- (void)enqueue:(id)object
{
    if(object == nil)
    {
        return;
    }
    [queue addObject:object];
}

- (void)push:(id)object
{
    if(object == nil)
    {
        return;
    }
    [queue insertObject:object atIndex:0];
}

- (NSInteger)count
{
    NSInteger queueCount;
    queueCount = [queue count];
    return queueCount;
}

@end