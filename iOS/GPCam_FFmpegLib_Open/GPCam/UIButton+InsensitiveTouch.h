//
//  UIButton+UIButton_InsensitiveTouch.h
//  GoPlus Cam
//
//  Created by Robin on 2019/11/20.
//  Copyright © 2019 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>
@interface UIButton (InsensitiveTouch)
//开启UIButton防连点模式
+ (void)enableInsensitiveTouch;
//关闭UIButton防连点模式
+ (void)disableInsensitiveTouch;
//设置防连续点击最小时间差(s),不设置则默认值是0.5s
+ (void)setInsensitiveMinTimeInterval:(NSTimeInterval)interval;
@end
