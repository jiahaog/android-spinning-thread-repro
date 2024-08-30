# SpinningThreadRepro

## Repro

1. Open with Android Studio
2. [Choose](https://developer.android.com/studio/run#changing-variant) the `release` Active Build Variant (Tool Windows > Build Variants)
3. Run the app on the device

It should crash. In the logcat, we should see that the "blah jni main" log is printed multiple times across hundreds of threads, even though it is only called once in `MainActivity#onCreate`.

## APK

The built arm64 APK can be found at https://github.com/jiahaog/android-spinning-thread-repro/blob/main/app-release.apk

## C++ Build Command

Can be found in `app/.cxx/RelWithDebInfo/4n184f62/arm64-v8a/compile_commands.json` after a successful build.

## Copied from the bug report

b/363925227

This is a repro constructed from https://github.com/jiahaog/android-spinning-thread-repro.

All this app does is:

1. On `onCreate`, makes a JNI call
2. the JNI call creates a thread which runs `while (true) {} ` forever. The [code](https://github.com/jiahaog/android-spinning-thread-repro/blob/main/app/src/main/cpp/native-lib.cpp) is below:

```cc
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
```

For some reason, the app crashes shortly after, and we see that `blah jni main`, which should only be logged once, is logged hundreds of times, across hundreds of threads.

This only happens when compiling for the "release" Active Build Variant. I have attached the `app/.cxx/RelWithDebInfo/4n184f62/arm64-v8a/compile_commands.json` which contains the command below:

```
/Users/jiahaog/Library/Android/sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++ \
  --target=aarch64-none-linux-android24 \
  --sysroot=/Users/jiahaog/Library/Android/sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/darwin-x86_64/sysroot \
  -Dspinningthreadrepro_EXPORTS \
  -g \
  -DANDROID \
  -fdata-sections \
  -ffunction-sections \
  -funwind-tables \
  -fstack-protector-strong \
  -no-canonical-prefixes \
  -D_FORTIFY_SOURCE=2 \
  -Wformat \
  -Werror=format-security \
  -O2 \
  -g \
  -DNDEBUG \
  -fPIC \
  -o \
  CMakeFiles/spinningthreadrepro.dir/native-lib.cpp.o \
  -c /Users/jiahaog/StudioProjects/SpinningThreadRepro/app/src/main/cpp/native-lib.cpp
```

The above bug report is from a physical Pixel 6a, but I also tested this on:

- Pixel 6a API 30 x86 emulator
- Various physical devices with the SM8650 chipset

