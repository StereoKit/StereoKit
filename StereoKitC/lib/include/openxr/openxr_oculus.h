/************************************************************************************

Filename    :   openxr_oculus.h
Content     :   Oculus OpenXR Extensions.
Language    :   C99

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#ifndef OPENXR_OCULUS_H_
#define OPENXR_OCULUS_H_ 1

#if defined(__cplusplus)
extern "C" {
#endif

// Extension 89
#define XR_KHR_loader_init 1
#define XR_KHR_loader_init_SPEC_VERSION 1
#define XR_KHR_LOADER_INIT_EXTENSION_NAME "XR_KHR_loader_init"
typedef struct XR_MAY_ALIAS XrLoaderInitInfoBaseHeaderKHR {
    XrStructureType type;
    const void* XR_MAY_ALIAS next;
} XrLoaderInitInfoBaseHeaderKHR;

typedef XrResult(XRAPI_PTR* PFN_xrInitializeLoaderKHR)(
    const XrLoaderInitInfoBaseHeaderKHR* loaderInitInfo);

#ifndef XR_NO_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL
xrInitializeLoaderKHR(const XrLoaderInitInfoBaseHeaderKHR* loaderInitInfo);
#endif

// Extension 90
#ifdef XR_USE_PLATFORM_ANDROID

#define XR_KHR_loader_init_android 1
#define XR_KHR_loader_init_android_SPEC_VERSION 1
#define XR_KHR_LOADER_INIT_ANDROID_EXTENSION_NAME "XR_KHR_loader_init_android"
typedef struct XrLoaderInitInfoAndroidKHR {
    XrStructureType type;
    const void* XR_MAY_ALIAS next;
    void* XR_MAY_ALIAS applicationVM;
    void* XR_MAY_ALIAS applicationContext;
} XrLoaderInitInfoAndroidKHR;

static const XrStructureType XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR = (XrStructureType)1000089000;
#endif /* XR_USE_PLATFORM_ANDROID */

#ifdef __cplusplus
}
#endif

#endif // OPENXR_OCULUS_H_
