#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#define APPNAME "MyApp"

void* busy_thread(void* arg) {
  while (true) {}
  return nullptr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_spinningthreadrepro_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
  __android_log_write(ANDROID_LOG_WARN, APPNAME, "blah jni main");

  pthread_t thread_id;
  int ret = pthread_create(&thread_id, nullptr, busy_thread, nullptr);
  if (ret != 0) {
    __android_log_write(ANDROID_LOG_WARN, APPNAME, "blah error creating thread");
    abort();
    return env->NewStringUTF("blah error creating thread");
  }

  return env->NewStringUTF("blah created thread");
}
