//
//  GLRenderer.hpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 7/11/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#ifndef C_GLRenderer_hpp
#define C_GLRenderer_hpp

#include <stdio.h>
#include <stdlib.h>

#if defined(ANDROID) || defined(__ANDROID__)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#else
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#endif

#define DISPLAY_SCALE_FIT        0
#define DISPLAY_SCALE_FILL       1
#define DISPLAY_SCALE_STRETCH    2

#include "Defines.h"
//----------------------------------------------------------------
class C_GLRenderer
{
public:
    C_GLRenderer();
    ~C_GLRenderer();
    
    
    bool init();
    
    bool isValid();
    void resolveUniforms(GLuint program);
    void PlatformDisplay(uint8_t *pData[],int i32width,int i32height, int i32format);
    bool prepareRender();
    
    void layoutSubviews();

    void setViewSize(int width, int height)
    {
        _backingWidth = width;
        _backingHeight = height;
    }

    void setWidth(int i32Width)     { _sourceWidth = i32Width;}
    void setHeight(int i32Height)   { _sourceHeight = i32Height;}
    
    void updateVertices();
    
    void setGLView(I_GLViewAgent* pGLView)  { _GLViewAgent = pGLView;}

    GLuint GetRenderBuffer()                { return _renderbuffer;}
    
    void Reset()                            { _bFirstFrame = true; }
    
    void SetScaleMode(int i32mode)     { _FullScreenMode = i32mode; }

    void SetZoomInRatio(float fRatio)  { if(fRatio > 0.0f) {_zoomInRadio = fRatio;} }
private:
    
    void render(uint8_t *pData[],int i32width,int i32height, int i32format);
    bool loadShaders();
    
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    GLint           _backingWidth;
    GLint           _backingHeight;
    GLuint          _program;
    GLint           _uniformMatrix;
    GLfloat         _vertices[8];
    
    GLint           _uniformSamplers[3];
    GLuint          _textures[3];
    
    int             _sourceWidth;
    int             _sourceHeight;
    
    bool            _bFirstFrame;
    int             _FullScreenMode;
    float           _zoomInRadio;
    I_GLViewAgent        *_GLViewAgent;
    
};


#endif /* C_GLRenderer_hpp */
