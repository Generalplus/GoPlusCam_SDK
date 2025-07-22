//
//  FFmpegPlayer.h
//  ffmpegTest
//
//  Created by generalplus_sa1 on 1/4/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "FFDecodeFrame.h"


#define FFMPEG_STATUS_PLAYING                   0
#define FFMPEG_STATUS_STOPPED                   1
#define FFMPEG_STATUS_SAVESNAPSHOTCOMPLETE      2
#define FFMPEG_STATUS_SAVEVIDEOCOMPLETE         3
#define FFMPEG_STATUS_BUFFERING                 4

#define DISPLAY_SCALE_FIT                       0
#define DISPLAY_SCALE_FILL                      1
#define DISPLAY_SCALE_STRETCH                   2

#define ENCODE_TYPE_MP4                         0
#define ENCODE_TYPE_AVI                         1

#define EXTRACTOR_OK                            0
#define EXTRACTOR_BUSY                          1
#define EXTRACTOR_READFILEFAILED                2
#define EXTRACTOR_DECODEFAILED                  3
#define EXTRACTOR_NOSUCHFRAME                   4

#define CODEC_ID_NONE                           0
#define CODEC_ID_MJPEG                          8
#define CODEC_ID_H264                           28

#define CUSTOM_PACKET_VIDEO                     0
#define CUSTOM_PACKET_AUDIO                     1

#define CUSTOM_PACKET_TIMESTAMP_AUTO           -1

#define CODEC_NAME_NONE                        @""
#define CODEC_NAME_MJPEG                       @"mjpeg"
#define CODEC_NAME_H264                        @"h264"
//----------------------------------------------------------------
@protocol FFmpegPlayerDelegate
/**
 * \brief
 *     The player status change notification.
 *
 * \param[in] i32Status
 *    The player status.  FFMPEG_STATUS_PLAYING is playing , FFMPEG_STATUS_STOPPED is stopped.
 *                      FFMPEG_STATUS_SAVESNAPSHOTCOMPLETE is saving snapshot complete  , FFMPEG_STATUS_SAVEVIDEOCOMPLETE is ssaving video complete .
 *
 */
-(void) StatusChange:(int)i32Status;

@end

//----------------------------------------------------------------
@interface FFmpegPlayer : NSObject

/**
 * \brief
 *     Play the streaming.
 *
 * \return
 *    Return 0 if this function succeeded. Otherwise, other value returned.
 *
 */
-(int) Play;

/**
 * \brief
 *     Stop the streaming.
 *
 */
-(void) Stop;

/**
 * \brief
 *     Pause the streaming.
 *
 */
-(void) Pause;

/**
 * \brief
 *     Resume the streaming.
 *
 */
-(void) Resume;

/**
 * \brief
 *     Set the streaming path.
 *
 * \param[in] moviePath
 *    The streaming path.
 */
-(void) VideoPath:(NSString *)moviePath;

/**
 *
 * \brief
 *    The video information string.
 */
-(NSString *) GetVideoInfo:(NSString *)moviePath;

/**
 * \brief
 *     Set the video codec and audio codec for decoding custom packet.
 *
 * \param[in] VideoCodecName
 *    The video codec name.
 * \param[in] AudioCodecName
 *    The audio codec name.
 */
-(void) CustomProtocol:(NSString*) VideoCodecName
            AudioCodec:(NSString*) AudioCodecName;
/**
 * \brief
 *     Push the packet data to decode.
 *
 * \param[in] pbyData
 *    The packet data.
 * \param[in] i32Size
 *    The packet size.
 * \param[in] i32Type
 *    The packet type. CUSTOM_PACKET_VIDEO is video packet , CUSTOM_PACKET_AUDIO is audio packet.
 * \param[in] lTimeStamp
 *    The packet timestamp. CUSTOM_PACKET_TIMESTAMP_AUTO is for assign timestamp automatically.
 *
 * \return
 *    Return 0 if this function succeeded. Otherwise, other value returned.
 */
-(int) PushCustomPacket:(const unsigned char*) pbyData
                   Size:(int) i32Size
                   Type:(int) i32Type
              TimeStamp:(int64_t) lTimeStamp;
/**
 * \brief
 *     Set the option for streaming.
 *
 * \details
 *  The option string format is "option1=argument1;option2=argument2;...".
 *  Ex: RTSP streaming over TCP "rtsp_transport=tcp".
 *
 * \param[in] options
 *    The options.
 */
-(void) UserOptions:(NSString *)options;

/**
 * \brief
 *     Set the option for encode streaming.
 *
 * \details
 *  The option string format is "option1=argument1;option2=argument2;...".
 *  Ex: minimum quantizer set to 5 "qmin=5" ,
 *      maximum quantizer set to 32 "qmax=32" ,
 *      average bitrate set to 400000 "b=400000"
 *
 * \param[in] options
 *    The options.
 */
-(void) TransCodeOptions:(NSString *)options;

/**
 * \brief
 *     Set the option for decode streaming.
 *
 * \details
 *  The option string format is "option1=argument1;option2=argument2;...".
 *  Ex: Decoding H264 with low lantency "flags=low_delay".
 *
 * \param[in] options
 *    The options.
 */
-(void) DecodeOptions:(NSString *)options;

/**
 * \brief
 *     Save the streaming snapshot to file.
 *
 * \param[in] Path
 *    The file path.
 */
-(void) Savesnapshot:(NSString *)Path;

/**
 * \brief
 *     Enable force to transcode during saving stream.
 *
 * \details
 *  Saving streaming to file directly now by default. Enable this option
 *  will do transcode frames to H264 before saving to file.
 *  This option is only for mjpeg streaming.
 *  Android is using x264 library(software) to transcode streaming,and iOS is using videotoobox(hardware).
 *
 * \param[in] bEnable
 *    Enable/Disable transcode.
 */
-(void) ForceToTranscode:(bool)bEnable;

/**
 * \brief
 *     Save the streaming to file.
 *
 * \details
 *  The file path will automatically appends the correct file extension for the stream.
 *  Ex: H264 streaming => appends .mp4 , MJPEG streaming => appends .avi
 *
 * \param[in] Path
 *    The file path.
 *
 * \return
 *    Return 0 if this function succeeded. Otherwise, other value returned.
 */
-(int) SaveVideo:(NSString *)Path;

/**
 * \brief
 *     Stop save streaming.
 *
 * \return
 *    The file extension is .mp4 return ENCODE_TYPE_MP4 , The file extension is .avi return ENCODE_TYPE_AVI.
 */
-(int)  StopSaveVideo;

/**
 * \brief
 *     Extract frame from video file and save the frame to file.
 *
 * \param[in] VideoPath
 *    The video path.
 * \param[in] SavePath
 *    The save path.
 * \param[in] frameIdx
 *    The frame index.
 *
 * \return
 *    The extract result.
 *    EXTRACTOR_OK = Extract frame Extract successfully.
 *    EXTRACTOR_BUSY = Extractor is busy.
 *    EXTRACTOR_READFILEFAILED = Failed to read video file.
 *    EXTRACTOR_DECODEFAILED = Failed to decode frame.
 *    EXTRACTOR_NOSUCHFRAME = No such "frameIdx" in video file.
 */
-(int) ExtractFrame:(NSString *) VideoPath
           SavePath:(NSString *) SavePath
         FrameIndex:(int64_t) frameIdx;

/**
 * \brief
 *     Seek the streaming to position.
 *
 * \details
 *  Only for playing local file.
 *
 * \param[in] position
 *    The file position (microsecond).
 */
-(void) Seek:(int64_t) position;

/**
 * \brief
 *     Get the streaming duration.
 *
 * \details
 *  Only for playing local file.
 *
 * \return
 *    The streaming duration (microsecond).
 */
-(int64_t) GetDuration;

/**
 * \brief
 *     Get the current streaming position.
 *
 * \details
 *  Only for playing local file (microsecond).
 *
 * \return
 *    The file position.
 */
-(int64_t) GetPosition;


/**
 * \brief
 *     Get the current streaming codec ID.
 *
 * \return
 *    The codec ID.
 */
-(int) GetStreamCodecID;

/**
 * \brief
 *     Set play the streaming repeatedly.
 *
 * \details
 *  Only for playing local file.
 *
 * \param[in] bRepeat
 *    Enable/Disable repeat.
 */
-(void) SetRepeat:(bool)bRepeat;

/**
 * \brief
 *     Set the streaming is from network or not.
 *
 * \details
 *  Enable streaming mode will playing the streaming in low lantency.
 *
 * \param[in] bEnable
 *    Enable/Disable streaming mode.
 */
-(void) SetStreaming:(bool)bEnable;

/**
 * \brief
 *     Set encoding the stream by using local timestamp which is the time shows on the screen.
 *
 * \details
 *  Enable this will ignoring timestamp from the stream and MJPEG streaming will increase file size.
 *
 * \param[in] bEnable
 *    Enable/Disable encoding the stream by using local timestamp. Default is disable.
 */
-(void) SetEncodeByLocalTime:(bool)bEnable;


/**
 * \brief
 *     Set the display scale mode. Must before setting DrawView.
 *
 * \param[in] i32Mode
 *    The display scale mode.
 *  DISPLAY_SCALE_FIT = Fit the screen
 *  DISPLAY_SCALE_FILL = Fill the screen
 *  DISPLAY_SCALE_STRETCH = Stretch the screen
 */
-(void) SetDisplayScale:(int) i32Mode;

/**
 * \brief
 *     Set reconnect the network stream after disconnect.
 *
 * \param[in] bEnable
 *    Enable/Disable auto-reconnect.
 */
-(void) SetAutoReconnect:(bool) bEnable;

/**
 * \brief
 *     Set display debug message on screen. (Mjpeg only).
 *
 * \param[in] bEnable
 *    Enable/Disable display debug message on screen.
 */
-(void) SetDebugMessage:(bool) bEnable;

/**
 * \brief
 *     Set the format for coverting decode frame.
 *
 * \param[in] i32format
 *    The format ID which been defined in FFDecodeFrame class.
 *    Assign -1 to disable conversion.
 */
-(void) SetCovertDecodeFrameFormat:(int) i32formatID;

/**
 * \brief
 *     Set the display zoom in ratio
 *
 * \param[in] fRatio
 *    The display scale ratio. Value must be greater than 0
 */
-(void) SetZoomInRatio:(float) fRatio;

/**
 * \brief
 *     Set the buffering time when display queue is emtpy during streaming.
 *
 * \param[in] bufferTime
 *    The buffering time in millisecond. Value must be greater than 0
 */
-(void) SetBufferingTime:(int64_t) bufferTime;

/**
 * \brief
 * 	The FFmpegPlayerDelegate.
 */
@property (weak) id<FFmpegPlayerDelegate>   delegate;

/**
 * \brief
 *     Is streaming playing or not.
 */
@property (NS_NONATOMIC_IOSONLY, readonly) BOOL playing;

/**
 * \brief
 *     The view to draw video steaming. Must set SetDisplayScale() first.
 */
@property (nonatomic, strong) UIView    *DrawView;

/**
 * \brief
 *  The view to display the same context of DrawView. Must assign DrawView first.
 */
@property (nonatomic, strong) UIView    *DrawCloneView;

/**
 * \brief
 *     Get the decoded frame.
 */
@property (atomic, getter=getDecodeFrame, readonly) FFDecodeFrame *DecodeFrame;

@end
