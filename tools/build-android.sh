#!/bin/bash
BUILD_DIR=$(pwd)
COPYRIGHT="# Copyright $(date +%Y) Bifrost Entertainment. All rights reserved."
HEADER="# This file was generated by $(basename $0)"
NDK_HOME="/usr/local/opt/android-ndk"
PACKAGE="com/bifrostentertainment/rainbow"
PROJECT=$(cd -P "$(dirname $0)/.." && pwd)
TARGET=rainbow

# Auto-generate files
cd $PROJECT
tools/shaders-gen.py
cd $BUILD_DIR

# Clean the folder
rm -fr AndroidManifest.xml ant.properties bin build.xml gen jni libs \
	local.properties obj proguard-project.txt project.properties res src

# Create project files
mkdir jni
android -s create project --name "Rainbow" \
	--target "android-19" --path jni \
	--package "com.bifrostentertainment.rainbow" --activity "Rainbow" || exit 1
rm -r jni/src/*
mv jni/* .

# Create package and link to RainbowActivity
mkdir -p src/$PACKAGE
ln -s $PROJECT/src/Platform/RainbowActivity.java src/$PACKAGE/

echo -n "Generating jni/Android.mk..."

# Gather Rainbow source files
cd $PROJECT
SRC_FILES=$(find src -name '*.cpp' | xargs)

# Include libraries
libraries=("Lua")
if [[ $@ = *USE_PHYSICS* ]]; then
	libraries+=("Box2D")
fi
for lib in "${libraries[@]}"; do
	SRC_FILES="$SRC_FILES \\"$'\n'$(find lib/$lib -name '*.c' -and ! -name 'lua.c' -and ! -name 'luac.c' -or -name '*.cpp' | xargs)
done

# Include FreeType and libpng
SRC_FILES="$SRC_FILES src/ThirdParty/FreeType/freetype.c src/ThirdParty/libpng/libpng.c"

cd $BUILD_DIR

cat > jni/Android.mk << ANDROID_MK
$COPYRIGHT
$HEADER

include \$(CLEAR_VARS)

LOCAL_PATH := $PROJECT
LOCAL_MODULE := $TARGET
LOCAL_SRC_FILES := $SRC_FILES

LOCAL_C_INCLUDES := $PROJECT/src $PROJECT/lib \
                    $PROJECT/src/ThirdParty/FreeType $PROJECT/lib/FreeType/include $PROJECT/lib/FreeType/src \
                    $PROJECT/lib/Lua \
                    $PROJECT/src/ThirdParty/libpng $PROJECT/lib/libpng \
                    $NDK_HOME/sources/android/native_app_glue
LOCAL_CFLAGS := $@
LOCAL_CPPFLAGS := -std=c++11 -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo -fno-rtti -fno-exceptions

LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_LDLIBS := -landroid -lEGL -lGLESv2 -lOpenSLES -llog -lz

include \$(BUILD_SHARED_LIBRARY)
\$(call import-module,android/native_app_glue)
ANDROID_MK
echo " done"

echo -n "Generating jni/Application.mk..."
cat > jni/Application.mk << APPLICATION_MK
$COPYRIGHT
$HEADER

APP_ABI := armeabi-v7a  # all
APP_PLATFORM := android-9
APP_STL := gnustl_shared  # Required by Box2D
APPLICATION_MK
echo " done"

echo -n "Generating res/values/themes.xml..."
cat > res/values/themes.xml << THEMES_XML
<?xml version="1.0" encoding="utf-8"?>
<resources>
	<style name="Rainbow.Theme.Default" parent="@android:style/Theme.NoTitleBar.Fullscreen"></style>
</resources>
THEMES_XML
echo " done"

echo -n "Generating res/values-v11/themes.xml..."
mkdir -p res/values-v11
cat > res/values-v11/themes.xml << THEMES_XML
<?xml version="1.0" encoding="utf-8"?>
<resources>
	<style name="Rainbow.Theme.Default" parent="@android:style/Theme.Holo.NoActionBar.Fullscreen"></style>
</resources>
THEMES_XML
echo " done"

echo -n "Generating AndroidManifest.xml..."
cat > AndroidManifest.xml << ANDROIDMANIFEST_XML
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
          package="com.bifrostentertainment.rainbow"
          android:versionCode="1"
          android:versionName="1.0">
	<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />
	<uses-permission android:name="android.permission.RECORD_AUDIO" />
	<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
	<uses-sdk android:minSdkVersion="9" android:targetSdkVersion="14" />
	<uses-feature android:name="android.hardware.screen.portrait"
	              android:glEsVersion="0x00020000" />
	<application android:icon="@drawable/ic_launcher"
	             android:label="@string/app_name"
	             android:theme="@style/Rainbow.Theme.Default">
		<activity android:configChanges="orientation|screenSize"
		          android:label="@string/app_name"
		          android:launchMode="singleTop"
		          android:name=".RainbowActivity"
		          android:screenOrientation="sensorLandscape">
			<intent-filter>
				<action android:name="android.intent.action.MAIN" />
				<category android:name="android.intent.category.LAUNCHER" />
			</intent-filter>
			<meta-data android:name="android.app.lib_name" android:value="rainbow" />
		</activity>
	</application>
</manifest>
ANDROIDMANIFEST_XML
echo " done"

NDK_DEBUG=${NDK_DEBUG:-1} NDK_TOOLCHAIN_VERSION=${NDK_TOOLCHAIN_VERSION:-clang} ndk-build -j &&
ant debug
