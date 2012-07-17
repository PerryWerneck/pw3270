#!/bin/bash -x
adb logcat | ndk-stack -sym obj/local/armeabi

