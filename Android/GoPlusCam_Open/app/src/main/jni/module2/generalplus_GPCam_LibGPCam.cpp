#include <stdbool.h>
#include "generalplus_GPCam_LibGPCam.h"
#include "GPCamCommandAPI.h"
#include <android/log.h>
#include <cstdlib>
//---------------------------------------------------------------------------------
#include "QueueTemplate.h"

typedef struct StatusCallBackInfo {
	int i32CMDIndex;
	int i32Type;
	int i32Mode;
	int i32CMDID;
	int i32DataSize;BYTE* pbyData;

} S_StatusCallBackInfo;

typedef struct DataCallBackInfo {
	bool bIsWrite;
	int i32DataSize;BYTE* pbyData;

} S_DataCallBackInfo;

class C_StatusCallBackQueue: public T_Queue<S_StatusCallBackInfo> {
public:
	C_StatusCallBackQueue() {
	}
	~C_StatusCallBackQueue() {
	}

	virtual void FreeObjectContent(S_StatusCallBackInfo* pObject) {
		if (pObject->pbyData) {
			free(pObject->pbyData);
			pObject->pbyData = NULL;
		}
	}
};

class C_DataCallBackQueue: public T_Queue<S_DataCallBackInfo> {
public:
	C_DataCallBackQueue() {
	}
	~C_DataCallBackQueue() {
	}

	virtual void FreeObjectContent(S_DataCallBackInfo* pObject) {
		if (pObject->pbyData) {
			free(pObject->pbyData);
			pObject->pbyData = NULL;
		}
	}
};

typedef struct ThreadQueueInfo {
	bool bStatusCallStart;
	C_StatusCallBackQueue StatusCallBackQueue;
	pthread_t StatusCallBackThreadID;

	bool bDataCallBackStart;
	C_DataCallBackQueue DataCallBackQueue;
	pthread_t DataCallBackThreadID;

} S_ThreadQueueInfo;

S_ThreadQueueInfo g_ThreadQueueInfo;
//---------------------------------------------------------------------------------

JNIEnv *g_env = 0;
jclass *g_class = 0;

static JavaVM *gJavaVM;
static jobject gInterfaceObject;
const char *kInterfacePath = "generalplus/com/GPCamLib/CamWrapper";

#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR  , "GPCamLib",__VA_ARGS__)
int GPCamStatusCallBack(int i32CMDIndex, int i32Type, int i32Mode, int i32CMDID,
		int i32DataSize, BYTE* pbyData);
int GPCamDataCallBack(bool bIsWrite, int i32DataSize, BYTE* pbyData);

void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
	jclass cls = env->FindClass(path);
	if (!cls) {
		LOGD("initClassHelper: failed to get %s class reference", path);
		return;
	}
	jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
	if (!constr) {
		LOGD("initClassHelper: failed to get %s constructor", path);
		return;
	}
	jobject obj = env->NewObject(cls, constr);
	if (!obj) {
		LOGD("initClassHelper: failed to create a %s object", path);
		return;
	}
	(*objptr) = env->NewGlobalRef(obj);
}
//---------------------------------------------------------------------------------
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv *env;
	gJavaVM = vm;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGD("Failed to get the environment using GetEnv()");
		return -1;
	}
	initClassHelper(env, kInterfacePath, &gInterfaceObject);

	g_ThreadQueueInfo.bStatusCallStart = false;
	g_ThreadQueueInfo.StatusCallBackThreadID = 0;

	g_ThreadQueueInfo.bDataCallBackStart = false;
	g_ThreadQueueInfo.DataCallBackThreadID = 0;

	return JNI_VERSION_1_4;
}
//---------------------------------------------------------------------------------
int GPCamStatusCallBack(int i32CMDIndex, int i32Type, int i32Mode, int i32CMDID,
		int i32DataSize, BYTE* pbyData) {

	S_StatusCallBackInfo *pStatus = new S_StatusCallBackInfo;

	pStatus->i32CMDIndex = i32CMDIndex;
	pStatus->i32Type = i32Type;
	pStatus->i32Mode = i32Mode;
	pStatus->i32CMDID = i32CMDID;
	pStatus->i32DataSize = i32DataSize;

	pStatus->pbyData = new BYTE[i32DataSize];
	memcpy(pStatus->pbyData, pbyData, sizeof(BYTE) * i32DataSize);

	g_ThreadQueueInfo.StatusCallBackQueue.PushOject(pStatus);

	return 0;
}
//---------------------------------------------------------------------------------
int GPCamDataCallBack(bool bIsWrite, int i32DataSize, BYTE* pbyData) {

	S_DataCallBackInfo *pData = new S_DataCallBackInfo;

	pData->bIsWrite = bIsWrite;
	pData->i32DataSize = i32DataSize;

	pData->pbyData = new BYTE[i32DataSize];
	memcpy(pData->pbyData, pbyData, sizeof(BYTE) * i32DataSize);

	g_ThreadQueueInfo.DataCallBackQueue.PushOject(pData);

	return 0;
}
//---------------------------------------------------------------------------------
void* StatusThread(void *param) {
	int status;
	JNIEnv *env;
	bool isAttached = false;

	status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
	if (status < 0) {
		status = gJavaVM->AttachCurrentThread(&env, NULL);
		if (status < 0) {
			LOGD("GPCamStatusCallBack: failed to attach current thread");
			return 0;
		}
		isAttached = true;
	}

	jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
	if (!interfaceClass) {
		LOGD("GPCamStatusCallBack: failed to get class reference");
		if (isAttached)
			gJavaVM->DetachCurrentThread();
		return 0;
	}

	jmethodID method = env->GetMethodID(interfaceClass, "GPCamStatusCallBack",
			"(IIIII[B)V");
	if (!method) {
		LOGD("GPCamStatusCallBack: failed to get method ID");
		if (isAttached)
			gJavaVM->DetachCurrentThread();
		return 0;
	}

	g_ThreadQueueInfo.bStatusCallStart = true;
	while (g_ThreadQueueInfo.bStatusCallStart) {
		S_StatusCallBackInfo *pStatus =
				g_ThreadQueueInfo.StatusCallBackQueue.PopObject(true);
		if (pStatus == NULL)
			continue;

		jbyteArray jData = env->NewByteArray(pStatus->i32DataSize);
		env->SetByteArrayRegion(jData, 0, pStatus->i32DataSize,
				(jbyte*) pStatus->pbyData);

		env->CallVoidMethod(gInterfaceObject, method, pStatus->i32CMDIndex,
				pStatus->i32Type, pStatus->i32Mode, pStatus->i32CMDID,
				pStatus->i32DataSize, jData);

		env->DeleteLocalRef(jData);

		free(pStatus->pbyData);
		delete pStatus;

	}

	env->DeleteLocalRef(interfaceClass);
	if (isAttached)
		gJavaVM->DetachCurrentThread();

	g_ThreadQueueInfo.StatusCallBackThreadID = 0;
	g_ThreadQueueInfo.StatusCallBackQueue.ClearQueue();

	return 0;
}
//---------------------------------------------------------------------------------
void* DataThread(void *param) {

	int status;
	JNIEnv *env;
	bool isAttached = false;

	status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
	if (status < 0) {
		status = gJavaVM->AttachCurrentThread(&env, NULL);
		if (status < 0) {
			LOGD("GPCamDataCallBack: failed to attach current thread");
			return 0;
		}
		isAttached = true;
	}

	jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
	if (!interfaceClass) {
		LOGD("GPCamDataCallBack: failed to get class reference");
		if (isAttached)
			gJavaVM->DetachCurrentThread();
		return 0;
	}

	jmethodID method = env->GetMethodID(interfaceClass, "GPCamDataCallBack",
			"(ZI[B)V");
	if (!method) {
		LOGD("GPCamDataCallBack: failed to get method ID");
		if (isAttached)
			gJavaVM->DetachCurrentThread();
		return 0;
	}

	g_ThreadQueueInfo.bDataCallBackStart = true;
	while (g_ThreadQueueInfo.bDataCallBackStart) {
		S_DataCallBackInfo *pData =
				g_ThreadQueueInfo.DataCallBackQueue.PopObject(true);
		if (pData == NULL)
			continue;

		jbyteArray jData = env->NewByteArray(pData->i32DataSize);
		env->SetByteArrayRegion(jData, 0, pData->i32DataSize,
				(jbyte*) pData->pbyData);

		env->CallVoidMethod(gInterfaceObject, method, pData->bIsWrite,
				pData->i32DataSize, jData);

		env->DeleteLocalRef(jData);

		free(pData->pbyData);
		delete pData;

	}

	env->DeleteLocalRef(interfaceClass);
	if (isAttached)
		gJavaVM->DetachCurrentThread();

	g_ThreadQueueInfo.DataCallBackThreadID = 0;
	g_ThreadQueueInfo.DataCallBackQueue.ClearQueue();

	return 0;
}
//---------------------------------------------------------------------------------
void StopThreads() {
	if (g_ThreadQueueInfo.StatusCallBackThreadID != 0) {
		g_ThreadQueueInfo.bStatusCallStart = false;
		g_ThreadQueueInfo.StatusCallBackQueue.UnLockQueue();
		void *ret = NULL;
		pthread_join(g_ThreadQueueInfo.StatusCallBackThreadID, &ret);
	}

	if (g_ThreadQueueInfo.DataCallBackThreadID != 0) {
		g_ThreadQueueInfo.bDataCallBackStart = false;
		g_ThreadQueueInfo.DataCallBackQueue.UnLockQueue();
		void *ret = NULL;
		pthread_join(g_ThreadQueueInfo.DataCallBackThreadID, &ret);
	}
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamConnectToDevice(
		JNIEnv *env, jclass obj, jstring strIPAddress, jint i32PortNum) {

	StopThreads();
	pthread_create(&g_ThreadQueueInfo.StatusCallBackThreadID, NULL,
			StatusThread, NULL);
	pthread_create(&g_ThreadQueueInfo.DataCallBackThreadID, NULL, DataThread,
			NULL);

	//LOGD("Java_generalplus_com_GPCamLib_CamWrapper_GPCamConnectToDevice ... ");
	int Ret = 0;
	jboolean jBool;
	const char *ipbuffer_buf = env->GetStringUTFChars(strIPAddress, &jBool);
	GPCam_SetCmdStatusCallBack(&GPCamStatusCallBack);
	GPCam_SetDataCallBack(&GPCamDataCallBack);
	Ret = GPCam_ConnectToDevice((LPCTSTR) ipbuffer_buf, i32PortNum);
	env->ReleaseStringUTFChars(strIPAddress, ipbuffer_buf);
	return Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamDisconnect
(JNIEnv *env, jclass obj)
{
	StopThreads();
	//LOGD("Java_generalplus_com_GPCamLib_CamWrapper_GPCamDisconnect ...");
	GPCam_Disconnect();
}
//---------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSetDownloadPath
(JNIEnv *env, jclass obj, jstring strPath)
{
	jboolean jBool;
	const char *path_buf = env->GetStringUTFChars(strPath, &jBool);
	GPCam_SetDownloadPath((LPCTSTR)path_buf);
	env->ReleaseStringUTFChars(strPath, path_buf);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamAbort(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_Abort(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendSetMode(
		JNIEnv *env, jclass obj, jint i32Mode) {
	//LOGD("Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendSetMode ... ");
	return GPCam_SendSetMode(i32Mode);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetSetPIP(
		JNIEnv *env, jclass obj, jint i32Type) {
	//LOGD("Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetSetPIP ... ");
	return GPCam_SendGetSetPIP(i32Type);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetStatus(
		JNIEnv *env, jclass obj) {
	//LOGD("Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetStatus ... ");
	return GPCam_SendGetStatus();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetParameterFile(
		JNIEnv *env, jclass obj, jstring strPath) {
	int Ret = 0;
	jboolean jBool;
	const char *path_buf = env->GetStringUTFChars(strPath, &jBool);
	Ret = GPCam_SendGetParameterFile((LPCTSTR) path_buf);
	env->ReleaseStringUTFChars(strPath, path_buf);
	return Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendPowerOff(
		JNIEnv *env, jclass obj) {
	return GPCam_SendPowerOff();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendRestartStreaming(
		JNIEnv *env, jclass obj) {
	return GPCam_SendRestartStreaming();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendRecordCmd(
		JNIEnv *env, jclass obj) {
	return GPCam_SendRecordCmd();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendAudioOnOff(
		JNIEnv *env, jclass obj, jboolean bIsOn) {
	return GPCam_SendAudioOnOff(bIsOn);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendCapturePicture(
		JNIEnv *env, jclass obj) {
	return GPCam_SendCapturePicture();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendStartPlayback(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_SendStartPlayback(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendPausePlayback(
		JNIEnv *env, jclass obj) {
	return GPCam_SendPausePlayback();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetFullFileList(
		JNIEnv *env, jclass obj) {
	return GPCam_SendGetFullFileList();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetFileThumbnail(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_SendGetFileThumbnail(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetFileRawdata(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_SendGetFileRawdata(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendStopPlayback(
		JNIEnv *env, jclass obj) {
	return GPCam_SendStopPlayback();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSetNextPlaybackFileListIndex(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_SetNextPlaybackFileListIndex(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendDeleteFile(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_SendDeleteFile(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetParameter(
		JNIEnv *env, jclass obj, jint i32ID) {
	return GPCam_SendGetParameter(i32ID);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendSetParameter(
		JNIEnv *env, jclass obj, jint i32ID, jint i32Size, jbyteArray pbyData) {
	unsigned char *Data_buf;

	jbyte *arr = env->GetByteArrayElements(pbyData, 0);
	Data_buf = (unsigned char*) arr;
	jint i32Ret = GPCam_SendSetParameter(i32ID, i32Size, Data_buf);
	env->ReleaseByteArrayElements(pbyData, arr, 0);
	return i32Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendFirmwareDownload(
		JNIEnv *env, jclass obj, jlong ui32FileSize, jlong ui32CheckSum) {
	return GPCam_SendFirmwareDownload(ui32FileSize, ui32CheckSum);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendFirmwareRawData(
		JNIEnv *env, jclass obj, jlong ui32Size, jbyteArray pbyData) {
	unsigned char *Data_buf;

	jbyte *arr = env->GetByteArrayElements(pbyData, 0);
	Data_buf = (unsigned char*) arr;
	jint i32Ret = GPCam_SendFirmwareRawData(ui32Size, Data_buf);
	env->ReleaseByteArrayElements(pbyData, arr, 0);
	return i32Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendFirmwareUpgrade(
		JNIEnv *env, jclass obj) {
	return GPCam_SendFirmwareUpgrade();
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendCVFirmwareDownload(
		JNIEnv *env, jclass obj, jlong ui32FileSize, jlong ui32CheckSum) {
	return GPCam_SendCVFirmwareDownload(ui32FileSize, ui32CheckSum);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendCVFirmwareRawData(
		JNIEnv *env, jclass obj, jlong ui32Size, jbyteArray pbyData) {
	unsigned char *Data_buf;

	jbyte *arr = env->GetByteArrayElements(pbyData, 0);
	Data_buf = (unsigned char*) arr;
	jint i32Ret = GPCam_SendCVFirmwareRawData(ui32Size, Data_buf);
	env->ReleaseByteArrayElements(pbyData, arr, 0);
	return i32Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendCVFirmwareUpgrade(
		JNIEnv *env, jclass obj, jlong ui32Area) {
	return GPCam_SendCVFirmwareUpgrade(ui32Area);
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendVendorCmd(
		JNIEnv *env, jclass obj, jbyteArray pbyData, jint i32Size) {
	unsigned char *Data_buf;
	jbyte *arr = env->GetByteArrayElements(pbyData, 0);
	Data_buf = (unsigned char*) arr;
	jint i32Ret = GPCam_SendVendorCmd(Data_buf, i32Size);
	env->ReleaseByteArrayElements(pbyData, arr, 0);
	return i32Ret;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetStatus(
		JNIEnv *env, jclass obj) {
	return GPCam_GetStatus();
}
//---------------------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileName(
		JNIEnv *env, jclass obj, jint i32Index) {
	jstring str;
	str = env->NewStringUTF(GPCam_GetFileName(i32Index));
	return str;
}
//---------------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileTime(
		JNIEnv *env, jclass obj, jint i32Index, jbyteArray pTime) {
	unsigned char *Data_buf;
	jbyte *arr = env->GetByteArrayElements(pTime, 0);
	Data_buf = (unsigned char*) arr;
	jboolean bRet = GPCam_GetFileTime(i32Index, Data_buf);
	env->ReleaseByteArrayElements(pTime, arr, 0);
	return bRet;
}
//---------------------------------------------------------------------------------
JNIEXPORT jint JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileIndex(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_GetFileIndex(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileSize(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_GetFileSize(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jbyte JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileExt(
		JNIEnv *env, jclass obj, jint i32Index) {
	return GPCam_GetFileExt(i32Index);
}
//---------------------------------------------------------------------------------
JNIEXPORT jbyteArray JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamGetFileExtraInfo(
		JNIEnv *env, jclass obj, jint i32Index) {
	int i32ExtraSize = 0;
	BYTE *pbyExtraInfo = GPCam_GetFileExtraInfo(i32Index, &i32ExtraSize);

	jbyteArray data = env->NewByteArray(i32ExtraSize);
	if (data == NULL) {
		return NULL; //  out of memory error thrown
	}

	jbyte *bytes = env->GetByteArrayElements(data, 0);
	int i;
	for (i = 0; i < i32ExtraSize; i++) {
		bytes[i] = pbyExtraInfo[i];
	}

	// move from the temp structure to the java structure
	env->SetByteArrayRegion(data, 0, i32ExtraSize, bytes);

	return data;
}
//---------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamClearCommandQueue
(JNIEnv *env, jclass obj) {
	GPCam_ClearCmdQueue();
}
//---------------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSetFileNameMapping(
		JNIEnv *env, jclass obj, jstring strMapping) {
	jboolean jBool;
	const char *mapping_buf = env->GetStringUTFChars(strMapping, &jBool);
	jboolean bRet = GPCam_SetFileNameMapping((LPCTSTR) mapping_buf);
	env->ReleaseStringUTFChars(strMapping, mapping_buf);
	return bRet;
}
//---------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamCheckFileMapping
(JNIEnv *, jclass) {
	GPCam_CheckFileMapping();
}
//---------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_generalplus_com_GPCamLib_CamWrapper_GPCamSendGetFileByIndex(
		JNIEnv *env, jclass obj, jint i32Index) {
	GPCam_GetFileByIndex(i32Index);
}
