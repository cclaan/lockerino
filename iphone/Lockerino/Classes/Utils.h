/*
 *  Utils.h
 *  Lockerino
 *
 *  Created by Chris Laan on 9/26/11.
 *  Copyright 2011 Laan Labs. All rights reserved.
 *
 */



static NSString * PrettyTimeAgo(NSTimeInterval second_diff) {

	//NSMutableString * str = [[[NSMutableString alloc] init] autorelease];
	
	if ( second_diff < 0.0 ) {
		second_diff = 0.0;
		return @"Just Now";
	}
	
	int day_diff = floor(second_diff / (60.0 * 60.0 * 24.0));
	
	if ( day_diff == 0 ) {
		
		if ( second_diff < 10 ) {
			return @"just now";
		} else if ( second_diff < 60 ) {
			return [NSString stringWithFormat:@"%i seconds ago" , (int)second_diff];
		} else if ( second_diff < 120 ) {
			return @"a minute ago";
		} else if ( second_diff < 3600 ) {
			return [NSString stringWithFormat:@"%i minutes ago" , (int)floor(second_diff/60)];
		} else if ( second_diff < 7200 ) {
			return @"an hour ago";
		} else if ( second_diff < 86400 ) {
			return [NSString stringWithFormat:@"%i hours ago" , (int)floor(second_diff/3600)];
		}
		
	} else if ( day_diff == 1 ) {
		return @"Yesterday";
	} else if ( day_diff < 7 ) {
		return [NSString stringWithFormat:@"%i days ago" , (int)(day_diff)];
	} else if ( day_diff < 31 ) {
		return [NSString stringWithFormat:@"%i weeks ago" , (int)(floor(day_diff/7))];
	} else if ( day_diff < 365 ) {
		return [NSString stringWithFormat:@"%i months ago" , (int)(floor(day_diff/30))];
	} else  {
		return [NSString stringWithFormat:@"%i years ago" , (int)(floor(day_diff/365))];
	}
	
	
	/*
	 
	 if day_diff < 0:
	 return ''
	 
	 if day_diff == 0:
	 if second_diff < 10:
	 return "just now"
	 if second_diff < 60:
	 return str(second_diff) + " seconds ago"
	 if second_diff < 120:
	 return  "a minute ago"
	 if second_diff < 3600:
	 return str( second_diff / 60 ) + " minutes ago"
	 if second_diff < 7200:
	 return "an hour ago"
	 if second_diff < 86400:
	 return str( second_diff / 3600 ) + " hours ago"
	 if day_diff == 1:
	 return "Yesterday"
	 if day_diff < 7:
	 return str(day_diff) + " days ago"
	 if day_diff < 31:
	 return str(day_diff/7) + " weeks ago"
	 if day_diff < 365:
	 return str(day_diff/30) + " months ago"
	 return str(day_diff/365) + " years ago"
	 
	 */
	
	
	
	
}

