#include <openxr/openxr.h>

typedef enum XrEnums_su {
    XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT = 1000097000,
    XR_TYPE_SCENE_CREATE_INFO_MSFT = 1000097001,
    XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT = 1000097002,
    XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT = 1000097003,
    XR_TYPE_SCENE_COMPONENT_STATES_MSFT = 1000097004,
    XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT = 1000097005,
    XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT = 1000097006,
    XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT = 1000097007,
    XR_TYPE_SCENE_OBJECT_STATES_MSFT = 1000097008,
    XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT = 1000097009,
    XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT = 1000097010,
    XR_TYPE_SCENE_PLANE_STATES_MSFT = 1000097011,
    XR_TYPE_SCENE_PLANE_ALIGNMENT_FILTER_INFO_MSFT = 1000097012,
    XR_TYPE_SCENE_MESH_STATES_MSFT = 1000097013,
    XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT = 1000097014,
    XR_TYPE_SCENE_MESH_BUFFERS_MSFT = 1000097015,
    XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT = 1000097016,
    XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT = 1000097017,
    XR_TYPE_SCENE_MESH_INDICES_UINT16_MSFT = 1000097018,
    XR_TYPE_SERIALIZED_SCENE_FRAGMENT_DATA_GET_INFO_MSFT = 1000098000,
    XR_TYPE_SCENE_DESERIALIZE_INFO_MSFT = 1000098001,

    XR_ERROR_COMPUTE_NEW_SCENE_NOT_COMPLETED_MSFT = -1000097000,
    XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT = -1000097001,
    XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT = -1000097002,
    XR_ERROR_SCENE_MESH_BUFFER_ID_INVALID_MSFT = -1000097003,
    XR_ERROR_SCENE_COMPUTE_FEATURE_INCOMPATIBLE_MSFT = -1000097004,

    XR_OBJECT_TYPE_SCENE_OBSERVER_MSFT = 1000097000,
    XR_OBJECT_TYPE_SCENE_MSFT = 1000097001,
} XrEnums_su;

#define XR_MSFT_scene_understanding_preview3 1
            XR_DEFINE_HANDLE(XrSceneObserverMSFT)
            XR_DEFINE_HANDLE(XrSceneMSFT)

#define XR_MSFT_scene_understanding_preview3_SPEC_VERSION 2
#define XR_MSFT_SCENE_UNDERSTANDING_PREVIEW3_EXTENSION_NAME "XR_MSFT_scene_understanding_preview3"

typedef enum XrSceneComputeFeatureMSFT {
    XR_SCENE_COMPUTE_FEATURE_PLANE_MSFT = 1,
    XR_SCENE_COMPUTE_FEATURE_PLANE_MESH_MSFT = 2,
    XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT = 3,
    XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT = 4,
    XR_SCENE_COMPUTE_FEATURE_SERIALIZE_SCENE_MSFT = 1000098000,
    XR_SCENE_COMPUTE_FEATURE_OCCLUSION_HINT_MSFT = 1000099000,
    XR_SCENE_COMPUTE_FEATURE_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrSceneComputeFeatureMSFT;

typedef enum XrMeshComputeLodMSFT {
    XR_MESH_COMPUTE_LOD_COARSE_MSFT = 1,
    XR_MESH_COMPUTE_LOD_MEDIUM_MSFT = 2,
    XR_MESH_COMPUTE_LOD_FINE_MSFT = 3,
    XR_MESH_COMPUTE_LOD_UNLIMITED_MSFT = 4,
    XR_MESH_COMPUTE_LOD_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrMeshComputeLodMSFT;

typedef enum XrSceneComponentTypeMSFT {
    XR_SCENE_COMPONENT_TYPE_INVALID_MSFT = -1,
    XR_SCENE_COMPONENT_TYPE_OBJECT_MSFT = 1,
    XR_SCENE_COMPONENT_TYPE_PLANE_MSFT = 2,
    XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT = 3,
    XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT = 4,
    XR_SCENE_COMPONENT_TYPE_SERIALIZED_SCENE_FRAGMENT_MSFT = 1000098000,
    XR_SCENE_COMPONENT_TYPE_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrSceneComponentTypeMSFT;

typedef enum XrSceneObjectTypeMSFT {
    XR_SCENE_OBJECT_TYPE_UNCATEGORIZED_MSFT = -1,
    XR_SCENE_OBJECT_TYPE_BACKGROUND_MSFT = 1,
    XR_SCENE_OBJECT_TYPE_WALL_MSFT = 2,
    XR_SCENE_OBJECT_TYPE_FLOOR_MSFT = 3,
    XR_SCENE_OBJECT_TYPE_CEILING_MSFT = 4,
    XR_SCENE_OBJECT_TYPE_PLATFORM_MSFT = 5,
    XR_SCENE_OBJECT_TYPE_INFERRED_MSFT = 6,
    XR_SCENE_OBJECT_TYPE_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrSceneObjectTypeMSFT;

typedef enum XrScenePlaneAlignmentTypeMSFT {
    XR_SCENE_PLANE_ALIGNMENT_TYPE_NON_ORTHOGONAL_MSFT = 0,
    XR_SCENE_PLANE_ALIGNMENT_TYPE_HORIZONTAL_MSFT = 1,
    XR_SCENE_PLANE_ALIGNMENT_TYPE_VERTICAL_MSFT = 2,
    XR_SCENE_PLANE_ALIGNMENT_TYPE_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrScenePlaneAlignmentTypeMSFT;

typedef enum XrSceneComputeStateMSFT {
    XR_SCENE_COMPUTE_STATE_NONE_MSFT = 0,
    XR_SCENE_COMPUTE_STATE_UPDATING_MSFT = 1,
    XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT = 2,
    XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT = 3,
    XR_SCENE_COMPUTE_STATE_MAX_ENUM_MSFT = 0x7FFFFFFF
} XrSceneComputeStateMSFT;
typedef struct XrUuidMSFT {
    uint8_t    bytes[16];
} XrUuidMSFT;

typedef struct XrSceneObserverCreateInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrSceneObserverCreateInfoMSFT;

typedef struct XrSceneCreateInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
} XrSceneCreateInfoMSFT;

typedef struct XrSceneSphereBoundMSFT {
    XrVector3f    center;
    float         radius;
} XrSceneSphereBoundMSFT;

typedef struct XrSceneOrientedBoxBoundMSFT {
    XrPosef       pose;
    XrVector3f    extents;
} XrSceneOrientedBoxBoundMSFT;

typedef struct XrSceneFrustumBoundMSFT {
    XrPosef    pose;
    XrFovf     fov;
    float      farDistance;
} XrSceneFrustumBoundMSFT;

typedef struct XrSceneBoundsMSFT {
    XrSpace                               space;
    XrTime                                time;
    uint32_t                              sphereCount;
    const XrSceneSphereBoundMSFT*         spheres;
    uint32_t                              boxCount;
    const XrSceneOrientedBoxBoundMSFT*    boxes;
    uint32_t                              frustumCount;
    const XrSceneFrustumBoundMSFT*        frustums;
} XrSceneBoundsMSFT;

typedef struct XrNewSceneComputeInfoMSFT {
    XrStructureType                     type;
    const void* XR_MAY_ALIAS            next;
    uint32_t                            requestedFeatureCount;
    const XrSceneComputeFeatureMSFT*    requestedFeatures;
    XrBool32                            disableInferredSceneObjects;
    XrSceneBoundsMSFT                   bounds;
} XrNewSceneComputeInfoMSFT;

// XrVisualMeshComputeLodInfoMSFT extends XrNewSceneComputeInfoMSFT
typedef struct XrVisualMeshComputeLodInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrMeshComputeLodMSFT        lod;
} XrVisualMeshComputeLodInfoMSFT;

typedef struct XrSceneComponentStateMSFT {
    XrSceneComponentTypeMSFT    componentType;
    XrUuidMSFT                  componentId;
    XrUuidMSFT                  parentObjectId;
    XrTime                      updateTime;
} XrSceneComponentStateMSFT;

typedef struct XrSceneComponentStatesMSFT {
    XrStructureType               type;
    void* XR_MAY_ALIAS            next;
    uint32_t                      componentCapacityInput;
    uint32_t                      componentCountOutput;
    XrSceneComponentStateMSFT*    components;
} XrSceneComponentStatesMSFT;

typedef struct XrSceneComponentsGetInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSceneComponentTypeMSFT    componentType;
} XrSceneComponentsGetInfoMSFT;

typedef struct XrSceneComponentLocationMSFT {
    XrSpaceLocationFlags    flags;
    XrPosef                 pose;
} XrSceneComponentLocationMSFT;

typedef struct XrSceneComponentLocationsMSFT {
    XrStructureType                  type;
    void* XR_MAY_ALIAS               next;
    uint32_t                         locationCount;
    XrSceneComponentLocationMSFT*    locations;
} XrSceneComponentLocationsMSFT;

typedef struct XrSceneComponentsLocateInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     baseSpace;
    XrTime                      time;
    uint32_t                    idCount;
    const XrUuidMSFT*           ids;
} XrSceneComponentsLocateInfoMSFT;

typedef struct XrSceneObjectStateMSFT {
    XrSceneObjectTypeMSFT    objectType;
} XrSceneObjectStateMSFT;

// XrSceneObjectStatesMSFT extends XrSceneComponentStatesMSFT
typedef struct XrSceneObjectStatesMSFT {
    XrStructureType            type;
    void* XR_MAY_ALIAS         next;
    uint32_t                   sceneObjectCount;
    XrSceneObjectStateMSFT*    sceneObjects;
} XrSceneObjectStatesMSFT;

// XrSceneComponentParentFilterInfoMSFT extends XrSceneComponentsGetInfoMSFT
typedef struct XrSceneComponentParentFilterInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrUuidMSFT                  parentObjectId;
} XrSceneComponentParentFilterInfoMSFT;

// XrSceneObjectTypesFilterInfoMSFT extends XrSceneComponentsGetInfoMSFT
typedef struct XrSceneObjectTypesFilterInfoMSFT {
    XrStructureType                 type;
    const void* XR_MAY_ALIAS        next;
    uint32_t                        objectTypeCount;
    const XrSceneObjectTypeMSFT*    objectTypes;
} XrSceneObjectTypesFilterInfoMSFT;

typedef struct XrScenePlaneStateMSFT {
    XrScenePlaneAlignmentTypeMSFT    alignment;
    XrExtent2Df                      size;
    uint64_t                         meshBufferId;
    XrBool32                         supportsIndicesUint16;
} XrScenePlaneStateMSFT;

// XrScenePlaneStatesMSFT extends XrSceneComponentStatesMSFT
typedef struct XrScenePlaneStatesMSFT {
    XrStructureType           type;
    void* XR_MAY_ALIAS        next;
    uint32_t                  scenePlaneCount;
    XrScenePlaneStateMSFT*    scenePlanes;
} XrScenePlaneStatesMSFT;

// XrScenePlaneAlignmentFilterInfoMSFT extends XrSceneComponentsGetInfoMSFT
typedef struct XrScenePlaneAlignmentFilterInfoMSFT {
    XrStructureType                         type;
    const void* XR_MAY_ALIAS                next;
    uint32_t                                alignmentCount;
    const XrScenePlaneAlignmentTypeMSFT*    alignments;
} XrScenePlaneAlignmentFilterInfoMSFT;

typedef struct XrSceneMeshStateMSFT {
    uint64_t    meshBufferId;
    XrBool32    supportsIndicesUint16;
} XrSceneMeshStateMSFT;

// XrSceneMeshStatesMSFT extends XrSceneComponentStatesMSFT
typedef struct XrSceneMeshStatesMSFT {
    XrStructureType          type;
    void* XR_MAY_ALIAS       next;
    uint32_t                 sceneMeshCount;
    XrSceneMeshStateMSFT*    sceneMeshes;
} XrSceneMeshStatesMSFT;

typedef struct XrSceneMeshBuffersGetInfoMSFT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    uint64_t                    meshBufferId;
} XrSceneMeshBuffersGetInfoMSFT;

typedef struct XrSceneMeshBuffersMSFT {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
} XrSceneMeshBuffersMSFT;

typedef struct XrSceneMeshVertexBufferMSFT {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    uint32_t              vertexCapacityInput;
    uint32_t              vertexCountOutput;
    XrVector3f*           vertices;
} XrSceneMeshVertexBufferMSFT;

typedef struct XrSceneMeshIndicesUint32MSFT {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    uint32_t              indexCapacityInput;
    uint32_t              indexCountOutput;
    uint32_t*             indices;
} XrSceneMeshIndicesUint32MSFT;

typedef struct XrSceneMeshIndicesUint16MSFT {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    uint32_t              indexCapacityInput;
    uint32_t              indexCountOutput;
    uint16_t*             indices;
} XrSceneMeshIndicesUint16MSFT;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateSceneComputeFeaturesMSFT)(XrInstance instance, XrSystemId systemId, uint32_t featureCapacityInput, uint32_t* featureCountOutput, XrSceneComputeFeatureMSFT* features);
typedef XrResult (XRAPI_PTR *PFN_xrCreateSceneObserverMSFT)(XrSession session, const XrSceneObserverCreateInfoMSFT* createInfo, XrSceneObserverMSFT* sceneObserver);
typedef XrResult (XRAPI_PTR *PFN_xrDestroySceneObserverMSFT)(XrSceneObserverMSFT sceneObserver);
typedef XrResult (XRAPI_PTR *PFN_xrCreateSceneMSFT)(XrSceneObserverMSFT sceneObserver, const XrSceneCreateInfoMSFT* createInfo, XrSceneMSFT* scene);
typedef XrResult (XRAPI_PTR *PFN_xrDestroySceneMSFT)(XrSceneMSFT scene);
typedef XrResult (XRAPI_PTR *PFN_xrComputeNewSceneMSFT)(XrSceneObserverMSFT sceneObserver, const XrNewSceneComputeInfoMSFT* computeInfo);
typedef XrResult (XRAPI_PTR *PFN_xrGetSceneComputeStateMSFT)(XrSceneObserverMSFT sceneObserver, XrSceneComputeStateMSFT* state);
typedef XrResult (XRAPI_PTR *PFN_xrGetSceneComponentsMSFT)(XrSceneMSFT scene, const XrSceneComponentsGetInfoMSFT* getInfo, XrSceneComponentStatesMSFT* components);
typedef XrResult (XRAPI_PTR *PFN_xrLocateSceneComponentsMSFT)(XrSceneMSFT scene, const XrSceneComponentsLocateInfoMSFT* locateInfo, XrSceneComponentLocationsMSFT* locations);
typedef XrResult (XRAPI_PTR *PFN_xrGetSceneMeshBuffersMSFT)(XrSceneMSFT scene, const XrSceneMeshBuffersGetInfoMSFT* getInfo, XrSceneMeshBuffersMSFT* buffers);

#ifndef XR_NO_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSceneComputeFeaturesMSFT(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    featureCapacityInput,
    uint32_t*                                   featureCountOutput,
    XrSceneComputeFeatureMSFT*                  features);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateSceneObserverMSFT(
    XrSession                                   session,
    const XrSceneObserverCreateInfoMSFT*        createInfo,
    XrSceneObserverMSFT*                        sceneObserver);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroySceneObserverMSFT(
    XrSceneObserverMSFT                         sceneObserver);

XRAPI_ATTR XrResult XRAPI_CALL xrCreateSceneMSFT(
    XrSceneObserverMSFT                         sceneObserver,
    const XrSceneCreateInfoMSFT*                createInfo,
    XrSceneMSFT*                                scene);

XRAPI_ATTR XrResult XRAPI_CALL xrDestroySceneMSFT(
    XrSceneMSFT                                 scene);

XRAPI_ATTR XrResult XRAPI_CALL xrComputeNewSceneMSFT(
    XrSceneObserverMSFT                         sceneObserver,
    const XrNewSceneComputeInfoMSFT*            computeInfo);

XRAPI_ATTR XrResult XRAPI_CALL xrGetSceneComputeStateMSFT(
    XrSceneObserverMSFT                         sceneObserver,
    XrSceneComputeStateMSFT*                    state);

XRAPI_ATTR XrResult XRAPI_CALL xrGetSceneComponentsMSFT(
    XrSceneMSFT                                 scene,
    const XrSceneComponentsGetInfoMSFT*         getInfo,
    XrSceneComponentStatesMSFT*                 components);

XRAPI_ATTR XrResult XRAPI_CALL xrLocateSceneComponentsMSFT(
    XrSceneMSFT                                 scene,
    const XrSceneComponentsLocateInfoMSFT*      locateInfo,
    XrSceneComponentLocationsMSFT*              locations);

XRAPI_ATTR XrResult XRAPI_CALL xrGetSceneMeshBuffersMSFT(
    XrSceneMSFT                                 scene,
    const XrSceneMeshBuffersGetInfoMSFT*        getInfo,
    XrSceneMeshBuffersMSFT*                     buffers);
#endif