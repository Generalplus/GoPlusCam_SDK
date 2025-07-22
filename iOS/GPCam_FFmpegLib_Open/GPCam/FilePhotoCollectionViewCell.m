//
//  FilePhotoCollectionViewCell.m
//  GPCam
//
//  Created by generalplus_sa1 on 8/24/15.
//  Copyright (c) 2015 generalplus_sa1. All rights reserved.
//

#import "FilePhotoCollectionViewCell.h"

@interface FilePhotoCollectionViewCell ()
{
    
}
@property(retain, nonatomic) IBOutlet UIImageView *photoImageView;
@end

@implementation FilePhotoCollectionViewCell

//----------------------------------------------------------------------
- (id)initWithCoder:(NSCoder *)aCoder
{
    if(self = [super initWithCoder:aCoder])
    {
        self.backgroundColor = [UIColor colorWithWhite:0.12 alpha:1];
        self.photoImageView.frame = self.bounds;
        self.photoImageView.contentMode = UIViewContentModeScaleAspectFill;
        self.photoImageView.clipsToBounds = YES;
        self.photoImageView.autoresizesSubviews = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
        _fileNameLabel.text = @"";
    }
    return self;
}
//----------------------------------------------------------------------
- (id)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        
        // Grey background
        self.backgroundColor = [UIColor colorWithWhite:0.12 alpha:1];
        // Image
        self.photoImageView = [UIImageView new];
        self.photoImageView.frame = self.bounds;
        self.photoImageView.contentMode = UIViewContentModeScaleAspectFill;
        self.photoImageView.clipsToBounds = YES;
        self.photoImageView.autoresizesSubviews = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
        self.photoImageView.image = [UIImage imageNamed:@"Loading.gif"];
        [self addSubview:self.photoImageView];
        _fileNameLabel.text = @"";
    }
    return self;
}

//----------------------------------------------------------------------
- (void) setImage:(UIImage *)image
{
    _image = image;
    self.photoImageView.image = image;
}
//----------------------------------------------------------------------
@end
