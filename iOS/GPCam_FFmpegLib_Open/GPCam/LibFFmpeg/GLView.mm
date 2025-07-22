//
//  GLView.m
//
//
//  Created by generalplus_sa1 on 7/11/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#import "GLView.h"
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "GLRenderer.h"

//----------------------------------------------------------------
class C_GLViewAgent : public I_GLViewAgent
{
public:
    
    C_GLViewAgent() {}
    ~C_GLViewAgent() {}
    
    void setGLView(GLView *pGLView)
    {
        _GLView = pGLView;
    }
    
    virtual void LoadGLBuffer()
    {
        [_GLView LoadBufferStorage];
    }
    
private:
    GLView *_GLView;
};

//----------------------------------------------------------------
@implementation GLView {
    
    EAGLContext     *_context;
    C_GLRenderer    *_render;
    C_GLViewAgent   *_GLAgnet;
}
//----------------------------------------------------------------
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
//----------------------------------------------------------------
- (id) initWithFrame:(CGRect)frame
           scaleMode:(int) i32mode
{
    self = [super initWithFrame:frame];
    if (self) {
        
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*) self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        
        if (!_context ||
            ![EAGLContext setCurrentContext:_context]) {
            
            NSLog(@"failed to setup EAGLContext");
            self = nil;
            return nil;
        }

        _render = new C_GLRenderer();
        _GLAgnet = new C_GLViewAgent();
        
        _GLAgnet->setGLView(self);
        _render->setGLView(_GLAgnet);
        _render->SetScaleMode(i32mode);
        if(!_render->init())
            return nil;

    }
    
    return self;
}
//----------------------------------------------------------------
- (void)freeContext
{
    if ([EAGLContext currentContext] == _context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    _context = nil;
    delete _render;
    delete _GLAgnet;
}
//----------------------------------------------------------------
-(void) LoadBufferStorage
{
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
}
//----------------------------------------------------------------
-(void)setSourceWidth:(int) i32Width
{
    _sourceWidth = i32Width;
    _render->setWidth(i32Width);
}
//----------------------------------------------------------------
-(void)setSourceHeight:(int) i32Height
{
    _sourceHeight = i32Height;
    _render->setHeight(i32Height);
}
//----------------------------------------------------------------
- (void)layoutSubviews
{
    _render->layoutSubviews();
}
//----------------------------------------------------------------
- (void) didMoveToWindow
{
    if (self.window)
        self.contentScaleFactor = self.window.screen.scale;
}
//----------------------------------------------------------------
- (void)setContentMode:(UIViewContentMode)contentMode
{
    [super setContentMode:contentMode];
    _render->updateVertices();
}
//----------------------------------------------------------------
-(void) Reset
{
    _render->Reset();
}
//----------------------------------------------------------------
-(void) PlatformDisplay:(uint8_t *[])pData
                  width:(int)i32width
                 height:(int)i32height
                 format:(int)i32format
{

    [EAGLContext setCurrentContext:_context];
    _render->PlatformDisplay(pData, i32width, i32height,i32format);
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}
//----------------------------------------------------------------
-(void) SetZoomInRatio:(float)fRatio
{
    _render->SetZoomInRatio(fRatio);
}
//----------------------------------------------------------------
@end
