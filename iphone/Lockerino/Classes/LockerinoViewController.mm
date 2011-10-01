//
//  LockerinoViewController.m
//  Lockerino
//
//  Created by Chris Laan on 9/25/11.
//  Copyright 2011 Laan Labs. All rights reserved.
//

#import "LockerinoViewController.h"
#import "Utils.h"
#import "JSONKit.h"


//static NSString * NOT_SECURE_LOCK_ID = nil;
//static NSString * APP_ENGINE_BASE_URL = nil;

#define LOCK_CHANGE_REQUEST_TIMEOUT 5

@interface LockerinoViewController()

-(void) processStatusJson:(NSData*)responseData;

-(void) showLoadingIndicator:(NSString*)str;
-(void) showCompletedIndicator:(NSString*)str;
-(void) showFailedIndicator:(NSString*) str;

-(void) getLockStatus;

-(void) setLockedImage;
-(void) setUnlockedImage;
-(void) hideOverlayImage;
-(void) showOverayImage;

@end


@implementation LockerinoViewController

@synthesize lockId, baseUrl;


- (void) awakeFromNib
{
	self = [super init];
	if (self != nil) {
		
		lockState = LOCK_STATE_UNKNOWN;
		lockedImage = [[UIImage imageNamed:@"locked.png"] retain];
		unlockedImage = [[UIImage imageNamed:@"unlocked.png"] retain];
		timeoutImage = [[UIImage imageNamed:@"timeout-overlay.png"] retain];
		
		
		[ASIHTTPRequest setShouldUpdateNetworkActivityIndicator:NO];
		
		
	}
	
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
		
	
}

-(void) viewWillAppear:(BOOL)animated {
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	self.baseUrl = [defaults stringForKey:@"app_engine_url"];
	self.lockId = [defaults stringForKey:@"unsecure_lock_id"];
	
	
	if ( self.baseUrl == nil || self.lockId == nil ) {
		
		UIAlertView * al = [[UIAlertView alloc] initWithTitle:@"Not Setup" message:@"You need to edit app setttings and enter your lock code and app engine URL.\nOpen the settings application and find 'Lockerino'" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
		[al show];
		
		return;
	}
	
	[self resumeStatusTimer];
	
}

-(void) viewWillDisappear:(BOOL)animated {
	
	[self pauseStatusTimer];
	
	
}

-(void) pauseStatusTimer {
	
	if ( statusTimer ) { 
		[statusTimer invalidate];
		statusTimer = nil;
	}
	
}

-(void) resumeStatusTimer {
	
	if ( statusTimer ) { 
		[statusTimer invalidate];
		statusTimer = nil;
	}
	
	
	statusTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0) target:self selector:@selector(getLockStatus) userInfo:nil repeats:YES];
	[self getLockStatus];
	
}

-(void) updateStatusLabel {
	
	NSTimeInterval tsNow = [[NSDate date] timeIntervalSince1970];
	NSTimeInterval diff = tsNow - mostRecentLockUpdatedTimestamp;
	

	if ( diff > 20.0 ) {
		[self showOverayImage];
	} else {
		[self hideOverlayImage];
	}

	NSString * tm = PrettyTimeAgo(diff);

	
	infoLabel.text = tm;
	
	
}

-(void) getLockStatus {
	
	[self updateStatusLabel];
	
	// we're waiting for the lock to get to a new position
	NSTimeInterval tsNow = [[NSDate date] timeIntervalSince1970];
	
	if ( waitingForDesiredState && ((tsNow - lockChangeRequestStarted) > LOCK_CHANGE_REQUEST_TIMEOUT) ) {
		desiredState = LOCK_STATE_UNKNOWN;
		waitingForDesiredState = NO;
		[self showFailedIndicator:@"Timeout!"];
	}
	
	// dont fetch status if another is pending...
	if (statusRequest != nil && [statusRequest inProgress] ) {
		NSLog(@"in progress");
		return;
	}
	
	// dont fetch status while changing...
	if (lockChangeRequest != nil && [lockChangeRequest inProgress] ) {
		NSLog(@"CHANGE in progress");
		return;
	}
	
	NSString * urlString = [NSString stringWithFormat:@"%@/api/get_observed_status?unsecure_lock_id=%@" , self.baseUrl, self.lockId];
	
	NSURL *url = [NSURL URLWithString:urlString];
	
	
	
	statusRequest = [[ASIHTTPRequest alloc] initWithURL:url];
	[statusRequest setDelegate:self];
	[statusRequest startAsynchronous];
	[statusRequest setTimeOutSeconds:3.0];
	
	
}

- (void)requestFinished:(ASIHTTPRequest *)request
{
	
	NSLog(@"%@ " , [request responseString] );
	
	NSData *responseData = [request responseData];
	
	[self processStatusJson:responseData];
	
	[statusRequest release];
	statusRequest = nil;
	
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	NSError *error = [request error];
	NSLog(@"req failed with err: %@ " , error);
	
	[statusRequest release];
	statusRequest = nil;
	
}

-(void) processStatusJson:(NSData*)responseData {
	
	id JSON = nil;
	NSError *JSONError = nil;
	
	JSON = [[JSONDecoder decoder] objectWithData:responseData error:&JSONError];
	
	//NSLog(JSONError);
	//NSLog(@"%@" , JSON);
	
	NSString * _lockStatus = [JSON valueForKeyPath:@"observed_status"];
	//NSLog(@"status: %@ " , _lockStatus );
	
	
	if ( _lockStatus && (NSNull*)_lockStatus != [NSNull null] ) {
		
		//NSTimeInterval tsNow = [[NSDate date] timeIntervalSince1970];
		NSTimeInterval tsUpdated = [[JSON valueForKeyPath:@"last_updated_ts"] doubleValue];
		
		
		mostRecentLockUpdatedTimestamp = tsUpdated;
		
		
		if ( [_lockStatus isEqualToString:@"islocked"] ) {
			lockState = LOCK_STATE_LOCKED;
			[self setLockedImage];
		} else if ( [_lockStatus isEqualToString:@"unlocked"] ) {
			lockState = LOCK_STATE_UNLOCKED;
			[self setUnlockedImage];
		} else {
			lockState = LOCK_STATE_UNKNOWN;
		}
		
		// success condition for lock to change...
		if ( waitingForDesiredState && (lockState == desiredState) ) {
			
			if ( tsUpdated != lockUpdatedPreRequest ) {
				
				if ( desiredState == LOCK_STATE_LOCKED ) {
					[self showCompletedIndicator:@"Locked!"];
				} else if ( desiredState == LOCK_STATE_UNLOCKED ) {
					[self showCompletedIndicator:@"Unlocked!"];
				}
				desiredState = LOCK_STATE_UNKNOWN;
				waitingForDesiredState = NO;
				
			}
		}
		
	}
	
	
}

-(IBAction) unlockDoor {
	
	
	NSString * urlString = [NSString stringWithFormat:@"%@/api/set_lock?status=%@&unsecure_lock_id=%@" , self.baseUrl, @"unlocked", self.lockId];
	NSURL *url = [NSURL URLWithString:urlString];
	
	lockChangeRequest = [[ASIHTTPRequest alloc] initWithURL:url];
	[lockChangeRequest setRequestMethod:@"POST"];
	[lockChangeRequest setDelegate:self];
	[lockChangeRequest setDidFinishSelector:@selector(stateChangeSuccess:)];
	[lockChangeRequest setDidFailSelector:@selector(stateChangeFailed:)];
	[lockChangeRequest startAsynchronous];
	[lockChangeRequest setTimeOutSeconds:3.0];
	[lockChangeRequest setRetryCount:2];
	[lockChangeRequest setNumberOfTimesToRetryOnTimeout:2];
	
	desiredState = LOCK_STATE_UNLOCKED;
	
	[self showLoadingIndicator:@"Unlocking.."];
	
	
}

-(IBAction) lockDoor {
	
	NSString * urlString = [NSString stringWithFormat:@"%@/api/set_lock?status=%@&unsecure_lock_id=%@" , self.baseUrl, @"islocked", self.lockId];
	NSURL *url = [NSURL URLWithString:urlString];
	
	lockChangeRequest = [[ASIHTTPRequest alloc] initWithURL:url];
	[lockChangeRequest setRequestMethod:@"POST"];
	[lockChangeRequest setDelegate:self];
	[lockChangeRequest setDidFinishSelector:@selector(stateChangeSuccess:)];
	[lockChangeRequest setDidFailSelector:@selector(stateChangeFailed:)];
	[lockChangeRequest startAsynchronous];
	[lockChangeRequest setTimeOutSeconds:3.0];
	[lockChangeRequest setRetryCount:2];
	[lockChangeRequest setNumberOfTimesToRetryOnTimeout:2];
	
	desiredState = LOCK_STATE_LOCKED;
	
	[self showLoadingIndicator:@"Locking.."];
	
	
}

- (void)stateChangeSuccess:(ASIHTTPRequest *)request
{
	NSString *response = [request responseString];
	NSLog(@"success: %@" , [request responseHeaders] );
	
	NSData *responseData = [request responseData];
	
	
	id JSON = nil;
	NSError *JSONError = nil;
	
	JSON = [[JSONDecoder decoder] objectWithData:responseData error:&JSONError];
	
	
	
	NSNumber * set = [JSON valueForKeyPath:@"lock_set"];
	
	if ( set && [set intValue] == 1 ) {
		
		NSLog(@"set");
		waitingForDesiredState = YES;
		//desiredState = LOCK_STATE_LOCKED;
		lockUpdatedPreRequest = [[JSON valueForKeyPath:@"last_updated_ts"] doubleValue];
		
		// set when we start watching for the desired lock state, then timeout if we dont see it..
		lockChangeRequestStarted = [[NSDate date] timeIntervalSince1970];
		
	} else {
		
		NSLog(@"not set");
		[self showFailedIndicator:@"DB Error"];
		UIAlertView * al = [[UIAlertView alloc] initWithTitle:@"Error" message:@"Lock not set" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
		[al show];
		
	}
	
	[lockChangeRequest release];
	lockChangeRequest = nil;
	
	
}

- (void)stateChangeFailed:(ASIHTTPRequest *)request
{
	NSError *error = [request error];
	NSLog(@"fail: %@ " , error );
	
	UIAlertView * al = [[UIAlertView alloc] initWithTitle:@"Error" message:[NSString stringWithFormat:@"Lock not set %@" , error] delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
	[al show];
	
	[self showFailedIndicator:@"Net Error!"];
	
	[lockChangeRequest release];
	lockChangeRequest = nil;
	
}



-(void) showLoadingIndicator:(NSString*)str {
	
	if ( HUD == nil ) {
		HUD = [[MBProgressHUD alloc] initWithView:self.view];
	}
	
    [self.view addSubview:HUD];
	
	HUD.mode = MBProgressHUDModeIndeterminate;
	
	HUD.dimBackground = YES;
	
    //HUD.delegate = self;
    HUD.labelText = str;
	
    //HUD.detailsLabelText = @"updating data";
	[HUD show:YES];
	
    //[HUD showWhileExecuting:@selector(myTask) onTarget:self withObject:nil animated:YES];
	
}

-(void) showFailedIndicator:(NSString*) str {

	
	HUD.customView = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"hud-failed.png"]] autorelease];
	
    // Set custom view mode
    HUD.mode = MBProgressHUDModeCustomView;
	
	HUD.labelText = str;
	
	[HUD hide:YES afterDelay:2];
	
}

-(void) showCompletedIndicator:(NSString*)str {
	
	
	// The sample image is based on the work by http://www.pixelpressicons.com, http://creativecommons.org/licenses/by/2.5/ca/
	// Make the customViews 37 by 37 pixels for best results (those are the bounds of the build-in progress indicators)
	HUD.customView = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"hud-checkmark.png"]] autorelease];
	
    // Set custom view mode
    HUD.mode = MBProgressHUDModeCustomView;
	
    HUD.labelText = str;
	
	[HUD hide:YES afterDelay:1];
	
	
	
}

-(void) setLockedImage {
	
	if ( lockImageView.image != lockedImage ) {
		lockImageView.image = lockedImage;
	}
	
}

-(void) setUnlockedImage {
	
	if ( lockImageView.image != unlockedImage ) {
		lockImageView.image = unlockedImage;
	}
	
}

-(void) showOverayImage {
	if ( lockOverlayImageView.image != timeoutImage ) {
		lockOverlayImageView.image = timeoutImage;
	}
}

-(void) hideOverlayImage {
	lockOverlayImageView.image = nil;
}




/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/



- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}




- (void)dealloc {
	
	
	[lockedImage release];
	[timeoutImage release];
	[unlockedImage release];
	
    [super dealloc];
}

@end




