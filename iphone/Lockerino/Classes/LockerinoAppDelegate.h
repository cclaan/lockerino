//
//  LockerinoAppDelegate.h
//  Lockerino
//
//  Created by Chris Laan on 9/25/11.
//  Copyright 2011 Laan Labs. All rights reserved.
//

#import <UIKit/UIKit.h>

@class LockerinoViewController;

@interface LockerinoAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    LockerinoViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet LockerinoViewController *viewController;

@end

