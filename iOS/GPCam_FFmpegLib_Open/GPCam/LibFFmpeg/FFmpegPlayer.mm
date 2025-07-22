//
//  FFmpegPlayer.m
//  ffmpegTest
//
//  Created by generalplus_sa1 on 1/4/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#import "FFmpegPlayer.h"
#import "GLView.h"
#include "FFmpegPlayerCore.h"
#include "CoreAudioPlayer.h"
//----------------------------------------------------------------
@interface FFmpegPlayer()

-(void) InitComplete:(int)i32Result;

-(void) PlatformDisplay:(uint8_t *[])pData
                  width:(int)i32width
                 height:(int)i32height
                 format:(int)i32format
              lineSizes:(int[4])linesizes;

-(void)  SaveSanpshotComplete;
-(void)  SaveVideoComplete;
-(void)  StartBuffering;
-(void)  ContinuePlaying;


@end
//----------------------------------------------------------------
class C_FFmpegAgnet :   public I_FFmpegAgnet
{
public:

    C_FFmpegAgnet() {}
    ~C_FFmpegAgnet() {}

    //----------------------------------------------------------------
    // I_FFmpegAgnet
    //----------------------------------------------------------------
    void setPlayer(FFmpegPlayer *pPlayer)
    {
        _FFmpegPlayer = pPlayer;
    }

    virtual int InitComplete(int i32Result)
    {
        [_FFmpegPlayer InitComplete:i32Result];
        return 0;
    }

    virtual int PlatformDisplay(uint8_t *pData[], int width, int height, int format, int lineSizes[4])
    {
        @autoreleasepool {
            [_FFmpegPlayer PlatformDisplay:pData width:width height:height format:format lineSizes:lineSizes];
        }
        return 0;
    }

    virtual int SaveSanpshotComplete()
    {
        [_FFmpegPlayer SaveSanpshotComplete];
        return 0;
    }

    virtual int SaveVideoComplete()
    {
        [_FFmpegPlayer SaveVideoComplete];
        return 0;
    }
    
    virtual int StartBuffering()
    {
        [_FFmpegPlayer StartBuffering];
        return 0;
    }
    
    virtual int ContinuePlaying()
    {
        [_FFmpegPlayer ContinuePlaying];
        return 0;
    }

private:
    FFmpegPlayer *_FFmpegPlayer;
};

//---------------------------------------------------------------------------
@interface FFmpegPlayer()
{
    GLView                      *_movView;
    GLView                      *_movCloneView;

    CADisplayLink               *_displayLink;
    UIInterfaceOrientation      _CurrentOrientation;
    NSString                    *_VideoPath;
    NSString                    *_UserOptions;

    bool                        m_bCustomProtocol;
    char                        m_VideoCodecName[16];
    char                        m_AudioCodecName[16];
    int                         m_i32CusAudioCodec;

    C_FFMpegPlayer              m_player;
    C_CoreAudioPlayer           m_AudioPlayer;
    uint8_t                     *m_pData[8];
    C_FFmpegAgnet               _FFmpegAgnet;
    pthread_mutex_t             m_NewDatalock;
    NSTimeInterval              m_PrevTime;
    NSTimeInterval              m_PrevFrameTime;
    bool                        m_bResignActive;
    bool                        m_bPaused;
    bool                        m_bAutoReconnect;
    int                         m_i32ScaleMode;
    int                         m_i32Format;
    float                       m_fWidth;
    float                       m_fHeight;
    E_PlayerStatus              m_prevStatus;

    struct SwsContext           *m_outsws_ctx;
    int                         m_i32ConvertFormat;
}

@end

//---------------------------------------------------------------------------
@implementation FFmpegPlayer
//---------------------------------------------------------------------------
- (id)init{

    if ((self = [super init]))
    {
        _DrawView = nil;
        _DrawCloneView = nil;
        _movCloneView = nil;
        _displayLink = nil;
        _CurrentOrientation = UIInterfaceOrientationUnknown;
        m_bPaused = false;
        _playing = false;
        _VideoPath = nil;
        _UserOptions = nil;
        m_pData[0] = NULL;
        _FFmpegAgnet.setPlayer(self);
        _delegate = nil;
        m_bResignActive = false;
        m_bAutoReconnect = true;
        m_i32ScaleMode = DISPLAY_SCALE_FIT;
        m_fWidth = 0;
        m_fHeight = 0;
        m_i32Format = AV_PIX_FMT_NONE;
        m_outsws_ctx = 0;
        m_i32ConvertFormat = -1;
        m_prevStatus = E_PlayerStatus_Stoped;

        m_bCustomProtocol = false;

        pthread_mutex_init(&m_NewDatalock, NULL);

        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationWillResignActive:)
                                                     name: UIApplicationWillResignActiveNotification
                                                   object: nil];

        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(applicationDidBecomeActive:)
                                                     name: UIApplicationDidBecomeActiveNotification
                                                   object: nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(deviceOrientationDidChange:)
                                                     name:UIDeviceOrientationDidChangeNotification
                                                   object:nil];
        
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        
    }
    return self;

}
//---------------------------------------------------------------------------
- (void)ReflashStream
{
    if(_playing && !m_bPaused)
    {
        // reflash streaming
        DEBUG_PRINT("Reflash streaming...\n");
        [self Pause];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            [self Resume]; // reflash streaming
        });
    }
}
//---------------------------------------------------------------------------
- (void)applicationWillResignActive:(UIApplication *)application
{
    m_bResignActive = true;

    if(!m_bPaused)
    {
        m_AudioPlayer.Pause();
        m_player.Pause();
    }
}
//---------------------------------------------------------------------------
- (void)applicationDidBecomeActive:(UIApplication *)application
{
    m_bResignActive = false;
    m_PrevFrameTime = [NSDate timeIntervalSinceReferenceDate];

    if(!m_bPaused)
    {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            m_AudioPlayer.Resume();
            m_player.Resume();
        });
    }
}
//---------------------------------------------------------------------------
- (void)deviceOrientationDidChange:(NSNotification *)notification
{
    if(m_pData[0] != NULL && _movView)
    {
        dispatch_async( dispatch_get_main_queue(), ^{
            
            [_movView PlatformDisplay:m_pData width:_movView.sourceWidth height:_movView.sourceHeight format:m_i32Format];
            if(_movCloneView)
                [_movCloneView PlatformDisplay:m_pData width:_movCloneView.sourceWidth height:_movCloneView.sourceHeight format:m_i32Format];
        });
    }
}
//---------------------------------------------------------------------------
-(void) setDrawView:(UIView *)View
{
    if(_movView)
    {
        [_movView removeFromSuperview];
        [_movView freeContext];
        _movView = nil;
    }

    _movView = [[GLView alloc]initWithFrame:View.frame
                                  scaleMode:m_i32ScaleMode];
    _DrawView = View;
}
//---------------------------------------------------------------------------
-(void) setDrawCloneView:(UIView *)View
{
    if(_movCloneView)
    {
        [_movCloneView removeFromSuperview];
        [_movCloneView freeContext];
        _movCloneView = nil;
    }

    _movCloneView = [[GLView alloc]initWithFrame:View.frame
                                       scaleMode:m_i32ScaleMode];
    _DrawCloneView = View;
}
//---------------------------------------------------------------------------
-(void)AddLayoutConstraint:(UIView*)sourceView
                    Target:(UIView*)TargetView
{
    sourceView.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutConstraint *LC = [NSLayoutConstraint
                              constraintWithItem:sourceView attribute:NSLayoutAttributeLeft
                              relatedBy:NSLayoutRelationEqual toItem:TargetView attribute:
                              NSLayoutAttributeLeft multiplier:1.0 constant:0];

    NSLayoutConstraint *RC = [NSLayoutConstraint
                              constraintWithItem:sourceView attribute:NSLayoutAttributeRight
                              relatedBy:NSLayoutRelationEqual toItem:TargetView attribute:
                              NSLayoutAttributeRight multiplier:1.0 constant:0];

    NSLayoutConstraint *TC = [NSLayoutConstraint
                              constraintWithItem:sourceView attribute:NSLayoutAttributeTop
                              relatedBy:NSLayoutRelationEqual toItem:TargetView attribute:
                              NSLayoutAttributeTop multiplier:1.0 constant:0];


    NSLayoutConstraint *BC = [NSLayoutConstraint
                              constraintWithItem:sourceView attribute:NSLayoutAttributeBottom
                              relatedBy:NSLayoutRelationEqual toItem:TargetView attribute:
                              NSLayoutAttributeBottom multiplier:1.0 constant:0];

    [TargetView addConstraints:@[LC,RC,TC,BC]];

}
//---------------------------------------------------------------------------
- (void) drawView: (CADisplayLink*) displayLink
{
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    if(now - m_PrevTime > 1)
    {
        if(_playing)
        {
            if(m_player.GetStatus() != E_PlayerStatus_Playing)
            {
                if(_delegate)
                    [_delegate StatusChange:FFMPEG_STATUS_STOPPED];
                
                if(m_bAutoReconnect)
                {
                    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                        m_player.PlayMedia();
                    });
                }
                else
                {
                    _playing = false;
                }
            }
            
            if(m_player.GetStatus() == E_PlayerStatus_Playing && m_prevStatus != E_PlayerStatus_Playing)
            {
                if(_delegate)
                    [_delegate StatusChange:FFMPEG_STATUS_PLAYING];
            }
            
        }
        m_prevStatus = (E_PlayerStatus)m_player.GetStatus();
        m_PrevTime = now;
    }
}
//---------------------------------------------------------------------------
-(int) Play
{
    [self Stop];

    m_player.setAgent(&_FFmpegAgnet);
    const char *pOptions = nil;
    if(_UserOptions)
        pOptions = [_UserOptions cStringUsingEncoding:NSASCIIStringEncoding];

    int i32Ret = 0;

    if(!m_bCustomProtocol)
        i32Ret = m_player.InitMedia( [_VideoPath cStringUsingEncoding:NSASCIIStringEncoding],pOptions);
    else
        i32Ret = m_player.InitCustomProtocol(m_VideoCodecName,m_AudioCodecName,pOptions);

    return i32Ret;
}
//---------------------------------------------------------------------------
-(void)StartDisplay
{
    if(_displayLink==nil)
    {
        //Only for checking connection. Render() move to main GCD.
        _displayLink = [CADisplayLink displayLinkWithTarget:self
                                                   selector:@selector(drawView:)];

        [_displayLink setFrameInterval:6]; // 10fps
        [_displayLink addToRunLoop:[NSRunLoop currentRunLoop]
                           forMode:NSDefaultRunLoopMode];

        m_PrevTime = [NSDate timeIntervalSinceReferenceDate];
        m_PrevFrameTime = [NSDate timeIntervalSinceReferenceDate];
    }
}
//---------------------------------------------------------------------------
-(void)StopDisplay
{
    if(_displayLink!=nil)
    {
        [_displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        _displayLink = nil;
        m_player.Stop();
        m_AudioPlayer.Stop();
    }
}
//---------------------------------------------------------------------------
-(void) Stop
{
    m_prevStatus = E_PlayerStatus_Stoped;
    _playing = false;
    [_movView removeFromSuperview];
    if(_movCloneView)
        [_movCloneView removeFromSuperview];
    [self StopDisplay];
}
//-------------------------------------------------------------------------
-(void) Pause
{
    m_AudioPlayer.Pause();
    m_player.Pause();
    m_bPaused = true;
}
//-------------------------------------------------------------------------
-(void) Resume
{
    m_AudioPlayer.Resume();
    m_player.Resume();
    m_bPaused = false;
}
//---------------------------------------------------------------------------
-(void)VideoPath:(NSString *)moviePath
{
    _VideoPath = moviePath;
    m_bCustomProtocol = false;
}
//---------------------------------------------------------------------------
-(NSString *) GetVideoInfo:(NSString *)moviePath
{
    const char* info =m_player.GetMediaInfo([moviePath cStringUsingEncoding:NSASCIIStringEncoding]);
    return [NSString stringWithCString:info encoding:NSASCIIStringEncoding];
}
//---------------------------------------------------------------------------
-(void) CustomProtocol:(NSString*) VideoCodecName
            AudioCodec:(NSString*) AudioCodecName
{

    strncpy(m_VideoCodecName, [VideoCodecName cStringUsingEncoding:NSASCIIStringEncoding], 16);
    strncpy(m_AudioCodecName, [AudioCodecName cStringUsingEncoding:NSASCIIStringEncoding], 16);

    m_bCustomProtocol = true;
}
//---------------------------------------------------------------------------
-(int) PushCustomPacket:(const unsigned char*) pbyData
                   Size:(int) i32Size
                   Type:(int) i32Type
              TimeStamp:(int64_t) lTimeStamp
{
    return m_player.PushCustomPacket(pbyData, i32Size, i32Type,lTimeStamp);
}
//---------------------------------------------------------------------------
-(void) UserOptions:(NSString *)options
{
    _UserOptions = options;
}
//---------------------------------------------------------------------------
-(void) TransCodeOptions:(NSString *)options
{
    m_player.SetTransCodeOptions([options cStringUsingEncoding:NSASCIIStringEncoding]);
}
//---------------------------------------------------------------------------
-(void) DecodeOptions:(NSString *)options
{
    m_player.SetDecodeOptions([options cStringUsingEncoding:NSASCIIStringEncoding]);
}
//---------------------------------------------------------------------------
-(void) ForceToTranscode:(bool)bEnable
{
    m_player.ForceToTranscode(bEnable);
}
//---------------------------------------------------------------------------
-(void) Savesnapshot:(NSString *)Path
{
    if(_playing)
        m_player.SaveSnapshot([Path cStringUsingEncoding:NSASCIIStringEncoding]);
}
//---------------------------------------------------------------------------
-(int) SaveVideo:(NSString *)Path
{
    if(_playing)
        return m_player.SaveVideo([Path cStringUsingEncoding:NSASCIIStringEncoding]);
    else
        return FFMPEGPLAYER_SAVEVIDEOFAILED;
}
//---------------------------------------------------------------------------
-(int) StopSaveVideo
{
    return m_player.StopSaveVideo();
}
//---------------------------------------------------------------------------
-(int) ExtractFrame:(NSString *) VideoPath
           SavePath:(NSString *) SavePath
         FrameIndex:(int64_t) frameIdx
{
    return m_player.ExtractFrame(
                                 [VideoPath cStringUsingEncoding:NSASCIIStringEncoding],
                                 [SavePath cStringUsingEncoding:NSASCIIStringEncoding],
                                 frameIdx);
}
//---------------------------------------------------------------------------
-(void) ResetResolution:(int)i32Width
                 height:(int)i32Height
                 format:(int)i32format
              lineSizes:(int[4])linesizes
{
    if(m_pData[0]!=NULL)
    {
        av_freep(&m_pData[0]);
    }

    _movView.sourceWidth = i32Width;
    _movView.sourceHeight = i32Height;
    [_movView Reset];

    if(_movCloneView)
    {
        _movCloneView.sourceWidth = i32Width;
        _movCloneView.sourceHeight = i32Height;
        [_movCloneView Reset];
    }

    av_image_alloc(m_pData, linesizes, _movView.sourceWidth, _movView.sourceWidth ,(AVPixelFormat)i32format, 1);

    m_i32Format = i32format;

    //Set black color
    int i32Div = 4;
    if(i32format == AV_PIX_FMT_YUV422P || i32format == AV_PIX_FMT_YUVJ422P )
        i32Div = 2;
    if(i32format == AV_PIX_FMT_YUV444P || i32format == AV_PIX_FMT_YUVJ444P )
        i32Div = 1;

    memset(m_pData[0],0x00,_movView.sourceWidth*_movView.sourceHeight);
    memset(m_pData[1],0x80,_movView.sourceWidth*_movView.sourceHeight/i32Div);
    memset(m_pData[2],0x80,_movView.sourceWidth*_movView.sourceHeight/i32Div);
    _CurrentOrientation = UIInterfaceOrientationUnknown;

}
//---------------------------------------------------------------------------
-(void) InitComplete:(int) i32Result
{
    if(i32Result != FFMPEGPLAYER_NOERROR)
    {
        if(_delegate)
            [_delegate StatusChange:FFMPEG_STATUS_STOPPED];

        return ;
    }

    int LinSize[4]  = { m_player.GetWidth(), m_player.GetWidth() / 2, m_player.GetWidth() / 2 ,0};
    [self ResetResolution:m_player.GetWidth()
                   height:m_player.GetHeight()
                   format:AV_PIX_FMT_YUV420P
                lineSizes:LinSize];

    m_player.PlayMedia();
    
    dispatch_async(dispatch_get_main_queue(), ^{

        if(!_playing)
        {
            if(_DrawView)
            {
                //clear GLView
                [_movView PlatformDisplay:m_pData width:_movView.sourceWidth height:_movView.sourceHeight format:m_i32Format];
                if(_movCloneView)
                    [_movCloneView PlatformDisplay:m_pData width:_movCloneView.sourceWidth height:_movCloneView.sourceHeight format:m_i32Format];

                [_DrawView insertSubview:_movView atIndex:0];
                [self AddLayoutConstraint:_movView Target:_DrawView];
                if(_DrawCloneView)
                {
                    [_DrawCloneView insertSubview:_movCloneView atIndex:0];
                    [self AddLayoutConstraint:_movCloneView Target:_DrawCloneView];
                }
            }

            _playing = true;
        }

        [self StartDisplay];

        //if(_delegate)
        //    [_delegate StatusChange:FFMPEG_STATUS_PLAYING];
    });

}
//---------------------------------------------------------------------------
-(void) PlatformDisplay:(uint8_t *[])pData
                  width:(int)i32width
                 height:(int)i32height
                 format:(int)i32format
              lineSizes:(int[4])linesizes
{

    m_PrevFrameTime = [NSDate timeIntervalSinceReferenceDate];

    if(i32width!=_movView.sourceWidth || i32height!=_movView.sourceHeight || m_i32Format != i32format)
    {
        [self ResetResolution:i32width
                       height:i32height
                       format:i32format
                    lineSizes:linesizes];
    }

    av_image_copy(m_pData,linesizes,(const uint8_t **)pData,linesizes,(AVPixelFormat)i32format, _movView.sourceWidth, _movView.sourceHeight);

    dispatch_async( dispatch_get_main_queue(), ^{

        [_movView PlatformDisplay:m_pData width:_movView.sourceWidth height:_movView.sourceHeight format:m_i32Format];
        if(_movCloneView)
            [_movCloneView PlatformDisplay:m_pData width:_movCloneView.sourceWidth height:_movCloneView.sourceHeight format:m_i32Format];
    });

}
//---------------------------------------------------------------------------
-(void) SetRepeat:(bool)bRepeat
{
    m_player.setRepeat(bRepeat);
}
//---------------------------------------------------------------------------
-(void)  SetStreaming:(bool)bEnable
{
    m_player.SetLowLatency(bEnable);
}
//---------------------------------------------------------------------------
-(void) SetEncodeByLocalTime:(bool)bEnable
{
    m_player.SetEncodeByLocalTime(bEnable);
}
//---------------------------------------------------------------------------
-(void) SetBufferingTime:(int64_t) bufferTime
{
    m_player.SetBufferingTime(bufferTime);
}
//---------------------------------------------------------------------------
-(void) Seek:(int64_t) position
{
    m_player.Seek(position);
}
//---------------------------------------------------------------------------
-(int64_t) GetDuration
{
    return m_player.GetDuration();
}
//---------------------------------------------------------------------------
-(int64_t) GetPosition
{
    return m_player.GetPosition();
}
//---------------------------------------------------------------------------
-(int) GetStreamCodecID
{
    return m_player.GetCodecID();
}
//---------------------------------------------------------------------------
-(void) SetDisplayScale:(int) i32Mode
{
    m_i32ScaleMode = i32Mode;
}
//---------------------------------------------------------------------------
-(void) SetAutoReconnect:(bool) bEnable
{
    m_bAutoReconnect = bEnable;
}
//---------------------------------------------------------------------------
-(void) SetDebugMessage:(bool) bEnable
{
    m_player.EnableDebugMessage(bEnable);
}
//---------------------------------------------------------------------------
-(void) SaveSanpshotComplete
{
    if(_delegate)
        [_delegate StatusChange:FFMPEG_STATUS_SAVESNAPSHOTCOMPLETE];
}
//---------------------------------------------------------------------------
-(void) SaveVideoComplete
{
    if(_delegate)
        [_delegate StatusChange:FFMPEG_STATUS_SAVEVIDEOCOMPLETE];
}
//---------------------------------------------------------------------------
-(void) StartBuffering
{
    if(_delegate)
        [_delegate StatusChange:FFMPEG_STATUS_BUFFERING];
}
//---------------------------------------------------------------------------
-(void) ContinuePlaying
{
    if(_delegate)
        [_delegate StatusChange:FFMPEG_STATUS_PLAYING];
}
//---------------------------------------------------------------------------
-(void) SetCovertDecodeFrameFormat:(int) i32formatID
{
    m_i32ConvertFormat = i32formatID;
    switch(i32formatID)
    {
        case FFDECODE_FORMAT_YUV420P:
            m_i32ConvertFormat = AV_PIX_FMT_YUV420P;
            break;
        case FFDECODE_FORMAT_YUV422P:
            m_i32ConvertFormat = AV_PIX_FMT_YUV422P;
            break;
        case FFDECODE_FORMAT_YUV444P:
            m_i32ConvertFormat = AV_PIX_FMT_YUV444P;
            break;
        case FFDECODE_FORMAT_YUVJ420P:
            m_i32ConvertFormat = AV_PIX_FMT_YUVJ420P;
            break;
        case FFDECODE_FORMAT_YUVJ422P:
            m_i32ConvertFormat = AV_PIX_FMT_YUVJ422P;
            break;
        case FFDECODE_FORMAT_YUVJ444P:
            m_i32ConvertFormat = AV_PIX_FMT_YUVJ444P;
            break;
        default:
            m_i32ConvertFormat = AV_PIX_FMT_NONE;
            break;
    }

    if(m_outsws_ctx)
    {
        sws_freeContext(m_outsws_ctx);
        m_outsws_ctx = NULL;
    }
}
//---------------------------------------------------------------------------
-(FFDecodeFrame *) getDecodeFrame
{
    FFDecodeFrame *pFrame = [[FFDecodeFrame alloc]init];

    AVFrame *pDupFrame = m_player.DupDecodedFrame();

    pFrame->width = pDupFrame->width;
    pFrame->height = pDupFrame->height;
    int i32TargetFormat = pDupFrame->format;

    if(m_i32ConvertFormat!=-1)
    {
        i32TargetFormat = m_i32ConvertFormat;
        int ret = av_image_alloc(pFrame->data, pFrame->linesize, pDupFrame->width, pDupFrame->height,(AVPixelFormat)i32TargetFormat, 1);
        if(!m_outsws_ctx)
        {
            m_outsws_ctx = sws_getContext(
                                          pDupFrame->width ,
                                          pDupFrame->height,
                                          (AVPixelFormat)pDupFrame->format,
                                          pFrame->width ,
                                          pFrame->height,
                                          (AVPixelFormat)m_i32ConvertFormat,
                                          SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }

        ret = sws_scale(m_outsws_ctx,
                        pDupFrame->data,
                        pDupFrame->linesize,
                        0,
                        pFrame->height,
                        pFrame->data,
                        pFrame->linesize);

    }
    else
    {
        int ret = av_image_alloc(pFrame->data, pFrame->linesize, pDupFrame->width, pDupFrame->height,(AVPixelFormat)i32TargetFormat, 1);
        av_image_copy(pFrame->data,pFrame->linesize,
                      (const uint8_t **)pDupFrame->data,pDupFrame->linesize,
                      (AVPixelFormat)pDupFrame->format,
                      pDupFrame->width,
                      pDupFrame->height);
    }

    switch(i32TargetFormat)
    {
        case AV_PIX_FMT_YUV420P:
            pFrame->format = FFDECODE_FORMAT_YUV420P;
            break;
        case AV_PIX_FMT_YUV422P:
            pFrame->format = FFDECODE_FORMAT_YUV422P;
            break;
        case AV_PIX_FMT_YUV444P:
            pFrame->format = FFDECODE_FORMAT_YUV444P;
            break;
        case AV_PIX_FMT_YUVJ420P:
            pFrame->format = FFDECODE_FORMAT_YUVJ420P;
            break;
        case AV_PIX_FMT_YUVJ422P:
            pFrame->format = FFDECODE_FORMAT_YUVJ422P;
            break;
        case AV_PIX_FMT_YUVJ444P:
            pFrame->format = FFDECODE_FORMAT_YUVJ444P;
            break;
    }

    av_freep(&pDupFrame->data[0]);
    av_frame_unref(pDupFrame);
    av_frame_free(&pDupFrame);

    return pFrame;
}
//---------------------------------------------------------------------------
-(void) SetZoomInRatio:(float) fRatio
{
    if (_movView) {
        [_movView SetZoomInRatio:fRatio];
    }
}
//---------------------------------------------------------------------------

@end
