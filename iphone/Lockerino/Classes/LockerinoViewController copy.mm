//
//  LockerinoViewController.m
//  Lockerino
//
//  Created by Chris Laan on 9/25/11.
//  Copyright 2011 Laan Labs. All rights reserved.
//

#import "LockerinoViewController.h"
#import "AFJSONRequestOperation.h"
#import "Utils.h"

#include "Cripto.h"


static Cripto cripto;



#define LOCK_CHANGE_REQUEST_TIMEOUT 6


@implementation LockerinoViewController


- (id) awakeFromNib
{
	self = [super init];
	if (self != nil) {
		
		lockState = LOCK_STATE_UNKNOWN;
		lockedImage = [[UIImage imageNamed:@"locked.png"] retain];
		unlockedImage = [[UIImage imageNamed:@"unlocked.png"] retain];
		timeoutImage = [[UIImage imageNamed:@"timeout-overlay.png"] retain];
		
		statusQueue = [[NSOperationQueue alloc] init];
		statusPending = NO;
		
		//[self testCripto2];
		
	}
	return self;
}



static int getValue(uint8_t v) {
	if ( v >= 48 && v <= 57 ) {
		return v-48;
	} else if ( v >= 65 && v <= 70 ) {
		return v-55;
	}
}

-(void) testCripto2 {
	
	int len = 10;
	
	uint32_t k[] = {1,2,0xffffffff,4};
	
	uint32_t v[] = {0,9,8,7,6,0xffffffff,4,3,2,1};
	
	printf("v: %i " , v[0]);
	
	btea(v, 10, k);
	
	printf("v: %u , v:  %u " , v[0],v[9]);
	
	//cripto.setAlgorithm(3);
	
	//cripto.encrypt(v, len_, (uint32_t*)k2);
	
	
}



-(void) testCripto {
	
	// text size must be multiple of BLOCK_SIZE ( 16 ) 
	
	// so decryption must retain those block sizes 
	// you cant hand the decrypter 22 bytes of data
	
	#define len_ 32
	
	unsigned char text_16[65];
	
	//unsigned char text[] = {"hello there"};
	unsigned char text[33] = {
	  "hello there guy john e1234567890"
	// Ê:∞ò !AGêπw,%ä¶ÇáÃpJ1|ﬁÄ°ah bah blha
	};
	//text[31] = '0';
	
	unsigned char text_decrypt[33];
	memset(text_decrypt,0,33);
	
	unsigned char text2[len_];
	
	// Ê:∞ò !AGêπw,%ä¶ÇáÃpJ1|ﬁÄ°∑·Ü¿≠)>[nõ‰âÈΩLZã|Õ
	cripto.setAlgorithm(3);
	
	// 128 bit key ?
	uint32_t k[4];
	k[0] = 1232;
	k[1] = 1232;
	k[2] = 1232;
	k[3] = 1232;
	
	unsigned char k2[17] = {"heythere12345667"};
	//k2[15] = '7';
	printf("%c", k2[15]);
	// Ê:∞ò !AGêπw,%ä¶ÇáÃpJ1|ﬁÄ°ah bah blha
	
	cripto.encrypt(text, len_, (uint32_t*)k2);
	
	//text = "werfwf";
	
	//Serial.println(k[0]);
	//Serial.println((const char*)text);
	//NSLog(@"%s" , text);
	//k[1] = 1132;
	//k[3] = 0233;
	
	int map16[16] = {48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70};
	
	for (int i = 0; i < 32; i++) {
		
		
		int v = text[i];
		
		uint8_t lower = v & 0x0f;
		uint8_t higher = (v & 0xf0) >> 4;
		
		text_16[i*2] = map16[lower];
		text_16[i*2+1] = map16[higher];
		printf("%i, ",v);
		
		//printf("%i , low: %i high: %i \n" , v , lower, higher );
		
	}
	text_16[64] = 0;
	
	NSLog(@"%s" , text_16);
	
	for (int i = 0; i < 32; i++) {
		
		//int v = text_16[i];
		
		uint8_t lower = getValue(text_16[i*2]);
		uint8_t higher = getValue(text_16[i*2+1]);
		
		int v = (higher<<4) | lower;
		
		text_decrypt[i] = v;
		
		//printf("%i , low: %i high: %i \n" , v , lower, higher );
		
	}
	
	//return;
		
	//unsigned char coder[32] = {62, 86, 235, 44,57,130, 138,104, 6, 231,162,16, 45, 93, 147,228,150,212,171,69,45,69,254,200,113,49,212,43,135,229,17,178};
	//unsigned char coder[32] =   {'>','V','Î',',','9','Ç','ä','h','','Á','¢','','-',']','ì','‰','ñ','‘','´','E','-','E','˛','»','q','1','‘','+','á','Â'};
	//NSLog(@"\n\n%s" , coder);
	
	cripto.decrypt(text_decrypt, len_, (uint32_t*)k2);
	
	//Serial.println(k[0]);
	//Serial.println((const char*)text);
	NSLog(@"%s" , text_decrypt);
	
	
	//for (int i = 0; i < 43; i++) {
	//	printf("%i \n" , text[i] );
	//}
	
	
}


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	
	
	
	
	
}

-(void) viewWillAppear:(BOOL)animated {
	
	
	NSLog(@"appear");
	[self resumeStatusTimer];
	
}

-(void) viewWillDisappear:(BOOL)animated {
	
	[self pauseStatusTimer];
	NSLog(@"DISappear");
	
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
	
	statusTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0) target:self selector:@selector(getStatus) userInfo:nil repeats:YES];
	[self getStatus];
	
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

-(void) getStatus {
	
	[self updateStatusLabel];
	
	
	//if ( statusPending ) return;
	if ( [[statusQueue operations] count] > 0 ) {
		NSLog(@"too many ops");
		return;
	}
	
	NSMutableURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://lockitron.appspot.com/lock_status?observed=yes"]];
	//[request setHTTPMethod:@"POST"];
	
	statusPending = YES;
	
	AFJSONRequestOperation *operation = [AFJSONRequestOperation operationWithRequest:request success:^(id JSON) {
	
		statusPending = NO;
		
		
		NSString * _lockStatus = [JSON valueForKeyPath:@"observed_status"];
		//NSLog(@"status: %@ " , _lockStatus );
		
		
		if ( _lockStatus && _lockStatus != [NSNull null] ) {
			
			NSTimeInterval tsNow = [[NSDate date] timeIntervalSince1970];
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
			
			if ( waitingForDesiredState && (lockState == desiredState) ) {
				
				

				if ( tsUpdated != lockUpdatedPreRequest ) {
					desiredState = LOCK_STATE_UNKNOWN;
					waitingForDesiredState = NO;
					[self showCompletedIndicator];
				} else if ( (tsNow - lockChangeRequestStarted) > LOCK_CHANGE_REQUEST_TIMEOUT ) {
					[self showFailedIndicator];
					
				}
				
			} else if ( waitingForDesiredState ) {
			
				if ( (tsNow - lockChangeRequestStarted) > LOCK_CHANGE_REQUEST_TIMEOUT ) {
					desiredState = LOCK_STATE_UNKNOWN;
					waitingForDesiredState = NO;
					[self showFailedIndicator];
				}
				
			}
		}
		
	} failure:^(NSError * err) {
		NSLog(@"err!: %@ " , err );
		statusPending = NO;
		
	}
	];
	
	
	[statusQueue addOperation:operation];
	
}

-(IBAction) lockDoor {
	
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:@"http://lockitron.appspot.com/set_lock?status=islocked"]];
	// status=islocked&rand=1234&sig=w2jerifiwerfiwemggergweg
	[request setHTTPMethod:@"POST"];
	
	
	
	AFJSONRequestOperation *operation = [AFJSONRequestOperation operationWithRequest:request success:^(id JSON) {
		
		NSLog(@"Recieved lock: %@ %@ %@",JSON, [JSON valueForKeyPath:@"observed_status"], [JSON valueForKeyPath:@"last_updated"]);
		//infoLabel.text = [NSString stringWithFormat:@"Status: %@\nLast Update: %@ " , [JSON valueForKeyPath:@"observed_status"],[JSON valueForKeyPath:@"last_updated"]];
		
		
		NSNumber * set = [JSON valueForKeyPath:@"lock_set"];
		if ( set && [set intValue] == 1 ) {
			
			NSLog(@"set");
			waitingForDesiredState = YES;
			desiredState = LOCK_STATE_LOCKED;
			lockUpdatedPreRequest = [[JSON valueForKeyPath:@"last_updated_ts"] doubleValue];
			
		} else {
			NSLog(@"not set");
			[HUD hide:YES];
			UIAlertView * al = [[UIAlertView alloc] initWithTitle:@"Error" message:@"Lock not set" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
			[al show];
			
		}
		
		
		
	} failure:^(NSError * err) {
		NSLog(@"err!: %@ " , err );
		
	}
										 ];
	
	NSOperationQueue *queue = [[[NSOperationQueue alloc] init] autorelease];
	[queue addOperation:operation];
	
	
	lockChangeRequestStarted = [[NSDate date] timeIntervalSince1970];
	
	[self showLoadingIndicator];
	
	
}

-(IBAction) unlockDoor {
	
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:@"http://lockitron.appspot.com/set_lock?status=unlocked"]];
	[request setHTTPMethod:@"POST"];
	
	
	AFJSONRequestOperation *operation = [AFJSONRequestOperation operationWithRequest:request success:^(id JSON) {
		
		NSLog(@"Recieved Unlock: %@ -- %@ %@", JSON, [JSON valueForKeyPath:@"observed_status"], [JSON valueForKeyPath:@"last_updated"]);
		
		NSNumber * set = [JSON valueForKeyPath:@"lock_set"];
		
		if ( set && [set intValue] == 1 ) {

			NSLog(@"set");
			waitingForDesiredState = YES;
			desiredState = LOCK_STATE_UNLOCKED;
			lockUpdatedPreRequest = [[JSON valueForKeyPath:@"last_updated_ts"] doubleValue];
			
		} else {
			NSLog(@"not set");
			[HUD hide:YES];
			UIAlertView * al = [[UIAlertView alloc] initWithTitle:@"Error" message:@"Lock not set" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
			[al show];
		}
		
		//infoLabel.text = [NSString stringWithFormat:@"Status: %@\nLast Update: %@ " , [JSON valueForKeyPath:@"observed_status"],[JSON valueForKeyPath:@"last_updated"]];
				
		
	} failure:^(NSError * err) {
		NSLog(@"err!: %@ " , err );
		
	}
										 ];
	
	NSOperationQueue *queue = [[[NSOperationQueue alloc] init] autorelease];
	[queue addOperation:operation];

	
	lockChangeRequestStarted = [[NSDate date] timeIntervalSince1970];
	[self showLoadingIndicator];
	
}

-(void) showLoadingIndicator {
	
	if ( HUD == nil ) {
		HUD = [[MBProgressHUD alloc] initWithView:self.view];
	}
	
    [self.view addSubview:HUD];
	
	HUD.mode = MBProgressHUDModeIndeterminate;
	
	HUD.dimBackground = YES;
	
    //HUD.delegate = self;
    HUD.labelText = @"Locking";
    //HUD.detailsLabelText = @"updating data";
	[HUD show:YES];
	
    //[HUD showWhileExecuting:@selector(myTask) onTarget:self withObject:nil animated:YES];
	
}

-(void) showFailedIndicator {

	
	HUD.customView = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"hud-failed.png"]] autorelease];
	
    // Set custom view mode
    HUD.mode = MBProgressHUDModeCustomView;
	
	HUD.labelText = @"Failed!";
	
    //[HUD show:YES];
	[HUD hide:YES afterDelay:2];
	
}

-(void) showCompletedIndicator {
	
	//HUD = [[MBProgressHUD alloc] initWithView:self.navigationController.view];
	//[self.navigationController.view addSubview:HUD];
	
	// The sample image is based on the work by http://www.pixelpressicons.com, http://creativecommons.org/licenses/by/2.5/ca/
	// Make the customViews 37 by 37 pixels for best results (those are the bounds of the build-in progress indicators)
	HUD.customView = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"hud-checkmark.png"]] autorelease];
	
    // Set custom view mode
    HUD.mode = MBProgressHUDModeCustomView;
	
    //HUD.delegate = self;
    HUD.labelText = @"Completed";
	
    //[HUD show:YES];
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
    [super dealloc];
}

@end
