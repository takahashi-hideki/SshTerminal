//
//  RootViewController.h
//  SshTerminal
//
//  Created by takahashi-hideki on 12/09/25.
//  Copyright (c) 2012年 takahashi-hideki. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface RootViewController : UIViewController <UIPageViewControllerDelegate>

@property (strong, nonatomic) UIPageViewController *pageViewController;

@end
