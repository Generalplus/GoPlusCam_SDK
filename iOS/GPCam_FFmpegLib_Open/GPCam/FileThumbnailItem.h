//
//  FileThumbnailItem.h
//  
//
//  Created by generalplus_sa1 on 9/11/15.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
//------------------------------------------------------------------------------------------------
typedef enum
{
    E_ThumbnailStatus_Loading,
    E_ThumbnailStatus_WaitingCommnad,
    E_ThumbnailStatus_Ready,
    E_ThumbnailStatus_Failed,
    E_ThumbnailStatus_Broken,

}E_ThumbnailStatus;
//------------------------------------------------------------------------------------------------
@interface FileThumbnailItem : NSObject

@property E_ThumbnailStatus Status;
@property int CommandIndex;
@property int Retry;
@property int Index;
@property (nonatomic, retain) NSString *Name;
@property bool bIsLoadImage;
@property bool bIsLoadFileName;

@end
