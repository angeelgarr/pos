#include<jni.h>
#include "go.h"

#define FUNC_NAME "sendData"
static JNIEnv *sgEnv;
static TLS jobject sgObj;
static jmethodID sgSend;

void testFunction(int a)
{
	if(!sgEnv)
	{
		LOGW("sgEnv is Empty:");
		return;
	}
	LOGW("a=:%d",a);
	(*sgEnv)->ExceptionClear(sgEnv);
	(*sgEnv)->CallVoidMethod(sgEnv, sgObj, sgSend,a);
	if((*sgEnv)->ExceptionOccurred(sgEnv))
		{
			LOGE("fail to set progress");
			return;
		}
		return;
}


JNIEXPORT jint JNICALL Java_com_example_newdemo02_JniClient_initNdk(JNIEnv *env,jobject obj,jint a,jint b)
{
	LOGW("calling function:%s",__func__);
	return a + b;

}

JNIEXPORT jstring JNICALL Java_com_example_newdemo02_JniClient_startNdk(JNIEnv *env,jobject obj,jstring str)
{
	return str;
}

JNIEXPORT jint JNICALL Java_com_example_newdemo02_JniClient_initMethod
  (JNIEnv *env, jclass cls){

	LOGI("start ininMethod");
		sgSend = (*env)->GetMethodID(env,cls,FUNC_NAME,"(I)V");//Note:(I)V represent void func(int)
		                                            /**   "([BII)I" I stand int   "()Z" z stand for boolean
																**/
		if(!sgSend) 
		{
			LOGW("can not find the %s",FUNC_NAME);
		}
	LOGI("finish ininMethod");
}

JNIEXPORT void JNICALL Java_com_example_newdemo02_JniClient_initEnv(JNIEnv *env,jobject obj)
{

	    LOGI("start   initEnv...");
		sgEnv = env;
		sgObj = (*env)->NewGlobalRef(env, obj);//Note:initEnc() can't be static in java code 
		LOGI("sgEnv=%d,sgObj=%d",sgEnv,sgObj);
		LOGI("finish  initEnv...");
}

JNIEXPORT void JNICALL Java_com_example_newdemo02_JniClient_nativeMethod
  (JNIEnv *env, jobject obj){
	LOGI("start call nativeMethod");
	testFunction(500);
	LOGI("End call nativeMethod");
}


