# These are used by AndroidManifest.xml. We can calculate them sufficiently
# well this way, but feel free to alter this.
set(APK_VERSION_NAME       "${CMAKE_PROJECT_VERSION}")
math(EXPR APK_VERSION_CODE "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")

###############################################################################
## Find our SDK locations and paths
###############################################################################

# Find the Android SDK
if(NOT DEFINED ANDROID_SDK_ROOT)
	if(DEFINED ENV{ANDROID_HOME})
		set(ANDROID_SDK_ROOT $ENV{ANDROID_HOME})
	elseif(DEFINED ENV{ANDROID_SDK_ROOT})
		set(ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})
	elseif(DEFINED ANDROID_HOME)
		set(ANDROID_SDK_ROOT ANDROID_HOME)
	endif()
endif()
if (NOT EXISTS "${ANDROID_SDK_ROOT}/platform-tools")
	message(FATAL_ERROR "Android SDK not found. Set ANDROID_HOME or ANDROID_SDK_ROOT as an environment or cmake variable.")
endif()

# Find a build-tools folder in the Android SDK that matches our CMAKE_SYSTEM_VERSION
if(ANDROID_BUILD_TOOLS_VERSION AND EXISTS "${ANDROID_SDK_ROOT}/build-tools/${ANDROID_BUILD_TOOLS_VERSION}")
	set(ANDROID_BUILD_TOOLS_PATH "${ANDROID_SDK_ROOT}/build-tools/${ANDROID_BUILD_TOOLS_VERSION}")
else()
	file(GLOB BUILD_TOOLS_VERSIONS LIST_DIRECTORIES true "${ANDROID_SDK_ROOT}/build-tools/*")
	foreach(VERSION_DIR ${BUILD_TOOLS_VERSIONS})
		get_filename_component(VERSION_NAME ${VERSION_DIR} NAME)
		if(VERSION_NAME MATCHES "${CMAKE_SYSTEM_VERSION}\.")
			set(ANDROID_BUILD_TOOLS_PATH "${VERSION_DIR}")
			break()
		endif()
	endforeach()
endif()
if(NOT EXISTS "${ANDROID_BUILD_TOOLS_PATH}")
	message(STATUS "Can't find build-tools matching ANDROID_BUILD_TOOLS_VERSION ${ANDROID_BUILD_TOOLS_VERSION} or CMAKE_SYSTEM_VERSION ${CMAKE_SYSTEM_VERSION}")
endif()

# Find JDK bin folder
if(NOT DEFINED JAVA_HOME)
	if(DEFINED ENV{JAVA_HOME})
		set(JAVA_HOME $ENV{JAVA_HOME})
	endif()
endif()
if (DEFINED JAVA_HOME)
	cmake_path(APPEND JAVA_HOME_BIN ${JAVA_HOME} "bin")
endif()

message(STATUS "APK build var - ANDROID_SDK_ROOT         : ${ANDROID_SDK_ROOT}")
message(STATUS "APK build var - CMAKE_ANDROID_NDK        : ${CMAKE_ANDROID_NDK}")
message(STATUS "APK build var - ANDROID_BUILD_TOOLS_PATH : ${ANDROID_BUILD_TOOLS_PATH}")
message(STATUS "APK build var - JAVA_HOME_BIN            : ${JAVA_HOME_BIN}")
message(STATUS "APK build var - CMAKE_ANDROID_ARCH_ABI   : ${CMAKE_ANDROID_ARCH_ABI}")
message(STATUS "APK build var - CMAKE_SYSTEM_VERSION     : ${CMAKE_SYSTEM_VERSION}")
if(NOT DEFINED ANDROID_DEBUGGABLE)
	if(CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
		set(ANDROID_DEBUGGABLE true)
	else()
		set(ANDROID_DEBUGGABLE false)
	endif()
endif()
message(STATUS "APK build var - ANDROID_DEBUGGABLE       : ${ANDROID_DEBUGGABLE}")

###############################################################################
## Get tools for building APKs
###############################################################################

set(AAPT2    "${ANDROID_BUILD_TOOLS_PATH}/aapt2")
set(AAPT     "${ANDROID_BUILD_TOOLS_PATH}/aapt")
set(ZIPALIGN "${ANDROID_BUILD_TOOLS_PATH}/zipalign")
set(APKSIGN  "${ANDROID_BUILD_TOOLS_PATH}/apksigner")
set(D8       "${ANDROID_BUILD_TOOLS_PATH}/d8")
find_program(JAVAC    NAMES javac     REQUIRED PATHS ${JAVA_HOME_BIN})
find_program(KEYTOOL  NAMES keytool   REQUIRED PATHS ${JAVA_HOME_BIN})
# https://developer.android.com/tools/aapt2
# https://developer.android.com/build/building-cmdline

###############################################################################
## Keystore for signing the APK
###############################################################################

# Set default keystore variables
set(DEFAULT_KEYSTORE       "${ANDROID_SOURCE_DIR}/debug.keystore")
set(DEFAULT_KEYSTORE_ALIAS "androiddebugkey")
set(DEFAULT_KEYSTORE_PASS  "android")
set(DEFAULT_KEY_ALIAS_PASS "android")

# Check if keystore variables are provided, otherwise use defaults
set(KEYSTORE       "${DEFAULT_KEYSTORE}"       CACHE STRING "Path to the keystore")
set(KEY_ALIAS      "${DEFAULT_KEYSTORE_ALIAS}" CACHE STRING "Alias for the key")
set(KEYSTORE_PASS  "${DEFAULT_KEYSTORE_PASS}"  CACHE STRING "Password for the keystore")
set(KEY_ALIAS_PASS "${DEFAULT_KEY_ALIAS_PASS}" CACHE STRING "Password for the key")

if(NOT EXISTS "${KEYSTORE}")
	message(STATUS "APK keystore not found, generating new keystore...")
	execute_process(COMMAND ${KEYTOOL}
		-genkeypair -v
		-keyalg RSA -keysize 2048 -validity 10000
		-keystore "${KEYSTORE}" -alias "${KEY_ALIAS}"
		-storepass "${KEYSTORE_PASS}" -keypass "${KEY_ALIAS_PASS}"
		-dname "CN=Android Debug,O=Android,C=US"
		RESULT_VARIABLE KEYTOOL_RESULT)
	if(NOT KEYTOOL_RESULT EQUAL "0")
		message(FATAL_ERROR "Failed to create keystore")
	endif()
endif()

# Debug message to confirm which keystore is being used
message(STATUS "APK using keystore: ${KEYSTORE} with alias ${KEY_ALIAS}")

###############################################################################
## Add glue code and libraries
###############################################################################

# Modify the main project to include all libraries and code necessary to glue
# the app to Android.
target_link_libraries     (${ANDROID_APP_TARGET} PRIVATE android log )
target_include_directories(${ANDROID_APP_TARGET} PUBLIC  ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)
target_sources            (${ANDROID_APP_TARGET} PRIVATE 
	${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c 
	${ANDROID_SOURCE_DIR}/android_main.cpp)

###############################################################################
## Get a list of shared libraries to pack
###############################################################################

# Here, we go through all libraries that ${ANDROID_APP_TARGET} depends on, check if
# they're shared, and then set up some paths for when we need to copy them into
# the APK. This currently doesn't do recursive searching, so if your
# dependencies have dependencies that are shared, you may need to improve this!
get_target_property(PROJECT_LIBRARIES ${ANDROID_APP_TARGET} LINK_LIBRARIES)
set(APK_SRC_LIBRARIES $<TARGET_FILE:${ANDROID_APP_TARGET}>)
set(APK_COPY_LIBRARIES lib/${CMAKE_ANDROID_ARCH_ABI}/$<TARGET_FILE_NAME:${ANDROID_APP_TARGET}>)
foreach(CURR ${PROJECT_LIBRARIES})
	if (TARGET ${CURR})
		get_target_property(TARGET_TYPE ${CURR} TYPE)
		if(${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
			list(APPEND APK_SRC_LIBRARIES $<TARGET_FILE:${CURR}>)
			list(APPEND APK_COPY_LIBRARIES lib/${CMAKE_ANDROID_ARCH_ABI}/$<TARGET_FILE_NAME:${CURR}>)
		endif()
	endif()
endforeach()
# Ensure OpenXR loader is packaged if present as a shared target
if(TARGET openxr_loader)
	get_target_property(TARGET_TYPE openxr_loader TYPE)
	if(${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
		list(APPEND APK_SRC_LIBRARIES  $<TARGET_FILE:openxr_loader>)
		list(APPEND APK_COPY_LIBRARIES lib/${CMAKE_ANDROID_ARCH_ABI}/$<TARGET_FILE_NAME:openxr_loader>)
	endif()
endif()

###############################################################################
## Building the APK
###############################################################################

# If these files exist from a previous build, we get overwrite errors when
# generating the APK.
set(APK_TEMP "${CMAKE_CURRENT_BINARY_DIR}/apk")
set(APK_BASE      ${APK_TEMP}/${ANDROID_APP_TARGET}.1.base.apk)
set(APK_UNALIGNED ${APK_TEMP}/${ANDROID_APP_TARGET}.2.unaligned.apk)
set(APK_UNSIGNED  ${APK_TEMP}/${ANDROID_APP_TARGET}.3.unsigned.apk)

# We need to make a few folders so copies can succeed when copying to them.
file(MAKE_DIRECTORY 
	"${APK_TEMP}/obj"
	"${APK_TEMP}/lib/${CMAKE_ANDROID_ARCH_ABI}")

# Manifest has a couple name/number variables that we want to resolve nicely!
configure_file(
	${ANDROID_SOURCE_DIR}/AndroidManifest.xml
	${APK_TEMP}/obj/AndroidManifest.xml
	@ONLY)

# Put together a dummy java file for the APK
file(WRITE ${APK_TEMP}/src/android/Empty.java "public class Empty {}")
add_custom_command(
	DEPENDS ${APK_TEMP}/src/android/Empty.java
	OUTPUT  ${APK_TEMP}/obj/classes.dex
	COMMAND ${JAVAC} -d ${APK_TEMP}/obj -sourcepath src ${APK_TEMP}/src/android/Empty.java
	COMMAND ${D8} --release ${APK_TEMP}/obj/Empty.class --output ${APK_TEMP}/obj
	COMMENT "Building Java boilerplate for APK" )

# Build the resources
add_custom_command(
	OUTPUT  ${APK_TEMP}/obj/apk_resources.zip
	DEPENDS ${ANDROID_RESOURCE_DIR}
	COMMAND ${AAPT2} compile
		--dir ${ANDROID_RESOURCE_DIR}
		-o ${APK_TEMP}/obj/apk_resources.zip
	COMMENT "Compiling APK resources" )

# Assemble the base APK, resources and assets
add_custom_command(
	DEPENDS
		${ANDROID_ASSETS_DIR}
		${APK_TEMP}/obj/classes.dex
		${APK_TEMP}/obj/apk_resources.zip
		${APK_TEMP}/obj/AndroidManifest.xml
	OUTPUT
		${APK_BASE}
	COMMAND ${CMAKE_COMMAND} -E rm -f ${APK_BASE}
	COMMAND ${AAPT2} link # Link all the files into an APK
		-o ${APK_BASE}
		--manifest ${APK_TEMP}/obj/AndroidManifest.xml
		-A ${ANDROID_ASSETS_DIR}
		-I ${ANDROID_SDK_ROOT}/platforms/android-${CMAKE_SYSTEM_VERSION}/android.jar
		${APK_TEMP}/obj/apk_resources.zip
	COMMAND cd ${APK_TEMP}/obj
	COMMAND ${AAPT} add ${APK_BASE} classes.dex
    COMMENT "Building base APK")

# Assemble the final APK, add binaries and align/sign the base APK
add_custom_command(
	DEPENDS
		${ANDROID_APP_TARGET}
		${APK_BASE}
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_APP_TARGET}.apk
	COMMAND ${CMAKE_COMMAND} -E rm -f
		${APK_UNALIGNED}
		${APK_UNSIGNED}
		${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_APP_TARGET}.apk
	COMMAND cd ${APK_TEMP}/obj
	COMMAND ${CMAKE_COMMAND} -E copy
		${APK_SRC_LIBRARIES}
		${APK_TEMP}/lib/${CMAKE_ANDROID_ARCH_ABI}/
	COMMAND cd ${APK_TEMP}
	COMMAND ${CMAKE_COMMAND} -E copy ${APK_BASE} ${APK_UNALIGNED}
	COMMAND ${AAPT} add ${APK_UNALIGNED} ${APK_COPY_LIBRARIES}
	COMMAND ${ZIPALIGN} -p 4 ${APK_UNALIGNED} ${APK_UNSIGNED}
	COMMAND ${APKSIGN} sign --ks ${KEYSTORE} --ks-key-alias ${KEY_ALIAS} --ks-pass pass:${KEYSTORE_PASS} --key-pass pass:${KEY_ALIAS_PASS} --out ${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_APP_TARGET}.apk ${APK_UNSIGNED}
	COMMENT "Building final APK")

# Wrap up APK building into a target!
add_custom_target(${ANDROID_APP_TARGET}_Android_apk
	DEPENDS 
		${ANDROID_APP_TARGET} 
		${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_APP_TARGET}.apk
	COMMENT "Building Android APK")

# A convenience target for installing and running the APK we build.
add_custom_target(${ANDROID_APP_TARGET}_Android_run
	DEPENDS apk
	COMMAND ${ANDROID_SDK_ROOT}/platform-tools/adb install ${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_APP_TARGET}.apk
	COMMAND ${ANDROID_SDK_ROOT}/platform-tools/adb shell am start -n ${ANDROID_PACKAGE_NAME}/android.app.NativeActivity
	COMMENT "Running Android APK")
