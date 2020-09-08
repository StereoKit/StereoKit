#pragma once

#include "openxr.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define FOR_EACH_EXTENSION_FUNCTION(_)           \
    _(xrCreateSpatialAnchorMSFT)                 \
    _(xrCreateSpatialAnchorSpaceMSFT)            \
    _(xrDestroySpatialAnchorMSFT)                \
    _(xrConvertWin32PerformanceCounterToTimeKHR) \
    _(xrConvertTimeToWin32PerformanceCounterKHR) \
    _(xrGetGraphicsRequirementsKHR)              \
    _(xrGetVisibilityMaskKHR)                    \
    _(xrCreateHandTrackerEXT)                    \
    _(xrDestroyHandTrackerEXT)                   \
    _(xrLocateHandJointsEXT)                     \
    _(xrCreateSpatialGraphNodeSpaceMSFT)         \

#define GET_INSTANCE_PROC_ADDRESS(name) (void)xrGetInstanceProcAddr(instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&result.name)));
#define DEFINE_PROC_MEMBER(name) PFN_##name name;

struct XrExtTable {
    FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);
};

inline XrExtTable xrCreateExtensionTable(XrInstance instance) {
    XrExtTable result = {};
    FOR_EACH_EXTENSION_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
    return result;
}

#undef DEFINE_PROC_MEMBER
#undef GET_INSTANCE_PROC_ADDRESS
#undef FOR_EACH_EXTENSION_FUNCTION