//
//  ModelController.h
//  SshTerminal
//
//  Created by takahashi-hideki on 12/09/25.
//  Copyright (c) 2012年 takahashi-hideki. All rights reserved.
//

#import <UIKit/UIKit.h>

@class DataViewController;

@interface ModelController : NSObject <UIPageViewControllerDataSource>

- (DataViewController *)viewControllerAtIndex:(NSUInteger)index storyboard:(UIStoryboard *)storyboard;
- (NSUInteger)indexOfViewController:(DataViewController *)viewController;

@end
