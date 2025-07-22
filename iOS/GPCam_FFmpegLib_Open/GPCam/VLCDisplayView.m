//
//  VLCDisplayView.m
//  GoPlus Cam
//
//  Created by generalplus_sa1 on 11/26/15.
//  Copyright © 2015 generalplus_sa1. All rights reserved.
//

#import "VLCDisplayView.h"

@implementation VLCDisplayView
//---------------------------------------------------------------------------
- (id)initWithCoder:(NSCoder *)aDecoder {
    if ((self = [super initWithCoder:aDecoder])) {
        _PassThroughViews = nil;
    }
    return self;
}
//---------------------------------------------------------------------------
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    UIView *hitView = [super hitTest:point withEvent:event];
    
    for (UIView* item in _PassThroughViews)
    {
        if(item.layer.zPosition == MAXFLOAT)
        {
            UIView *hitViewPass = [item hitTest:[self convertPoint:point toView:item] withEvent:event];
            if(hitViewPass!=nil)
                return hitViewPass;
        }
    }
    
    return hitView;
}
//---------------------------------------------------------------------------
@end
