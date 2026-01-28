#include <openxr/openxr.h>

// Not in the official OpenXR spec yet, selectively extracted from here:
// https://github.com/google-ar/jetpack-xr-natives/blob/release/third_party/OpenXR_KHR/generated/include/openxr/openxr.h

typedef enum XrStructureType_LE {
    XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID = 1000700000,
    XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID = 1000700001,
    XR_TYPE_LIGHT_ESTIMATE_ANDROID = 1000700002,
    XR_TYPE_DIRECTIONAL_LIGHT_ANDROID = 1000700003,
    XR_TYPE_SPHERICAL_HARMONICS_ANDROID = 1000700004,
    XR_TYPE_AMBIENT_LIGHT_ANDROID = 1000700005,
    XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_ANDROID = 1000700006,
    
    XR_TYPE_SYSTEM_ADVANCED_LIGHT_ESTIMATION_PROPERTIES_ANDROID = 1000721000,
    XR_TYPE_ADVANCED_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID = 1000721001,
    XR_TYPE_LIGHTING_CUBEMAP_DATA_ANDROID = 1000721002,
    XR_TYPE_POINT_LIGHTS_ANDROID = 1000721003,

    XR_OBJECT_TYPE_LIGHT_ESTIMATOR_ANDROID = 1000700000,
} XrStructureType_LE;

// XR_ANDROID_light_estimation is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_light_estimation 1
XR_DEFINE_HANDLE(XrLightEstimatorANDROID)
#define XR_ANDROID_light_estimation_SPEC_VERSION 1
#define XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME "XR_ANDROID_light_estimation"

typedef enum XrLightEstimateStateANDROID {
    XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID = 0,
    XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID = 1,
    XR_LIGHT_ESTIMATE_STATE_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrLightEstimateStateANDROID;

typedef enum XrSphericalHarmonicsKindANDROID {
    XR_SPHERICAL_HARMONICS_KIND_TOTAL_ANDROID = 0,
    XR_SPHERICAL_HARMONICS_KIND_AMBIENT_ANDROID = 1,
    XR_SPHERICAL_HARMONICS_KIND_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrSphericalHarmonicsKindANDROID;
// XrSystemLightEstimationPropertiesANDROID extends XrSystemProperties
typedef struct XrSystemLightEstimationPropertiesANDROID {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsLightEstimation;
} XrSystemLightEstimationPropertiesANDROID;

typedef struct XrLightEstimatorCreateInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrLightEstimatorCreateInfoANDROID;

typedef struct XrLightEstimateGetInfoANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
} XrLightEstimateGetInfoANDROID;

typedef struct XrLightEstimateANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrTime                         lastUpdatedTime;
} XrLightEstimateANDROID;

// XrDirectionalLightANDROID extends XrLightEstimateANDROID
typedef struct XrDirectionalLightANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrVector3f                     intensity;
    XrVector3f                     direction;
} XrDirectionalLightANDROID;

// XrAmbientLightANDROID extends XrLightEstimateANDROID
typedef struct XrAmbientLightANDROID {
    XrStructureType                type;
    void* XR_MAY_ALIAS             next;
    XrLightEstimateStateANDROID    state;
    XrVector3f                     intensity;
    XrVector3f                     colorCorrection;
} XrAmbientLightANDROID;

// XrSphericalHarmonicsANDROID extends XrLightEstimateANDROID
typedef struct XrSphericalHarmonicsANDROID {
    XrStructureType                    type;
    void* XR_MAY_ALIAS                 next;
    XrLightEstimateStateANDROID        state;
    XrSphericalHarmonicsKindANDROID    kind;
    float                              coefficients[9][3];
} XrSphericalHarmonicsANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrCreateLightEstimatorANDROID)(XrSession session, XrLightEstimatorCreateInfoANDROID* createInfo, XrLightEstimatorANDROID* outHandle);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyLightEstimatorANDROID)(XrLightEstimatorANDROID estimator);
typedef XrResult (XRAPI_PTR *PFN_xrGetLightEstimateANDROID)(XrLightEstimatorANDROID estimator, const XrLightEstimateGetInfoANDROID* input, XrLightEstimateANDROID* output);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrCreateLightEstimatorANDROID(
    XrSession                                   session,
    XrLightEstimatorCreateInfoANDROID*          createInfo,
    XrLightEstimatorANDROID*                    outHandle);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyLightEstimatorANDROID(
    XrLightEstimatorANDROID                     estimator);

XRAPI_ATTR XrResult XRAPI_CALL xrGetLightEstimateANDROID(
    XrLightEstimatorANDROID                     estimator,
    const XrLightEstimateGetInfoANDROID*        input,
    XrLightEstimateANDROID*                     output);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */