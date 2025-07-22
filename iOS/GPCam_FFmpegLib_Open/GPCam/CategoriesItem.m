//
//  Categories.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/14/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "CategoriesItem.h"

@implementation CategoriesItem
//------------------------------------------------------------------------------------------------
-(id)init
{
    if ((self = [super init])) {
        self.Settings = [[NSMutableArray alloc]init];
    }
    return self;
}
//------------------------------------------------------------------------------------------------
@end
