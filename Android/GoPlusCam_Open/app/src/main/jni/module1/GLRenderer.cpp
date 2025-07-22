//
//  GLRenderer.cpp
//  ffmpegTest
//
//  Created by generalplus_sa1 on 7/11/16.
//  Copyright © 2016 generalplus_sa1. All rights reserved.
//

#include "GLRenderer.h"
#include <algorithm>
#include <libavutil/pixfmt.h>

//----------------------------------------------------------------
enum {
    ATTRIBUTE_VERTEX,
   	ATTRIBUTE_TEXCOORD,
};
//----------------------------------------------------------------
const GLchar vertexShaderString[] =
"attribute vec4 position;"
"attribute vec2 texcoord;"
"uniform mat4 modelViewProjectionMatrix;"
"varying vec2 v_texcoord;"

"void main()"
"{"
"    gl_Position = modelViewProjectionMatrix * position;"
"    v_texcoord = texcoord.xy;"
"}";

const GLchar yuvFragmentShaderString[] =
/*"precision mediump float;"
"varying vec2 v_texcoord;"
"uniform sampler2D s_texture_y;"
"uniform sampler2D s_texture_u;"
"uniform sampler2D s_texture_v;"

"void main()"
"{"
"    float y = texture2D(s_texture_y, v_texcoord).r;"
"    float u = texture2D(s_texture_u, v_texcoord).r - 0.5;"
"    float v = texture2D(s_texture_v, v_texcoord).r - 0.5;"
""
"    float r = y +             1.402 * v;"
"    float g = y - 0.344 * u - 0.714 * v;"
"    float b = y + 1.772 * u;"
"    gl_FragColor = vec4(r,g,b,1.0);"
"}";*/


 "precision mediump float;"
 "varying vec2 v_texcoord;"
 "uniform sampler2D s_texture_y;"
 "uniform sampler2D s_texture_u;"
 "uniform sampler2D s_texture_v;"
 
"void main()"
"{"
"vec3 yuv;"

"yuv.r = texture2D(s_texture_y, v_texcoord).r - 0.0625;"
"yuv.g = texture2D(s_texture_u, v_texcoord).r - 0.5;"
"yuv.b = texture2D(s_texture_v, v_texcoord).r - 0.5;"

"gl_FragColor = clamp(vec4(mat3(1.1643,  1.16430, 1.1643,"
                               "0.0,    -0.39173, 2.0170,"
                               "1.5958, -0.81290, 0.0) * yuv, 1.0), 0.0, 1.0);"
 
"}";


//----------------------------------------------------------------
static bool validateProgram(GLuint prog)
{
    GLint status;
    
    glValidateProgram(prog);
    
#ifdef DEBUG
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        DEBUG_PRINT("Program validate log:%s\n", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        DEBUG_PRINT("Failed to validate program %d\n", prog);
        return false;
    }
    
    return true;
}
//----------------------------------------------------------------
static GLuint compileShader(GLenum type, const GLchar *shaderString)
{
    GLint status;
    const GLchar *sources = shaderString;
    
    GLuint shader = glCreateShader(type);
    if (shader == 0 || shader == GL_INVALID_ENUM) {
        DEBUG_PRINT("Failed to create shader %d\n", type);
        return 0;
    }
    
    glShaderSource(shader, 1, &sources, NULL);
    glCompileShader(shader);
    
#ifdef DEBUG
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        DEBUG_PRINT("Shader compile log:%s\n", log);
        free(log);
    }
#endif
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glDeleteShader(shader);
        DEBUG_PRINT("Failed to compile shader:\n");
        return 0;
    }
    
    return shader;
}
//----------------------------------------------------------------
static void mat4f_LoadOrtho(float left, float right, float bottom, float top, float near, float far, float* mout)
{
    float r_l = right - left;
    float t_b = top - bottom;
    float f_n = far - near;
    float tx = - (right + left) / (right - left);
    float ty = - (top + bottom) / (top - bottom);
    float tz = - (far + near) / (far - near);
    
    mout[0] = 2.0f / r_l;
    mout[1] = 0.0f;
    mout[2] = 0.0f;
    mout[3] = 0.0f;
    
    mout[4] = 0.0f;
    mout[5] = 2.0f / t_b;
    mout[6] = 0.0f;
    mout[7] = 0.0f;
    
    mout[8] = 0.0f;
    mout[9] = 0.0f;
    mout[10] = -2.0f / f_n;
    mout[11] = 0.0f;
    
    mout[12] = tx;
    mout[13] = ty;
    mout[14] = tz;
    mout[15] = 1.0f;
}
//----------------------------------------------------------------
C_GLRenderer::C_GLRenderer():
_sourceHeight(0),
_sourceWidth(0),
_GLViewAgent(NULL),
_bFirstFrame(true),
_FullScreenMode(DISPLAY_SCALE_FIT),
_zoomInRadio(1.0f)
{
    _textures[0] = 0;
    
}

C_GLRenderer::~C_GLRenderer()
{
    if (_textures[0])
        glDeleteTextures(3, _textures);
    
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    if (_renderbuffer) {
        glDeleteRenderbuffers(1, &_renderbuffer);
        _renderbuffer = 0;
    }
    
    if (_program) {
        glDeleteProgram(_program);
        _program = 0;
    }
    
}
GLuint texture = 0;
//----------------------------------------------------------------
bool C_GLRenderer::init()
{
    #if defined(__APPLE__)
    glGenFramebuffers(1, &_framebuffer);
    glGenRenderbuffers(1, &_renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    _GLViewAgent->LoadGLBuffer();
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
    #endif

    GLenum glError = glGetError();
    if (GL_NO_ERROR != glError) {
        
        DEBUG_PRINT("failed to setup GL %x\n", glError);
        return false;
    }
    
    if (!loadShaders())
        return false;
    
    _vertices[0] = -1.0f;  // x0
    _vertices[1] = -1.0f;  // y0
    _vertices[2] =  1.0f;  // ..
    _vertices[3] = -1.0f;
    _vertices[4] = -1.0f;
    _vertices[5] =  1.0f;
    _vertices[6] =  1.0f;  // x3
    _vertices[7] =  1.0f;  // y3

    if (_textures[0]) {
        glDeleteTextures(3, _textures);
        _textures[0] = 0;
    }
    
    DEBUG_PRINT("OK setup GL\n");
    
    return true;
}
//----------------------------------------------------------------
bool C_GLRenderer::isValid()
{
      return (_textures[0] != 0);
}
//----------------------------------------------------------------
void C_GLRenderer::resolveUniforms(GLuint program)
{
    _uniformSamplers[0] = glGetUniformLocation(program, "s_texture_y");
    _uniformSamplers[1] = glGetUniformLocation(program, "s_texture_u");
    _uniformSamplers[2] = glGetUniformLocation(program, "s_texture_v");
}
//----------------------------------------------------------------
void C_GLRenderer::PlatformDisplay(uint8_t *pData[],int i32width,int i32height,int i32format)
{
    static const GLfloat texCoords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    #if defined(__APPLE__)
        glBindFramebuffer(GL_FRAMEBUFFER, 1);
    #else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    #endif

    glViewport(0, 0, _backingWidth, _backingHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_program);
    
    updateVertices();
    render(pData, i32width, i32height,i32format);

    if (prepareRender()) {
        
        GLfloat modelviewProj[16];
        mat4f_LoadOrtho(-1.0f/_zoomInRadio, 1.0f/_zoomInRadio, -1.0f/_zoomInRadio, 1.0f/_zoomInRadio, -1.0f, 1.0f, modelviewProj);
        glUniformMatrix4fv(_uniformMatrix, 1, GL_FALSE, modelviewProj);
        
        glVertexAttribPointer(ATTRIBUTE_VERTEX, 2, GL_FLOAT, 0, 0, _vertices);
        glEnableVertexAttribArray(ATTRIBUTE_VERTEX);
        glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, 0, 0, texCoords);
        glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
}
//----------------------------------------------------------------
void C_GLRenderer::render(uint8_t *pData[],int i32width,int i32height, int i32format)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (0 == _textures[0])
    {
        glGenTextures(3, _textures);
        _bFirstFrame = true;
    }
    
    GLsizei widths[3]  = { i32width, i32width / 2, i32width / 2 };
    GLsizei heights[3] = { i32height, i32height / 2, i32height / 2};
    
    switch(i32format)
    {
        case (int)AV_PIX_FMT_YUV444P:
        case (int)AV_PIX_FMT_YUVJ444P:
            widths[1] = widths[2] = i32width;
        case (int)AV_PIX_FMT_YUV422P:
        case (int)AV_PIX_FMT_YUVJ422P:
            heights[1] = heights[2] = i32height;
            break;
        default:
            break;
    }

    for (int i = 0; i < 3; ++i) {
        
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        
        //if(_bFirstFrame)
        {

            glTexImage2D(GL_TEXTURE_2D,
                            0,
                            GL_LUMINANCE,
                            widths[i],
                            heights[i],
                            0,
                            GL_LUMINANCE,
                            GL_UNSIGNED_BYTE,
                            pData[i]);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        /*else
        {
            glTexSubImage2D( GL_TEXTURE_2D, 0,
                            0, 0,
                            widths[i],
                            heights[i],
                            GL_LUMINANCE, GL_UNSIGNED_BYTE, pData[i]);

        }*/
    }
    
    _bFirstFrame = false;
}
//----------------------------------------------------------------
bool C_GLRenderer::prepareRender()
{
    if (_textures[0] == 0)
        return false;
    
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glUniform1i(_uniformSamplers[i], i);
    }
    
    return true;
}
//----------------------------------------------------------------
void C_GLRenderer::layoutSubviews()
{
#if defined(__APPLE__)
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    _GLViewAgent->LoadGLBuffer();
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        
        DEBUG_PRINT("failed to make complete framebuffer object %x\n", status);
        
    } else {
        
        DEBUG_PRINT("OK setup GL framebuffer %d:%d\n", _backingWidth, _backingHeight);
    }
#endif

    updateVertices();
}
//----------------------------------------------------------------
bool C_GLRenderer::loadShaders()
{
    bool result = false;
    GLuint vertShader = 0, fragShader = 0;
    
    _program = glCreateProgram();
    
    vertShader = compileShader(GL_VERTEX_SHADER, vertexShaderString);
    if (!vertShader)
        goto exit;
    
    fragShader = compileShader(GL_FRAGMENT_SHADER, yuvFragmentShaderString);
    if (!fragShader)
        goto exit;
    
    glAttachShader(_program, vertShader);
    glAttachShader(_program, fragShader);
    glBindAttribLocation(_program, ATTRIBUTE_VERTEX, "position");
    glBindAttribLocation(_program, ATTRIBUTE_TEXCOORD, "texcoord");
    
    glLinkProgram(_program);
    
    GLint status;
    glGetProgramiv(_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        DEBUG_PRINT("Failed to link program %d\n", _program);
        goto exit;
    }
    
    result = validateProgram(_program);
    
    _uniformMatrix = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    resolveUniforms(_program);
    
exit:
    
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    if (result) {
        
        DEBUG_PRINT("OK setup GL programm\n");
        
    } else {
        
        glDeleteProgram(_program);
        _program = 0;
    }
    
    return result;
}
//----------------------------------------------------------------
void C_GLRenderer::updateVertices()
{
    const float width   = _sourceWidth;
    const float height  = _sourceHeight;
    float dH            = (float)_backingHeight / height;
    float dW            = (float)_backingWidth	/ width;
    float dd            = 0;
    
    switch(_FullScreenMode)
    {
        case DISPLAY_SCALE_FILL:
            dd  = std::max(dH, dW);
            dH = dd;
            dW = dd;
            break;
        case DISPLAY_SCALE_STRETCH:
            break;
        default:
            dd  = std::min(dH, dW);
            dH = dd;
            dW = dd;
            break;
    }
    
    const float h       = (height * dH / (float)_backingHeight);
    const float w       = (width  * dW / (float)_backingWidth );
    
    
    _vertices[0] = - w;
    _vertices[1] = - h;
    _vertices[2] =   w;
    _vertices[3] = - h;
    _vertices[4] = - w;
    _vertices[5] =   h;
    _vertices[6] =   w;
    _vertices[7] =   h;
}
//----------------------------------------------------------------


