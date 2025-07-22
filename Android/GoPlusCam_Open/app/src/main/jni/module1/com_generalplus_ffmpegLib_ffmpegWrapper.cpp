
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <pthread.h>

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdbool.h>
#include "com_generalplus_ffmpegLib_ffmpegWrapper.h"
#include <libavcodec/jni.h>
#ifdef __cplusplus
}
#endif


#include "Defines.h"
#include "VideoAgent.h"


//---------------------------------------------------------------------------------
C_VideoAgent        m_VideoPlayer;

static JavaVM *gJavaVM;
static jobject gInterfaceObject;
const char *kInterfacePath = "com/generalplus/ffmpegLib/ffmpegWrapper";

//---------------------------------------------------------------------------------
static void log_callback_null(void *ptr, int level, const char *fmt, va_list vl)
{
    static int print_prefix = 1;
    static int count;
    static char prev[1024];
    char line[1024];
    static int is_atty;

    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);

    strcpy(prev, line);
    //sanitize((uint8_t *)line);

    if (level >= AV_LOG_DEBUG)
    {
        if(strstr(line,"h264_mediacodec")!=NULL)
            DEBUG_PRINT("%s", line);
    }
}
//---------------------------------------------------------------------------------
void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
        jclass cls = env->FindClass(path);
        if (!cls) {
                DEBUG_PRINT("initClassHelper: failed to get %s class reference", path);
                return;
        }
        jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
        if (!constr) {
                DEBUG_PRINT("initClassHelper: failed to get %s constructor", path);
                return;
        }
        jobject obj = env->NewObject(cls, constr);
        if (!obj) {
                DEBUG_PRINT("initClassHelper: failed to create a %s object", path);
                return;
        }
        (*objptr) = env->NewGlobalRef(obj);
}
//---------------------------------------------------------------------------------
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        JNIEnv *env;
        gJavaVM = vm;
        if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
                DEBUG_PRINT("Failed to get the environment using GetEnv()");
                return -1;
        }
        initClassHelper(env, kInterfacePath, &gInterfaceObject);
        av_jni_set_java_vm(vm, NULL);
        //av_log_set_callback(log_callback_null);
        return JNI_VERSION_1_4;
}
//---------------------------------------------------------------------------------
int ffmpegWrapper_StatusChange(int i32Status)
{
        int status;
        JNIEnv *env;
        bool isAttached = false;

        status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
        if (status < 0) {
                status = gJavaVM->AttachCurrentThread(&env, NULL);
                if (status < 0) {
                        DEBUG_PRINT("StatusChange: failed to attach current thread");
                        return 0;
                }
                isAttached = true;
        }

        jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
        if (!interfaceClass) {
                DEBUG_PRINT("StatusChange: failed to get class reference");
                if (isAttached)
                        gJavaVM->DetachCurrentThread();
                return 0;
        }

        jmethodID method = env->GetMethodID(interfaceClass, "StatusChange",
                                            "(I)V");
        if (!method) {
                DEBUG_PRINT("StatusChange: failed to get method ID");
                if (isAttached)
                        gJavaVM->DetachCurrentThread();
                return 0;
        }

        env->CallVoidMethod(gInterfaceObject, method, i32Status);
        env->DeleteLocalRef(interfaceClass);
        if (isAttached)
                gJavaVM->DetachCurrentThread();

        return 0;
}


/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naInitAndPlay
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naInitAndPlay
        (JNIEnv *pEnv, jclass pObj, jstring pFileName , jstring pOptions)
{

        jboolean jBool;
        char *videoFileName= (char *)pEnv->GetStringUTFChars(pFileName, &jBool);
        DEBUG_PRINT("video file name is %s", videoFileName);

        char *Options= (char *)pEnv->GetStringUTFChars(pOptions, &jBool);
        DEBUG_PRINT("Options is %s", Options);

        int i32Ret = m_VideoPlayer.InitMedia(videoFileName,Options);


        return i32Ret;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetVideoInfo
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetVideoInfo
  (JNIEnv *pEnv, jclass pObj, jstring pFileName)
{

        jboolean jBool;
        char *videoFileName= (char *)pEnv->GetStringUTFChars(pFileName, &jBool);
        DEBUG_PRINT("video file name is %s", videoFileName);

        const char* pInfo = m_VideoPlayer.GetMediaInfo(videoFileName);

        jstring result = pEnv->NewStringUTF(pInfo);

        return result;

}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetVideoRes
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetVideoRes
        (JNIEnv *pEnv, jclass pObj)
{
        jintArray lRes;
        lRes = pEnv->NewIntArray(2);
        if (lRes == NULL) {
                DEBUG_PRINT("cannot allocate memory for video size");
                return NULL;
        }
        jint lVideoRes[2];
        lVideoRes[0] = m_VideoPlayer.GetWidth();
        lVideoRes[1] = m_VideoPlayer.GetHeight();
        pEnv->SetIntArrayRegion( lRes, 0, 2, lVideoRes);
        return lRes;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetup
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetup
        (JNIEnv *pEnv, jclass pObj, int pWidth, int pHeight)
{
        int i32Ret = 0;

        m_VideoPlayer.SetViewSize(pWidth,pHeight);

        return i32Ret;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naPlay
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naPlay
        (JNIEnv *pEnv, jclass pObj)
{

        m_VideoPlayer.PlayMedia();

        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naStop
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naStop
        (JNIEnv *pEnv, jclass pObj)
{
        m_VideoPlayer.Stop();
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naPause
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naPause
        (JNIEnv *, jclass)
{
        m_VideoPlayer.Pause();
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naResume
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naResume
        (JNIEnv *, jclass)
{
        m_VideoPlayer.Resume();
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSeek
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSeek
        (JNIEnv *pEnv, jclass pOb, jlong lPos)
{
        m_VideoPlayer.Seek(lPos);
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetDuration
 * Signature: ()L
 */
JNIEXPORT jlong JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetDuration
        (JNIEnv *, jclass)
{
        return  m_VideoPlayer.GetDuration();
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetPosition
 * Signature: ()L
 */
JNIEXPORT jlong JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetPosition
        (JNIEnv *, jclass)
{
        return  m_VideoPlayer.GetPosition();
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetStreaming
 * Signature: ()L
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetStreaming
        (JNIEnv *pEnv, jclass object, jboolean bLow)
{
        m_VideoPlayer.SetLowLatency(bLow);
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetEncodeByLocalTime
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetEncodeByLocalTime
        (JNIEnv *pEnv, jclass object, jboolean bEnable)
{
    m_VideoPlayer.SetEncodeByLocalTime(bEnable);
    return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetDebugMessage
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetDebugMessage
        (JNIEnv *pEnv, jclass object, jboolean bEnable)
{
    m_VideoPlayer.EnableDebugMessage(bEnable);
    return 0;
}


/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetRepeat
 * Signature: ()L
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetRepeat
        (JNIEnv *pEnv, jclass object, jboolean bRepeat)
{
        m_VideoPlayer.SetRepeat(bRepeat);
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naInitDrawFrame
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naInitDrawFrame
        (JNIEnv *, jclass)
{
        m_VideoPlayer.init();
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naDrawFrame
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naDrawFrame
        (JNIEnv *pEnv, jclass pObj)
{
        m_VideoPlayer.DrawFrame();
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naStatus
        (JNIEnv *, jclass)
{
        return m_VideoPlayer.GetStatus();
}


/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetRevSizeCnt
 * Signature: ()L
 */

JNIEXPORT jlong JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetRevSizeCnt
        (JNIEnv *, jclass)
{
        return m_VideoPlayer.GetRevSizeCnt();
}
/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetFrameCnt
 * Signature: ()L
 */

JNIEXPORT jlong JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetFrameCnt
        (JNIEnv *, jclass)
{
        return m_VideoPlayer.GetFrameCnt();
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naGetStreamCodecID
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetStreamCodecID
        (JNIEnv *, jclass)
{
        return m_VideoPlayer.GetCodeID();
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSaveSnapshot
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSaveSnapshot
        (JNIEnv *pEnv, jclass pObj, jstring pInPath)
{
        jboolean jBool;
        char *pPath = (char *)pEnv->GetStringUTFChars(pInPath, &jBool);

        return m_VideoPlayer.SaveSnapshot(pPath);
}


/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetForceToTranscode
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetForceToTranscode
        (JNIEnv *pEnv, jclass object, jboolean bEnable)
{
        m_VideoPlayer.SetForceToTranscode(bEnable);
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSaveVideo
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSaveVideo
        (JNIEnv *pEnv, jclass pObj, jstring pInPath)
{
        jboolean jBool;
        char *pPath = (char *)pEnv->GetStringUTFChars(pInPath, &jBool);

        return m_VideoPlayer.SaveVideo(pPath);
}


/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naStopSaveVideo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naStopSaveVideo
        (JNIEnv *pEnv, jclass pObj)
{
        return m_VideoPlayer.StopSaveVideo();
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naExtractFrame
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naExtractFrame
        (JNIEnv *pEnv, jclass pObj, jstring pVideo,jstring pSave,jlong lIndex)
{
    jboolean jBool;
    char *pVideoPath = (char *)pEnv->GetStringUTFChars(pVideo, &jBool);
    char *pSavePath = (char *)pEnv->GetStringUTFChars(pSave, &jBool);

    return m_VideoPlayer.ExtractFrame(pVideoPath,pSavePath,lIndex);
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetTransCodeOptions
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetTransCodeOptions
        (JNIEnv *pEnv, jclass pObj, jstring pOptions)
{
        jboolean jBool;
        char *pPath = (char *)pEnv->GetStringUTFChars(pOptions, &jBool);

        return m_VideoPlayer.SetTransCodeOptions(pPath);
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetDecodeOptions
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetDecodeOptions
        (JNIEnv *pEnv, jclass pObj, jstring pOptions)
{
        jboolean jBool;
        char *pPath = (char *)pEnv->GetStringUTFChars(pOptions, &jBool);

        return m_VideoPlayer.SetDecodeOptions(pPath);
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetScaleMode
 * Signature: (Ljava/lang/jint;)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetScaleMode
        (JNIEnv *pEnv, jclass pObj,  jint i32Mode)
{
        m_VideoPlayer.SetScaleMode(i32Mode);
        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetBufferingTime
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetBufferingTime
        (JNIEnv *pEnv, jclass pOb, jlong lTime)
{
    m_VideoPlayer.SetBufferingTime(lTime);
    return 0;
}

//---------------------------------------------------------------------------
static struct SwsContext           *m_outsws_ctx = NULL;
static int                         m_i32ConvertFormat = -1;
#define FFDECODE_FORMAT_YUV420P                 0
#define FFDECODE_FORMAT_YUV422P                 1
#define FFDECODE_FORMAT_YUV444P                 2
#define FFDECODE_FORMAT_YUVJ420P                3
#define FFDECODE_FORMAT_YUVJ422P                4
#define FFDECODE_FORMAT_YUVJ444P                5
/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetCovertDecodeFrameFormat
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetCovertDecodeFrameFormat
        (JNIEnv *pEnv, jclass pObj,  jint i32formatID)
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

        return 0;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    GetDecodeFrame
 * Signature: ()Lcom/generalplus/ffmpegLib/ffDecodeFrame;
 */
JNIEXPORT jobject JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naGetDecodeFrame
        (JNIEnv *pEnv , jclass pObj)
{
    AVFrame *pDupFrame = m_VideoPlayer.DupDecodedFrame();

    jmethodID cnstrctr;
    jclass frameClass = pEnv->FindClass("com/generalplus/ffmpegLib/ffDecodeFrame");
    if (frameClass == 0) {
        DEBUG_PRINT("Find ffDecodeFrame Class Failed.\n");
        return NULL;
    }

    cnstrctr = pEnv->GetMethodID( frameClass , "<init>", "([[B[IIII)V");
    if (cnstrctr == 0) {
        DEBUG_PRINT("Find ffDecodeFrame init method Failed.\n");
        return NULL;
    }

    int i32TargetFormat = pDupFrame->format;
    uint8_t     *Covertdata[8];
    int         Covertlinesize[8] = {0,0,0,0,0,0,0,0};
    int         *pLinesize = pDupFrame->linesize;
    uint8_t     *(*pData)[8] = &pDupFrame->data;

    if(m_i32ConvertFormat!=-1) {
        i32TargetFormat = m_i32ConvertFormat;
        int ret = av_image_alloc(Covertdata, Covertlinesize, pDupFrame->width,
                                 pDupFrame->height, (AVPixelFormat) i32TargetFormat, 1);
        pData =  &Covertdata;

        if (!m_outsws_ctx) {
            m_outsws_ctx = sws_getContext(
                    pDupFrame->width,
                    pDupFrame->height,
                    (AVPixelFormat) pDupFrame->format,
                    pDupFrame->width,
                    pDupFrame->height,
                    (AVPixelFormat) m_i32ConvertFormat,
                    SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }

        ret = sws_scale(m_outsws_ctx,
                        pDupFrame->data,
                        pDupFrame->linesize,
                        0,
                        pDupFrame->height,
                        *pData,
                        Covertlinesize);

        pLinesize = Covertlinesize;
    }

    int i32Div = 4;
    if(i32TargetFormat == AV_PIX_FMT_YUV422P || i32TargetFormat == AV_PIX_FMT_YUVJ422P )
        i32Div = 2;
    if(i32TargetFormat == AV_PIX_FMT_YUV444P || i32TargetFormat == AV_PIX_FMT_YUVJ444P )
        i32Div = 1;

    int i32DivArray[8] = {1,i32Div,i32Div,1,1,1,1,1};

    jclass byteArray1DClass = pEnv->FindClass("[B");
    jobjectArray DataArray = pEnv->NewObjectArray(AV_NUM_DATA_POINTERS, byteArray1DClass, NULL);


    for(int i=0;i<AV_NUM_DATA_POINTERS;i++)
    {
        if(pLinesize[i]>0)
        {
            jbyteArray Jdata = (jbyteArray)pEnv->NewByteArray(pDupFrame->width * pDupFrame->height / i32DivArray[i]);
            pEnv->SetByteArrayRegion(Jdata, (jsize)0, (jsize)pDupFrame->width * pDupFrame->height / i32DivArray[i], (jbyte *)(*pData)[i]);
            pEnv->SetObjectArrayElement(DataArray, i, Jdata);
            pEnv->DeleteLocalRef(Jdata);
        }
    }

    if(m_i32ConvertFormat!=-1)
        av_freep(&Covertdata[0]);

    jintArray lineSize =  pEnv->NewIntArray(AV_NUM_DATA_POINTERS);
    pEnv->SetIntArrayRegion(lineSize, (jsize)0, (jsize)AV_NUM_DATA_POINTERS, pLinesize);


    int i32Format = 0;
    switch(i32TargetFormat)
    {
        case AV_PIX_FMT_YUV420P:
            i32Format = FFDECODE_FORMAT_YUV420P;
            break;
        case AV_PIX_FMT_YUV422P:
            i32Format = FFDECODE_FORMAT_YUV422P;
            break;
        case AV_PIX_FMT_YUV444P:
            i32Format = FFDECODE_FORMAT_YUV444P;
            break;
        case AV_PIX_FMT_YUVJ420P:
            i32Format = FFDECODE_FORMAT_YUVJ420P;
            break;
        case AV_PIX_FMT_YUVJ422P:
            i32Format = FFDECODE_FORMAT_YUVJ422P;
            break;
        case AV_PIX_FMT_YUVJ444P:
            i32Format = FFDECODE_FORMAT_YUVJ444P;
            break;
    }

    jobject RetObject = pEnv->NewObject(frameClass,
                                        cnstrctr,
                                        DataArray,
                                        lineSize,
                                        pDupFrame->width,
                                        pDupFrame->height,
                                        i32Format);

    av_freep(&pDupFrame->data[0]);
    av_frame_unref(pDupFrame);
    av_frame_free(&pDupFrame);

    pEnv->DeleteLocalRef(DataArray);
    pEnv->DeleteLocalRef(lineSize);
    pEnv->DeleteLocalRef(frameClass);

    return RetObject;
}

/*
 * Class:     com_generalplus_ffmpegLib_ffmpegWrapper
 * Method:    naSetZoomInRatio
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_generalplus_ffmpegLib_ffmpegWrapper_naSetZoomInRatio
        (JNIEnv *pEnv, jclass pObj,  jfloat fRatio)
{
    m_VideoPlayer.SetZoomInRatio(fRatio);
    return 0;
}