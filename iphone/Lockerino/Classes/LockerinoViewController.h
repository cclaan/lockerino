//
//  LockerinoViewController.h
//  Lockerino
//
//  Created by Chris Laan on 9/25/11.
//  Copyright 2011 Laan Labs. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MBProgressHUD.h"
#import "ASIHTTPRequest.h"

typedef enum LockState {
	
	LOCK_STATE_UNKNOWN,
	LOCK_STATE_UNLOCKED,
	LOCK_STATE_LOCKED,
	
} LockState;

@interface LockerinoViewController : UIViewController {
	
	IBOutlet UILabel * infoLabel;
	IBOutlet UIImageView * lockImageView;
	IBOutlet UIImageView * lockOverlayImageView;
	
	NSTimer * statusTimer;
	MBProgressHUD * HUD;
	
	BOOL waitingForDesiredState;
	LockState desiredState;
	LockState lockState;
	
	UIImage * lockedImage;
	UIImage * unlockedImage;
	UIImage * timeoutImage;
	
	//NSOperationQueue * statusQueue;
	//BOOL statusPending;
	
	NSTimeInterval lockChangeRequestStarted;
	NSTimeInterval lockUpdatedPreRequest;
	
	NSTimeInterval mostRecentLockUpdatedTimestamp;
	
	ASIHTTPRequest * statusRequest;
	ASIHTTPRequest * lockChangeRequest;
	
}

@property (nonatomic, retain) NSString * baseUrl;
@property (nonatomic, retain) NSString * lockId;

-(void) pauseStatusTimer;
-(void) resumeStatusTimer;

-(IBAction) lockDoor;
-(IBAction) unlockDoor;



@end

