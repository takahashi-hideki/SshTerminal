//
//  AppDelegate.h
//  SshTerminal
//
//  Created by takahashi-hideki on 12/09/25.
//  Copyright (c) 2012年 takahashi-hideki. All rights reserved.
//

#import <UIKit/UIKit.h>

@class AccessViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
    UIWindow* window;
    UINavigationController* navigationController;
    AccessViewController* viewController;
}


@end
