//
//  FilePhotoCollectionViewCell.h
//  GPCam
//
//  Created by generalplus_sa1 on 8/24/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AssetsLibrary/AssetsLibrary.h>


@interface FilePhotoCollectionViewCell : UICollectionViewCell


@property(nonatomic, strong) UIImage *image;
@property(nonatomic) NSInteger      Index;
@property(retain, nonatomic) IBOutlet UILabel     *fileNameLabel;
@end
