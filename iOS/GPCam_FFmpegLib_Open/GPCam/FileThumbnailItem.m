//
//  FileThumbnailItem.m
//  
//
//  Created by generalplus_sa1 on 9/11/15.
//
//

#import "FileThumbnailItem.h"

@implementation FileThumbnailItem

//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        _Name = nil;
        _Status = E_ThumbnailStatus_Loading;
        _Retry = 5;
        _bIsLoadImage = false;
        _bIsLoadFileName = false;
        
    }
    return self;
}
//------------------------------------------------------------------------------------------------
@end
