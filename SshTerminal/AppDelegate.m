//
//  AppDelegate.m
//  SshTerminal
//
//  Created by takahashi-hideki on 12/09/25.
//  Copyright (c) 2012年 takahashi-hideki. All rights reserved.
//

#import "AppDelegate.h"
#import "AccessViewController.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    viewController = [[AccessViewController alloc] init];
    navigationController = [[UINavigationController alloc]
                            initWithRootViewController:viewController];
    [window addSubview:navigationController.view];
    [window makeKeyAndVisible];
}

@end
