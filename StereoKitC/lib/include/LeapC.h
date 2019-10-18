/******************************************************************************\
* Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/
#ifndef _LEAP_C_H
#define _LEAP_C_H

#ifndef LEAP_EXPORT
#  ifdef _MSC_VER
#    define LEAP_EXPORT __declspec(dllimport)
#  else
#    define LEAP_EXPORT
#  endif
#endif

#ifdef _MSC_VER
#  define LEAP_CALL __stdcall
#else
#  define LEAP_CALL
#endif

#if defined(__cplusplus) && __cplusplus >= 201103
#  define LEAP_STATIC_ASSERT static_assert
#else
#  define LEAP_STATIC_ASSERT(x, y)
#endif

// Define integer types for Visual Studio 2008 and earlier
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include <stdbool.h>

#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup Functions Functions
 * Functions exported by the LeapC library.
 * \defgroup Enum Enumerations
 * Enumerations used by the LeapC functions and data structures.
 * \defgroup Structs Data Structures
 * Structs passed to and received from LeapC functions.
 */

/** \ingroup Enum
 * Defines the codes returned by all LeapC functions.
 * @since 3.0.0
 */
typedef enum _eLeapRS {
  /**
   * The operation completed successfully.
   */
  eLeapRS_Success                       = 0x00000000,

  /**
   * An undetermined error has occurred.
   * This is usually the result of an abnormal operating condition in LeapC,
   * the Leap Motion service, or the host computer itself.
   * @since 3.0.0
   */
  eLeapRS_UnknownError                  = 0xE2010000,

  /**
   * An invalid argument was specified.
   * @since 3.0.0
   */
  eLeapRS_InvalidArgument               = 0xE2010001,

  /**
   * Insufficient resources existed to complete the request.
   * @since 3.0.0
   */
  eLeapRS_InsufficientResources         = 0xE2010002,

  /**
   * The specified buffer was not large enough to complete the request.
   * @since 3.0.0
   */
  eLeapRS_InsufficientBuffer            = 0xE2010003,

  /**
   * The requested operation has timed out.
   * @since 3.0.0
   */
  eLeapRS_Timeout                       = 0xE2010004,

  /**
   * The operation is invalid because there is no current connection.
   * @since 3.0.0
   */
  eLeapRS_NotConnected                  = 0xE2010005,

  /**
   * The operation is invalid because the connection is not complete.
   * @since 3.0.0
   */
  eLeapRS_HandshakeIncomplete           = 0xE2010006,

  /**
   * The specified buffer size is too large.
   * @since 3.0.0
   */
  eLeapRS_BufferSizeOverflow            = 0xE2010007,

  /**
   * A communications protocol error occurred.
   * @since 3.0.0
   */
  eLeapRS_ProtocolError                 = 0xE2010008,

  /**
   * The server incorrectly specified zero as a client ID.
   * @since 3.0.0
   */
  eLeapRS_InvalidClientID               = 0xE2010009,

  /**
   * The connection to the service was unexpectedly closed while reading or writing a message.
   * The server may have terminated.
   * @since 3.0.0
   */
  eLeapRS_UnexpectedClosed              = 0xE201000A,

  /**
   * The specified request token does not appear to be valid
   *
   * Provided that the token value which identifies the request itself was, at one point, valid, this
   * error condition occurs when the request to which the token refers has already been satisfied or
   * is currently being satisfied.
   * @since 3.0.0
   */
  eLeapRS_UnknownImageFrameRequest      = 0xE201000B,

  /**
   * The specified frame ID is not valid or is no longer valid
   *
   * Provided that frame ID was, at one point, valid, this error condition occurs when the identifier
   * refers to a frame that occurred further in the past than is currently recorded in the rolling
   * frame window.
   * @since 3.0.0
   */
  eLeapRS_UnknownTrackingFrameID        = 0xE201000C,

  /**
   * The specified timestamp references a future point in time
   *
   * The related routine can only operate on time points having occurred in the past, and the
   * provided timestamp occurs in the future.
   * @since 3.1.2
   */
  eLeapRS_RoutineIsNotSeer              = 0xE201000D,

  /**
   * The specified timestamp references a point too far in the past
   *
   * The related routine can only operate on time points occurring within its immediate record of
   * the past.
   * @since 3.1.2
   */
  eLeapRS_TimestampTooEarly             = 0xE201000E,

  /**
   * LeapPollConnection is called concurrently.
   * @since 3.1.2
   */
  eLeapRS_ConcurrentPoll                = 0xE201000F,

  /**
   * A connection to the Leap Motion service could not be established.
   @since 3.0.0
   */
  eLeapRS_NotAvailable                  = 0xE7010002,

  /**
   * The requested operation can only be performed while the device is sending data.
   * @since 3.0.0
   */
  eLeapRS_NotStreaming                  = 0xE7010004,

  /**
   * The specified device could not be opened. It is possible that the device identifier
   * is invalid, or that the device has been disconnected since being enumerated.
   * @since 3.0.0
   */
  eLeapRS_CannotOpenDevice              = 0xE7010005,
} eLeapRS;

/**
 * Evaluates to true if the specified return code is a success code
 * @since 3.1.3
 */
#define LEAP_SUCCEEDED(rs) ((rs & 0x80000000) == 0)

 /**
  * Evaluates to true if the specified return code is a failure code
  * @since 3.1.3
  */
#define LEAP_FAILED(rs) ((rs & 0x80000000) != 0)

/** \ingroup Structs
 * \struct LEAP_CONNECTION
 * A handle to the Leap connection object.
 * Use this handle to specify the connection for an operation.
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION *LEAP_CONNECTION;

/**  \ingroup Structs
 * \struct LEAP_DEVICE
 * A handle to a Leap device object.
 * Use this handle to specify the device for an operation.
 * @since 3.0.0
 */
typedef struct _LEAP_DEVICE *LEAP_DEVICE;

/**  \ingroup Structs
 * Represents a calibration object.
 * Not currently of any particular use.
 * @since 3.0.0
 */
typedef struct _LEAP_CALIBRATION *LEAP_CALIBRATION;

/**  \ingroup Structs
 * A reference to a Leap device.
 *
 * Get a LEAP_DEVICE_REF by calling LeapGetDeviceList(). Access a device by
 * calling LeapOpenDevice() with this reference. LeapOpenDevice() provides a
 * LEAP_DEVICE struct, which is a handle to an open device.
 */
typedef struct _LEAP_DEVICE_REF {
  /** An opaque handle. @since 3.0.0 */
  void* handle;
  /** a generic identifier. @since 3.0.0 */
  uint32_t id;
} LEAP_DEVICE_REF;

/** \ingroup Structs
 * Specifies the configuration for a connection.
 * Currently, there are no externally useful configuration options.
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION_CONFIG {
  /** Set to the final size of the structure. @since 3.0.0 */
  uint32_t size;

  /** The connection configuration flags. (currently there are none. @since 3.0.0 */
  uint32_t flags;

  /*
   * Specifies the server namespace to be used. Leave NULL to use the default namespace.
   *
   * It is possible to launch the service with a different IPC connection namespace
   * (using internal service functions). Clients wishing to connect to a different
   * server namespace may specify that namespace here.
   *
   * The default connection namespace is "Leap Service".
   */
  /** For internal use. @since 3.0.0 */
  const char* server_namespace;
} LEAP_CONNECTION_CONFIG;

/** \ingroup Enum
 * Defines the various types of data that may be allocated using the allocator.
 * @since 4.0.0
 */
typedef enum _eLeapAllocatorType {
  /** Signed 8-bit integer (char) @since 4.0.0 */
  eLeapAllocatorType_Int8 = 0,
  /** Unsigned 8-bit integer (byte) @since 4.0.0 */
  eLeapAllocatorType_Uint8 = 1,
  /** Signed 16-bit integer @since 4.0.0 */
  eLeapAllocatorType_Int16 = 2,
  /** Unsigned 16-bit integer @since 4.0.0 */
  eLeapAllocatorType_UInt16 = 3,
  /** Signed 32-bit integer @since 4.0.0 */
  eLeapAllocatorType_Int32 = 4,
  /** Unsigned 32-bit integer @since 4.0.0 */
  eLeapAllocatorType_UInt32 = 5,
  /** Single-precision 32-bit floating-point @since 4.0.0 */
  eLeapAllocatorType_Float = 6,
  /** Signed 64-bit integer @since 4.0.0 */
  eLeapAllocatorType_Int64 = 8,
  /** Unsigned 64-bit integer @since 4.0.0 */
  eLeapAllocatorType_UInt64 = 9,
  /** Double-precision 64-bit floating-point @since 4.0.0 */
  eLeapAllocatorType_Double = 10,
} eLeapAllocatorType;

/** \ingroup Structs
 * Specifies the allocator/deallocator functions to be used when the library
 * needs to dynamically manage memory.
 * @since 4.0.0
 */
typedef struct LEAP_ALLOCATOR {
  /**
   * Function pointer to an allocator function that is expected to return a
   * pointer to memory of at least the specified size in bytes. This will be
   * called when the library needs a block of memory that will be provided
   * back to the client in a subsequent event or response. A type hint is
   * provided in the case where the underlying buffer type needs to be known
   * at allocation time.
   */
  void* (*allocate)(uint32_t size, eLeapAllocatorType typeHint, void* state);

  /**
   * Function pointer to a deallocator function. The function receives the
   * address of a previously allocated block of memory from the ``allocate``
   * function pointer. The caller is not required to deallocate the memory,
   * but rather this call is used by the library to indicate to the client
   * that it will no longer reference the memory at this address, and that
   * the callee _may_ deallocate the memory when it is ready to do so.
   */
  void (*deallocate)(void* ptr, void* state);

  /**
   * Pointer to state to be passed to the allocate and deallocate functions.
   */
  void* state;
} LEAP_ALLOCATOR;

/** \ingroup Functions
 * Samples the universal clock used by the system to timestamp image and tracking frames.
 *
 * The returned counter value is given in microseconds since an epoch time. The clock used for the
 * counter itself is implementation-defined, but generally speaking, it is global, monotonic, and
 * makes use of the most accurate high-performance counter available on the system.
 * @returns microseconds since an unspecified epoch.
 * @since 3.0.0
 */
LEAP_EXPORT int64_t LEAP_CALL LeapGetNow(void);

/** \ingroup Functions
 * Creates a new LEAP_CONNECTION object.
 *
 * Pass the LEAP_CONNECTION pointer to LeapOpenConnection() to establish a
 * connection to the Leap Motion service; and to subsequent operations
 * on the same connection.
 *
 * @param pConfig The configuration to be used with the newly created connection.
 * If pConfig is null, a connection is created with a default configuration.
 * @param[out] phConnection Receives a pointer to the connection object
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapCreateConnection(const LEAP_CONNECTION_CONFIG* pConfig, LEAP_CONNECTION* phConnection);

/** \ingroup Functions
 * Opens a connection to the service.
 *
 * This routine will not block. A connection to the service will not be established until the first
 * invocation of LeapPollConnection.
 *
 * @param hConnection A handle to the connection object, created by LeapCreateConnection().
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapOpenConnection(LEAP_CONNECTION hConnection);

typedef enum _eLeapServiceDisposition {
  /**
   * The service cannot receive frames fast enough from the underlying hardware.
   * @since 3.1.3
   */
  eLeapServiceState_LowFpsDetected           = 0x00000001,

  /**
   * The service has paused itself due to an insufficient frame rate from the hardware.
   * @since 3.1.3
   */
  eLeapServiceState_PoorPerformancePause     = 0x00000002,

  /**
   * The combination of all valid flags in this enumeration
   */
  eLeapServiceState_ALL = eLeapServiceState_LowFpsDetected | eLeapServiceState_PoorPerformancePause
} eLeapServiceDisposition;

/**  \ingroup Structs
 * Received from LeapPollConnection() when a connection to the Leap Motion service is established.
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION_EVENT {
  /** A combination of eLeapServiceDisposition flags. @since 3.1.3 */
  uint32_t flags;
} LEAP_CONNECTION_EVENT;

/** \ingroup Structs
 * Received from LeapPollConnection() when a connection to the Leap Motion service is lost.
 *
 * If a LeapC function that performs a transaction with the Leap Motion service is called
 * after the connection is lost, the next call to LeapPollConnection() will return
 * this event. Otherwise, it can take up to 5 seconds of polling the connection to
 * receive this event.
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION_LOST_EVENT {
  /** Reserved for future use. @since 3.0.0 */
  uint32_t flags;
} LEAP_CONNECTION_LOST_EVENT;

/** \ingroup Enums
 * The connection status codes.
 * These codes can be read from the LEAP_CONNECTION_INFO struct created by
 * a call to LeapGetConnectionInfo().
 * @since 3.0.0
 */
typedef enum _eLeapConnectionStatus {
  /**
   * The connection is not open.
   * Call LeapOpenConnection() to open a connection to the Leap Motion service.
   * @since 3.0.0
   */
  eLeapConnectionStatus_NotConnected = 0,

  /**
   * The connection is open.
   * @since 3.0.0
   */
  eLeapConnectionStatus_Connected,

  /**
   * Opening the connection is underway, but not complete.
   * @since 3.0.0
   */
  eLeapConnectionStatus_HandshakeIncomplete,

  /**
   * The connection could not be opened because the Leap Motion service does not
   * appear to be running.
   * @since 3.0.0
   */
  eLeapConnectionStatus_NotRunning    = 0xE7030004
} eLeapConnectionStatus;

/** \ingroup Structs
 * Information about a connection.
 *
 * Call LeapCreateConnection() to generate the handle for the connection;
 * call LeapOpenConnection() to establish the connection; then call
 * LeapGetConnectionInfo(), which creates this struct, to check the connection status.
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION_INFO {
  /** The size of this structure. @since 3.0.0 */
  uint32_t size;

  /** The current status of this connection. @since 3.0.0 */
  eLeapConnectionStatus status;
} LEAP_CONNECTION_INFO;

/** \ingroup Functions
 * Retrieves status information about the specified connection.
 *
 * Call LeapCreateConnection() to generate the handle for the connection;
 * call LeapOpenConnection() to establish the connection; then call
 * this function to check the connection status.
 *
 * @param hConnection The handle of the connection of interest. Created by LeapCreateConnection.
 * @param[out] pInfo A pointer to a buffer that receives additional connection information. One input,
 *   the size field of pInfo is the size of the buffer(i.e. the size of a LEAP_CONNECTION_INFO
 *   struct); On output, the size field of pInfo receives the size necessary to hold
 *   the entire information block.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapGetConnectionInfo(LEAP_CONNECTION hConnection, LEAP_CONNECTION_INFO* pInfo);


/**  \ingroup Enum
 * Enumerates flags for the service policies.
 */
typedef enum _eLeapPolicyFlag {
  /** The policy allowing an application to receive frames in the background. @since 3.0.0 */
  eLeapPolicyFlag_BackgroundFrames = 0x00000001,

  /** The policy specifying whether to automatically stream images from the device. @since 4.0.0 */
  eLeapPolicyFlag_Images           = 0x00000002,

  /** The policy specifying whether to optimize tracking for head-mounted device. @since 3.0.0 */
  eLeapPolicyFlag_OptimizeHMD      = 0x00000004,

  /** The policy allowing an application to pause or resume service tracking. @since 3.0.0 */
  eLeapPolicyFlag_AllowPauseResume = 0x00000008,

  /** The policy allowing an application to receive per-frame map points. @since 4.0.0 */
  eLeapPolicyFlag_MapPoints        = 0x00000080,
} eLeapPolicyFlag;

/** \ingroup Structs
 * The response from a request to get or set a policy.
 * LeapPollConnection() creates this struct when the response becomes available.
 * @since 3.0.0
 */
typedef struct _LEAP_POLICY_EVENT {
  /** Reserved for future use. @since 3.0.0 */
  uint32_t reserved;

  /**
  * A bitfield containing the policies effective at the
  * time the policy event was processed. @since 3.0.0
  */
  uint32_t current_policy;
} LEAP_POLICY_EVENT;

/** \ingroup Functions
 * Sets or clears one or more policy flags.
 *
 * Changing policies is asynchronous. After you call this function, a subsequent
 * call to LeapPollConnection provides a LEAP_POLICY_EVENT containing the current
 * policies, reflecting any changes.
 *
 * To get the current policies without changes, specify zero for both the set
 * and clear parameters. When ready, LeapPollConnection() provides the
 * a LEAP_POLICY_EVENT containing the current settings.
 *
 * The eLeapPolicyFlag enumeration defines the policy flags.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param set A bitwise combination of flags to be set. Set to 0 if not setting any flags.
 * @param clear A bitwise combination of flags to be cleared. Set to 0 to if not clearing any flags.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapSetPolicyFlags(LEAP_CONNECTION hConnection, uint64_t set, uint64_t clear);

/** \ingroup Functions
 * Pauses the service
 *
 * Attempts to pause or unpause the service depending on the argument.
 * This is treated as a 'user pause', as though a user had requested a pause through the
 * Leap Control Panel. The connection must have the AllowPauseResume policy set
 * or it will fail with eLeapRS_InvalidArgument.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param pause Set to 'true' to pause, or 'false' to unpause
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 4.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapSetPause(LEAP_CONNECTION hConnection, bool pause);

/** \ingroup Fucntions
 * Sets the allocator functions to use for a particular connection.
 *
 * If user-supplied allocator functions are not supplied, the functions that require
 * dynamic memory allocation will not be available.
 *
 * @param hConnection A handle to the connection object, created by LeapCreateConnection().
 * @param allocator A pointer to a structure containing the allocator functions to be called
 * as needed by the library.
 * @since 4.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapSetAllocator(LEAP_CONNECTION hConnection, const LEAP_ALLOCATOR* allocator);

/**  \ingroup Enum
* Identifies the operative data type of a LEAP_VARIANT struct instance.
* @since 3.0.0
*/
typedef enum _eLeapValueType {
  /** The type is unknown (which is an abnormal condition). @since 3.0.0 */
  eLeapValueType_Unknown,
  /** A boolean value. @since 3.0.0 */
  eLeapValueType_Boolean,
  /** An integer value. @since 3.0.0 */
  eLeapValueType_Int32,
  /** A floating point value. @since 3.0.0 */
  eLeapValueType_Float,
  /** A string value. @since 3.0.0 */
  eLeapValueType_String,
  FORCE_DWORD = 0x7FFFFFFF
} eLeapValueType;
LEAP_STATIC_ASSERT(sizeof(eLeapValueType) == 4, "Incorrect enum size");

/** \ingroup Structs
 * A variant data type used to get and set service configuration values.
 * @since 3.0.0
 */
typedef struct _LEAP_VARIANT {
  /** The active data type in this instance. @since 3.0.0 */
  eLeapValueType type;
  union {
    /** A Boolean value. @since 3.0.0 */
    bool boolValue;
    /** An integer value. @since 3.0.0 */
    int32_t iValue;
    /** A floating point value. @since 3.0.0 */
    float fValue;
    /** A pointer to a string buffer. @since 3.0.0 */
    const char* strValue;
  };
} LEAP_VARIANT;

/** \ingroup Structs
 * Contains the response to a configuration value request.
 * Call LeapRequestConfigValue() to request a service config value. The value is
 * fetched asynchronously since it requires a service transaction. LeapPollConnection()
 * returns this event structure when the request has been processed. Use the requestID
 * value to correlate the response to the originating request.
 * @since 3.0.0
 */
typedef struct _LEAP_CONFIG_RESPONSE_EVENT {
  /** An identifier for correlating the request and response. @since 3.0.0 */
  uint32_t requestID;

  /**
   * The configuration value retrieved from the service. Do not free any memory pointed to by
   * this member. The value held is only valid until the next call to LeapPollConnection().
   * @since 3.0.0
   */
  LEAP_VARIANT value;
} LEAP_CONFIG_RESPONSE_EVENT;

/** \ingroup Structs
 * The result of a configuration change request. Contains a status of true for a
 * successful change.
 * Call LeapSaveConfigValue() to request a service config change. The change is
 * performed asynchronously -- and may fail. LeapPollConnection()
 * returns this event structure when the request has been processed. Use the requestID
 * value to correlate the response to the originating request.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
typedef struct _LEAP_CONFIG_CHANGE_EVENT {
  /** An identifier for correlating the request and response. @since 3.0.0 */
  uint32_t requestID;

  /** The result of the change operation: true on success; false on failure. @since 3.0.0 */
  bool status;
} LEAP_CONFIG_CHANGE_EVENT;

/** \ingroup Functions
 * Causes the client to commit a configuration change to the Leap Motion service.
 *
 * The change is performed asynchronously -- and may fail. LeapPollConnection()
 * returns this event structure when the request has been processed. Use the pRequestID
 * value to correlate the response to the originating request.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param key The key of the configuration to commit.
 * @param value The value of the configuration to commit.
 * @param[out] pRequestID A pointer to a memory location to which the id for this request is written, or nullptr if this value is not needed.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapSaveConfigValue(LEAP_CONNECTION hConnection, const char* key, const LEAP_VARIANT* value, uint32_t* pRequestID);

/** \ingroup Functions
 * Requests the current value of a service configuration setting.
 * The value is fetched asynchronously since it requires a service transaction. LeapPollConnection()
 * returns this event structure when the request has been processed. Use the pRequestID
 * value to correlate the response to the originating request.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param key The key of the configuration to request
 * @param[out] pRequestID A pointer to a memory location to which the id for this request is written.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRequestConfigValue(LEAP_CONNECTION hConnection, const char* key, uint32_t* pRequestID);

/** \ingroup Functions
 * Retrieves a list of Leap Motion devices currently attached to the system.
 *
 * To get the number of connected devices, call this function with the pArray parameter
 * set to null. The number of devices is written to the memory specified by pnArray.
 * Use the device count to create an array of LEAP_DEVICE_REF structs large enough to
 * hold the number of connected devices. Finally, call LeapGetDeviceList() with this
 * array and known count to get the list of Leap devices. A device must be opened with
 * LeapOpenDevice() before device properties can be queried.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param[out] pArray A pointer to an array that LeapC fills with the device list.
 * @param[in,out] pnArray On input, set to the number of elements in pArray; on output,
 * LeapC sets this to the number of valid device handles.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapGetDeviceList(LEAP_CONNECTION hConnection, LEAP_DEVICE_REF* pArray, uint32_t* pnArray);

/** \ingroup Functions
 * Opens a device reference and retrieves a handle to the device.
 *
 * To ensure resources are properly freed, users must call LeapCloseDevice()
 * when finished with the device, even if the retrieved device has problems
 * or cannot stream.
 *
 * @param rDevice A device reference.
 * @param[out] phDevice A pointer that receives the opened device handle.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapOpenDevice(LEAP_DEVICE_REF rDevice, LEAP_DEVICE* phDevice);

/**  \ingroup Enum
 * Flags enumerating Leap device capabilities. @since 3.0.0
 */
enum eLeapDeviceCaps {
  /** The device can send color images. @since 3.0.0 */
  eLeapDeviceCaps_Color      = 0x00000001,
};

/** \ingroup Enum
 * Device hardware types. @since 3.0.0
 */
typedef enum _eLeapDevicePID {
  /** An unknown device. @since 3.1.3 */
  eLeapDevicePID_Unknown         = 0x0000,

  /** The Leap Motion consumer peripheral. @since 3.0.0 */
  eLeapDevicePID_Peripheral      = 0x0003,

  /** Internal research product codename "Dragonfly". @since 3.0.0 */
  eLeapDevicePID_Dragonfly       = 0x1102,

  /** Internal research product codename "Nightcrawler". @since 3.0.0 */
  eLeapDevicePID_Nightcrawler    = 0x1201,

  /** Research product codename "Rigel". @since 4.0.0 */
  eLeapDevicePID_Rigel           = 0x1202,

  /** An invalid device type value. @since 3.1.3 */
  eLeapDevicePID_Invalid = 0xFFFFFFFF
} eLeapDevicePID;

/** \ingroup Structs
 * Properties of a Leap device.
 * Get a LEAP_DEVICE_INFO by calling LeapGetDeviceInfo() with the handle for
 * device. The device must be open.
 * @since 3.0.0
 **/
typedef struct _LEAP_DEVICE_INFO {
  /** Size of this structure. @since 3.0.0 */
  uint32_t size;

  /** A combination of eLeapDeviceStatus flags. @since 3.0.0 */
  uint32_t status;

  /** A combination of eLeapDeviceCaps flags. @since 3.0.0 */
  uint32_t caps;

  /** One of the eLeapDevicePID members. @since 3.0.0 */
  eLeapDevicePID pid;

  /**
   * The device baseline, in micrometers.
   *
   * The baseline is defined as the distance between the center axis of each lens in a stereo camera
   * system. For other camera systems, this value is set to zero.
   * @since 3.0.0
   */
  uint32_t baseline;

  /** The required length of the serial number char buffer including the null character. @since 3.0.0 */
  uint32_t serial_length;

  /** A pointer to the null-terminated device serial number string. @since 3.0.0 */
  char* serial;

  /** The horizontal field of view of this device in radians. @since 3.0.0 */
  float h_fov;

  /** The vertical field of view of this device in radians. @since 3.0.0 */
  float v_fov;

  /** The maximum range for this device, in micrometers. @since 3.0.0 */
  uint32_t range;
} LEAP_DEVICE_INFO;

/** \ingroup Functions
 * Gets device properties.
 *
 * To get the device serial number, you must supply a LEAP_DEVICE_INFO struct whose
 * serial member points to a char array large enough to hold the null-terminated
 * serial number string. To get the required length, call LeapGetDeviceInfo() using
 * a LEAP_DEVICE_INFO struct that has serial set to NULL. LeapC sets serial_length field of
 * the struct to the required length. You can then allocate memory for the string,
 * set the serial field, and call this function again.
 *
 * @param hDevice A handle to the device to be queried.
 * @param[out] info The struct to receive the device property data.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapGetDeviceInfo(LEAP_DEVICE hDevice, LEAP_DEVICE_INFO* info);

/** \ingroup Structs
 * Device event information.
 *
 * LeapPollConnection() produces a message containing this event when a new
 * device is detected. You can use the handle provided by the device filed to
 * open a device so that you can access its properties.
 * @since 3.0.0
 */
typedef struct _LEAP_DEVICE_EVENT {
  /** Reserved for future use. @since 3.0.0 */
  uint32_t flags;

  /** The handle reference of to the newly attached device. @since 3.0.0 */
  LEAP_DEVICE_REF device;

  /** The status of the connected device. A combination of flags from the eLeapDeviceStatus collection. */
  uint32_t status;
} LEAP_DEVICE_EVENT;

/**  \ingroup Enum
 * Enumerates the device status codes.
 * @since 3.0.0
 */
typedef enum _eLeapDeviceStatus {
  /** The device is sending out frames. @since 3.0.0*/
  eLeapDeviceStatus_Streaming      = 0x00000001,

  /** Device streaming has been paused. @since 3.0.0 */
  eLeapDeviceStatus_Paused         = 0x00000002,

  /**
   * There are known sources of infrared interference. Device has transitioned to
   * robust mode in order to compensate.  @since 3.1.3
   */
  eLeapDeviceStatus_Robust         = 0x00000004,

  /** The device's window is smudged, tracking may be degraded.  @since 3.1.3 */
  eLeapDeviceStatus_Smudged        = 0x00000008,

  /** The device has entered low-resource mode. @since 4.0.0 */
  eLeapDeviceStatus_LowResource    = 0x00000010,

  /** The device has failed, but the failure reason is not known. @since 3.0.0 */
  eLeapDeviceStatus_UnknownFailure = 0xE8010000,

  /** The device has a bad calibration record and cannot send frames. @since 3.0.0 */
  eLeapDeviceStatus_BadCalibration = 0xE8010001,

  /** The device reports corrupt firmware or cannot install a required firmware update. @since 3.0.0 */
  eLeapDeviceStatus_BadFirmware    = 0xE8010002,

  /** The device USB connection is faulty. @since 3.0.0 */
  eLeapDeviceStatus_BadTransport   = 0xE8010003,

  /** The device USB control interfaces failed to initialize. @since 3.0.0 */
  eLeapDeviceStatus_BadControl     = 0xE8010004,
} eLeapDeviceStatus;

/** \ingroup Structs
 * Device failure information.
 * LeapPollConnection() produces a message containing this event when a
 * device fails. Only partial information may be available. If hDevice is
 * non-null, then you can use it to identify the failed device with a known,
 * open device.
 * @since 3.0.0
 */
typedef struct _LEAP_DEVICE_FAILURE_EVENT {
  /** The status of this failure event. @since 3.0.0 */
  eLeapDeviceStatus status;

  /**
   * A handle to the device generating this failure event, if available, otherwise NULL.
   *
   * You are not responsible for closing this handle.
   * @since 3.0.0
   */
  LEAP_DEVICE hDevice;
} LEAP_DEVICE_FAILURE_EVENT;

/** \ingroup Structs
 * Identifying information for a frame of tracking data. @since 3.0.0
 */
typedef struct _LEAP_FRAME_HEADER {
  /** Reserved, set to zero. @since 3.0.0 */
  void* reserved;

  /**
   * A unique identifier for this frame
   *
   * All frames carrying this frame ID are part of the same unit of processing. This counter
   * is generally an increasing counter, but may reset to another value if the user stops and
   * restarts streaming.
   *
   * For interpolated frames, this value corresponds to the identifier of the frame upper bound.
   * @since 3.0.0
   */
  int64_t frame_id;

  /**
   * The timestamp for this image, in microseconds, referenced against LeapGetNow().
   * @since 3.0.0
   */
  int64_t timestamp;
} LEAP_FRAME_HEADER;

/** \ingroup Enum
 * Functional image types (not data formats).
 */
typedef enum _eLeapImageType {
  /** An invalid or unknown type. @since 3.0.0 */
  eLeapImageType_UNKNOWN = 0,

  /** Default, processed IR images. @since 3.0.0 */
  eLeapImageType_Default,

  /** Raw images from the device. @since 3.0.0 */
  eLeapImageType_Raw
} eLeapImageType;

/**  \ingroup Enum
 * Image formats.
 * @since 3.0.0
 */
typedef enum _eLeapImageFormat {
  /** An invalid or unknown format. @since 3.0.0 */
  eLeapImageFormat_UNKNOWN = 0,

  /** An infrared image. @since 3.0.0 */
  eLeapImageFormat_IR = 0x317249,

  /** A Bayer RGBIr image with uncorrected RGB channels. @since 3.0.0 */
  eLeapImageFormat_RGBIr_Bayer = 0x49425247,
} eLeapImageFormat;

/**  \ingroup Enum
 * Camera perspective types.
 * @since 3.0.0
 */
typedef enum _eLeapPerspectiveType {
  /** An unknown or invalid type.  @since 3.0.0 */
  eLeapPerspectiveType_invalid = 0,

  /** A canonically left image. @since 3.0.0 */
  eLeapPerspectiveType_stereo_left = 1,

  /** A canonically right image. @since 3.0.0 */
  eLeapPerspectiveType_stereo_right = 2,

  /** Reserved for future use. @since 3.0.0 */
  eLeapPerspectiveType_mono = 3,
} eLeapPerspectiveType;

/** \ingroup Structs
 * Properties of a sensor image.
 * @since 3.0.0
 */
typedef struct _LEAP_IMAGE_PROPERTIES {
  /** The type of this image. @since 3.0.0 */
  eLeapImageType type;

  /** The format of this image. @since 3.0.0 */
  eLeapImageFormat format;

  /** The number of bytes per image pixel. @since 3.0.0 */
  uint32_t bpp;

  /** The number of horizontal pixels in the image. @since 3.0.0 */
  uint32_t width;

  /** The number of rows of pixels in the image. @since 3.0.0 */
  uint32_t height;

  /** Reserved for future use. @since 3.0.0 */
  float x_scale;
  /** Reserved for future use. @since 3.0.0 */
  float y_scale;

  /** Reserved for future use. @since 3.0.0 */
  float x_offset;
  /** Reserved for future use. @since 3.0.0 */
  float y_offset;
} LEAP_IMAGE_PROPERTIES;

#define LEAP_DISTORTION_MATRIX_N 64
/** \ingroup Structs
 * A matrix containing lens distortion correction coordinates.
 *
 * Each point in the grid contains the coordinates of the pixel in the image buffer that
 * contains the data for the pixel in the undistorted image corresponding
 * to that point in the grid.
 * Interpolate between points in the matrix to correct image pixels that don't
 * fall directly underneath a point in the distortion grid.
 *
 * Current devices use a 64x64 point distortion grid.
 * @since 3.0.0
 */
typedef struct _LEAP_DISTORTION_MATRIX {
  /** A point in the distortion grid. @since 3.0.0 */
  struct {
    /** The x-pixel coordinate. @since 3.0.0 */
    float x;
    /** The y-pixel coordinate. @since 3.0.0 */
    float y;
  }
  /** A grid of 2D points. @since 3.0.0 */
  matrix[LEAP_DISTORTION_MATRIX_N][LEAP_DISTORTION_MATRIX_N];
} LEAP_DISTORTION_MATRIX;

/** \ingroup Structs
 * Describes the image to request.
 * Pass this struct to the LeapImageRequest() function.
 * @since 3.0.0
 */
typedef struct _LEAP_IMAGE_FRAME_DESCRIPTION {
  /**
   * The ID of the frame corresponding to the desired image. @since 3.0.0
   */
  int64_t frame_id;

  /**
   * The type of the desired image. @since 3.0.0
   */
  eLeapImageType type;

  /**
   * Length of your image buffer. The buffer must be large enough to
   * hold the request image.
   */
  uint64_t buffer_len;

  /**
   * An allocated buffer large enough to contain the requested image. The buffer
   * must remain valid until the image request completes or fails.
   * @since 3.0.0
   */
  void* pBuffer;
} LEAP_IMAGE_FRAME_DESCRIPTION;

/** \ingroup Structs
 * A three element, floating-point vector.
 * @since 3.0.0
 */
typedef struct _LEAP_VECTOR {
  /** You can access the vector members as either an array or individual float values. */
  union {
    /** The vector as an array. @since 3.0.0 */
    float v[3];
    struct {
      /** The x spatial coordinate. @since 3.0.0 */
      float x;
      /** The y spatial coordinate. @since 3.0.0 */
      float y;
      /** The z spatial coordinate. @since 3.0.0 */
      float z;
    };
  };
} LEAP_VECTOR;

typedef struct _LEAP_MATRIX_3x3 {
  LEAP_VECTOR m[3];
} LEAP_MATRIX_3x3;

/** \ingroup Structs
 * A four element, floating point quaternion. @since 3.1.2
 */
typedef struct _LEAP_QUATERNION {
  union {
    /** The quaternion as an array. @since 3.1.3 */
    float v[4];

    struct {
      /** The x coefficient of the vector portion of the quaternion. @since 3.1.2 */
      float x;
      /** The y coefficient of the vector portion of the quaternion. @since 3.1.2 */
      float y;
      /** The z coefficient of the vector portion of the quaternion. @since 3.1.2 */
      float z;
      /** The scalar portion of the quaternion. @since 3.1.2 */
      float w;
    };
  };
} LEAP_QUATERNION;

/** \ingroup Structs
 * Describes a bone's position and orientation.
 *
 * Bones are members of the LEAP_DIGIT struct.
 * @since 3.0.0
 */
typedef struct _LEAP_BONE {
  /** The base of the bone, closer to the heart. The bones origin. @since 3.0.0 */
  LEAP_VECTOR prev_joint;

  /** The end of the bone, further from the heart. @since 3.0.0 */
  LEAP_VECTOR next_joint;

  /** The average width of the flesh around the bone in millimeters. @since 3.0.0 */
  float width;

  /** Rotation in world space from the forward direction.
   * Convert the quaternion to a matrix to derive the basis vectors.
   * @since 3.1.2
   */
  LEAP_QUATERNION rotation;
} LEAP_BONE;

/** \ingroup Structs
 * Describes the digit of a hand.
 * Digits are members of the LEAP_HAND struct.
 * @since 3.0.0
 */
typedef struct _LEAP_DIGIT {
  /** The Leap identifier of this finger. @since 3.0.0 */
  int32_t finger_id;

  union {
    /** All the bones of a digit as an iterable collection. @since 3.0.0 */
    LEAP_BONE bones[4];

    struct {
      /**
       * The finger bone wholly inside the hand.
       * For thumbs, this bone is set to have zero length and width, an identity basis matrix,
       * and its joint positions are equal.
       * Note that this is anatomically incorrect; in anatomical terms, the intermediate phalange
       * is absent in a real thumb, rather than the metacarpal bone. In the Leap Motion model,
       * however, we use a "zero" metacarpal bone instead for ease of programming.
       * @since 3.0.0
       */
      LEAP_BONE metacarpal;

      /** The phalange extending from the knuckle. @since 3.0.0 */
      LEAP_BONE proximal;

      /** The bone between the proximal phalange and the distal phalange. @since 3.0.0 */
      LEAP_BONE intermediate;

      /** The distal phalange terminating at the finger tip. @since 3.0.0 */
      LEAP_BONE distal;
    };
  };

  /** Reports whether the finger is more or less straight. @since 3.0.0 */
  uint32_t is_extended;
} LEAP_DIGIT;

/** \ingroup Structs
 * Properties associated with the palm of the hand.
 * The Palm is a member of the LEAP_HAND struct.
 * @since 3.0.0
 */
typedef struct _LEAP_PALM {
  /**
   * The center position of the palm in millimeters from the Leap Motion origin.
   * @since 3.0.0
   */
  LEAP_VECTOR position;

  /**
   * The time-filtered and stabilized position of the palm.
   *
   * Smoothing and stabilization is performed in order to make
   * this value more suitable for interaction with 2D content. The stabilized
   * position lags behind the palm position by a variable amount, depending
   * primarily on the speed of movement.
   * @since 3.0.0
   */
  LEAP_VECTOR stabilized_position;

  /**
   * The rate of change of the palm position in millimeters per second.
   * @since 3.0.0
   */
  LEAP_VECTOR velocity;

  /**
   * The normal vector to the palm. If your hand is flat, this vector will
   * point downward, or "out" of the front surface of your palm.
   * @since 3.0.0
   */
  LEAP_VECTOR normal;

  /**
   * The estimated width of the palm when the hand is in a flat position.
   * @since 3.0.0
   */
  float width;

  /**
   * The unit direction vector pointing from the palm position toward the fingers.
   * @since 3.0.0
   */
  LEAP_VECTOR direction;

  /**
   * The quaternion representing the palm's orientation
   * corresponding to the basis {normal x direction, -normal, -direction}
   * @since 3.1.3
   */
  LEAP_QUATERNION orientation;
} LEAP_PALM;

/**  \ingroup Enum
 * The Hand chirality types.
 * Used in the LEAP_HAND struct.
 * @since 3.0.0
 */
typedef enum _eLeapHandType {
  /** A left hand. @since 3.0.0 */
  eLeapHandType_Left,

  /** A right hand. @since 3.0.0 */
  eLeapHandType_Right
} eLeapHandType;

/** \ingroup Structs
 * Describes a tracked hand. @since 3.0.0
 */
typedef struct _LEAP_HAND {
  /**
   * A unique ID for a hand tracked across frames.
   * If tracking of a physical hand is lost, a new ID is assigned when
   * tracking is reacquired.
   * @since 3.0.0
   */
  uint32_t id;

  /**
   * Reserved for future use. @since 3.0.0
   */
  uint32_t flags;

  /**
   * Identifies the chirality of this hand. @since 3.0.0
   */
  eLeapHandType type;

  /**
   * How confident we are with a given hand pose. Not currently used (always 1.0).
   * @since 3.0.0
   */
  float confidence;

  /**
   * The total amount of time this hand has been tracked, in microseconds.
   * @since 3.0.0
   */
  uint64_t visible_time;

  /**
   * The distance between index finger and thumb. @since 3.0.0
   */
  float pinch_distance;

  /**
   * The average angle of fingers to palm. @since 3.0.0
   */
  float grab_angle;

  /**
   * The normalized estimate of the pinch pose.
   * Zero is not pinching; one is fully pinched.
   * @since 3.0.0
   */
  float pinch_strength;

  /**
   * The normalized estimate of the grab hand pose.
   * Zero is not grabbing; one is fully grabbing.
   * @since 3.0.0
   */
  float grab_strength;

  /**
   * Additional information associated with the palm. @since 3.0.0
   */
  LEAP_PALM palm;

  /** The fingers of this hand. @since 3.0.0 */
  union {
    struct {
      /** The thumb. @since 3.0.0 */
      LEAP_DIGIT thumb;
      /** The index finger. @since 3.0.0 */
      LEAP_DIGIT index;
      /** The middle finger. @since 3.0.0 */
      LEAP_DIGIT middle;
      /** The ring finger. @since 3.0.0 */
      LEAP_DIGIT ring;
      /** The pinky finger. @since 3.0.0 */
      LEAP_DIGIT pinky;
    };
    /** The fingers of the hand as an array. @since 3.0.0 */
    LEAP_DIGIT digits[5];
  };

  /**
   * The arm to which this hand is attached.
   * An arm consists of a single LEAP_BONE struct.
   * @since 3.0.0
   */
  LEAP_BONE arm;
} LEAP_HAND;

/** \ingroup Structs
 * A snapshot, or frame of data, containing the tracking data for a single moment in time.
 * The LEAP_FRAME struct is the container for all the tracking data.
 * @since 3.0.0
 */
typedef struct _LEAP_TRACKING_EVENT {
  /** A universal frame identification header. @since 3.0.0 */
  LEAP_FRAME_HEADER info;

  /**
   * An identifier for this tracking frame. This identifier is meant to be monotonically
   * increasing, but values may be skipped if the client application does not poll for messages
   * fast enough. This number also generally increases at the same rate as info.frame_id, but
   * if the server cannot process every image received from the device cameras, the info.frame_id
   * identifier may increase faster.
   * @since 3.0.0
   */
  int64_t tracking_frame_id;

  /** The number of hands tracked in this frame, i.e. the number of elements in
   * the pHands array.
   * @since 3.0.0
   */
  uint32_t nHands;

  /**
   * A pointer to the array of hands tracked in this frame.
   * @since 3.0.0
   */
  LEAP_HAND* pHands;

  /**
   * Current tracking frame rate in hertz.
   *
   * This frame rate is distinct from the image frame rate, which is the rate that images are
   * being read from the device. Depending on host CPU limitations, the tracking frame rate
   * may be substantially less than the device frame rate.
   *
   * This number is generally equal to or less than the device frame rate, but there is one
   * case where this number may be _higher_ than the device frame rate:  When the device rate
   * drops. In this case, the device frame rate will fall sooner than the tracking frame rate.
   *
   * This number is equal to zero if there are not enough frames to estimate frame rate.
   *
   * This number cannot be negative.
   * @since 3.0.0
   */
  float framerate;
} LEAP_TRACKING_EVENT;

/** \ingroup Enum
 * System message severity types. @since 3.0.0
 */
typedef enum _eLeapLogSeverity {
  /** The message severity is not known or was not specified. @since 3.0.0 */
  eLeapLogSeverity_Unknown = 0,
  /** A message about a fault that could render the software or device non-functional. @since 3.0.0 */
  eLeapLogSeverity_Critical,
  /** A message warning about a condition that could degrade device capabilities. @since 3.0.0 */
  eLeapLogSeverity_Warning,
  /** A system status message. @since 3.0.0 */
  eLeapLogSeverity_Information
} eLeapLogSeverity;

/** \ingroup Structs
 * A system log message. @since 3.0.0
 */
typedef struct _LEAP_LOG_EVENT {
  /** The type of message. @since 4.0.0 */
  eLeapLogSeverity severity;
  /**
   * The timestamp of the message in microseconds.
   * Compare with the current values of LeapGetNow() and the system clock to
   * calculate the absolute time of the message.
   * @since 4.0.0
   */
  int64_t timestamp;
  /**
   * A pointer to a null-terminated string containing the current log message.
   * @since 4.0.0
   */
  const char* message;
} LEAP_LOG_EVENT;

typedef struct _LEAP_LOG_EVENTS {
  /** The number of log events being pointed to by the events field.
   * @since 4.0.0
   */
  uint32_t nEvents;

  /** An array of ``nEvent`` LEAP_LOG_EVENT structures.
   * @since 4.0.0
   */
  LEAP_LOG_EVENT* events;
} LEAP_LOG_EVENTS;

/** \ingroup Structs
 * A notification that a device's status has changed. One of these messages is received by the client
 * as soon as the service is connected, or when a new device is attached.
 * @since 3.1.3
 */
typedef struct _LEAP_DEVICE_STATUS_CHANGE_EVENT {
  /** A reference to the device whose status has changed */
  LEAP_DEVICE_REF device;

  /** The last known status of the device. This is a combination of eLeapDeviceStatus flags. @since 3.1.3*/
  uint32_t last_status;

  /** The current status of the device. This is a combination of eLeapDeviceStatus flags. @since 3.1.3*/
  uint32_t status;
} LEAP_DEVICE_STATUS_CHANGE_EVENT;

typedef enum _eLeapDroppedFrameType {
  eLeapDroppedFrameType_PreprocessingQueue,
  eLeapDroppedFrameType_TrackingQueue,
  eLeapDroppedFrameType_Other
} eLeapDroppedFrameType;

typedef struct _LEAP_DROPPED_FRAME_EVENT {
  int64_t frame_id;
  eLeapDroppedFrameType type;
} LEAP_DROPPED_FRAME_EVENT;

typedef struct _LEAP_IMAGE {
  /** The properties of the received image. */
  LEAP_IMAGE_PROPERTIES properties;

  /**
   * A version number for the distortion matrix. When the distortion matrix
   * changes, this number is updated. This number is guaranteed not to repeat
   * for the lifetime of the connection. This version number is also guaranteed
   * to be distinct for each perspective of an image.
   *
   * This value is guaranteed to be nonzero if it is valid.
   *
   * The distortion matrix only changes when the streaming device changes or when the
   * device orientation flips -- inverting the image and the distortion grid.
   * Since building a matrix to undistort an image can be a time-consuming task,
   * you can optimize the process by only rebuilding this matrix (or whatever
   * data type you use to correct image distortion) when the grid actually changes.
   */
  uint64_t matrix_version;

  /** Pointers to the camera's distortion matrix. */
  LEAP_DISTORTION_MATRIX* distortion_matrix;

  /** A pointer to the image data. */
  void* data;

  /** Offset, in bytes, from the beginning of the data ptr to the actual beginning of the image data */
  uint32_t offset;

} LEAP_IMAGE;

/** \ingroup Structs
 * A notification that a device's point mapping has changed.
 * @since 4.0.0
 */
typedef struct _LEAP_POINT_MAPPING_CHANGE_EVENT {
  /** The ID of the frame corresponding to the source of the currently tracked points. @since 4.0.0 */
  int64_t frame_id;
  /** The timestamp of the frame, in microseconds, referenced against LeapGetNow(). @since 4.0.0 */
  int64_t timestamp;
  /** The number of points being tracked. @since 4.0.0 */
  uint32_t nPoints;
} LEAP_POINT_MAPPING_CHANGE_EVENT;

/** \ingroup Structs
 * A notification that a device's point mapping has changed.  It contains
 * the entire set of points being mapped.
 * @since 4.0.0
 */
typedef struct _LEAP_POINT_MAPPING {
  /** The ID of the frame corresponding to the source of the currently tracked points. @since 4.0.0 */
  int64_t frame_id;
  /** The timestamp of the frame, in microseconds, referenced against LeapGetNow(). @since 4.0.0 */
  int64_t timestamp;
  /** The number of points being tracked. @since 4.0.0 */
  uint32_t nPoints;
  /** The 3D points being mapped. @since 4.0.0 */
  LEAP_VECTOR* pPoints;
  /** The IDs of the 3D points being mapped. @since 4.0.0 */
  uint32_t* pIDs;
} LEAP_POINT_MAPPING;

typedef struct _LEAP_HEAD_POSE_EVENT {
  /**
  * The timestamp for this image, in microseconds, referenced against LeapGetNow().
  * @since 3.2.1
  */
  int64_t timestamp;
  /**
  * The position and orientation of the user's head. Positional tracking must be enabled.
  * @since 3.2.1
  */
  LEAP_VECTOR head_position;
  LEAP_QUATERNION head_orientation;
} LEAP_HEAD_POSE_EVENT;

/** \ingroup Structs
 * Streaming stereo image pairs from the device.
 *
 * LeapPollConnection() produces this message when an image is available.
 * The struct contains image properties, the distortion grid, and a pointer to
 * the buffer containing the image data -- which was allocated using the allocator
 * function passed to LeapC using the LeapSetAllocator.
 * @since 4.0.0
 */
typedef struct _LEAP_IMAGE_EVENT {
  /** The information header identifying the images tracking frame. */
  LEAP_FRAME_HEADER info;

  /** The left and right images. */
  LEAP_IMAGE image[2];

  /** For internal use only. */
  LEAP_CALIBRATION calib;

} LEAP_IMAGE_EVENT;

/** \ingroup Enum
 * The types of event messages resulting from calling LeapPollConnection().
 * @since 3.0.0
 */
typedef enum _eLeapEventType {
  /**
   * No event has occurred within the timeout period specified when calling LeapPollConnection().
   * @since 3.0.0
   */
  eLeapEventType_None = 0,

  /**
   * A connection to the Leap Motion service has been established.
   * @since 3.0.0
   */
  eLeapEventType_Connection,

  /**
   * The connection to the Leap Motion service has been lost.
   * @since 3.0.0
   */
  eLeapEventType_ConnectionLost,

  /**
   * A device has been detected or plugged-in.
   * A device event is dispatched after a connection is established for any
   * devices already plugged in. (The system currently only supports one
   * streaming device at a time.)
   * @since 3.0.0
   */
  eLeapEventType_Device,

  /**
   * A device has failed.
   * Device failure could be caused by hardware failure, USB controller issues, or
   * other system instability. Note that unplugging a device generates an
   * eLeapEventType_DeviceLost event message, not a failure message.
   * @since 3.0.0
   */
  eLeapEventType_DeviceFailure,

  /**
   * A policy change has occurred.
   * This can be due to setting a policy with LeapSetPolicyFlags() or due to changing
   * or policy-related config settings, including images_mode.
   * (A user can also change these policies using the Leap Motion Control Panel.)
   * @since 3.0.0
   */
  eLeapEventType_Policy,

  /**
   * A tracking frame. The message contains the tracking data for the frame.
   * @since 3.0.0
   */
  eLeapEventType_Tracking = 0x100,

  /**
   * The request for an image has failed.
   * The message contains information about the failure. The client application
   * will not receive the requested image set.
   * @since 3.0.0
   */
  eLeapEventType_ImageRequestError,

  /**
  * The request for an image is complete.
  * The image data has been completely written to the application-provided
  * buffer.
  * @since 3.0.0
  */
  eLeapEventType_ImageComplete,

  /**
   * A system message. @since 3.0.0
   */
  eLeapEventType_LogEvent,

  /**
   * The device connection has been lost.
   *
   * This event is generally asserted when the device has been detached from the system, when the
   * connection to the service has been lost, or if the device is closed while streaming. Generally,
   * any event where the system can conclude no further frames will be received will result in this
   * message. The DeviceEvent field will be filled with the id of the formerly attached device.
   * @since 3.0.0
   */
  eLeapEventType_DeviceLost,

  /**
   * The asynchronous response to a call to LeapRequestConfigValue().
   * Contains the value of requested configuration item.
   * @since 3.0.0
   */
  eLeapEventType_ConfigResponse,

  /**
   * The asynchronous response to a call to LeapSaveConfigValue().
   * Reports whether the change succeeded or failed.
   * @since 3.0.0
   */
  eLeapEventType_ConfigChange,

  /**
   * Notification that a status change has been detected on an attached device
   *
   * @since 3.1.3
   */
  eLeapEventType_DeviceStatusChange,
  eLeapEventType_DroppedFrame,

  /**
   * Notification that an unrequested stereo image pair is available
   *
   * @since 4.0.0
   */
  eLeapEventType_Image,

  /**
   * Notification that point mapping has changed
   *
   * @since 4.0.0
   */
  eLeapEventType_PointMappingChange,

  /**
   * An array of system messages. @since 4.0.0
   */
  eLeapEventType_LogEvents,
  eLeapEventType_HeadPose
} eLeapEventType;

/** \ingroup Structs
 * Defines a basic message from the LeapC message queue.
 * Set by calling LeapPollConnection().
 * @since 3.0.0
 */
typedef struct _LEAP_CONNECTION_MESSAGE {
  /**
   * The size of this message struct. @since 3.0.0
   */
  uint32_t size;

  /**
   * The message type. @since 3.0.0
   */
  eLeapEventType type;

  /**
   * A pointer to the event data for the current type of message. @since 3.0.0
   */
  union {
    /** An untyped pointer. @since 3.0.0 */
    const void* pointer;
    /** A connection message. @since 3.0.0 */
    const LEAP_CONNECTION_EVENT* connection_event;
    /** A connection lost. @since 3.0.0 */
    const LEAP_CONNECTION_LOST_EVENT* connection_lost_event;
    /** A device detected message. @since 3.0.0 */
    const LEAP_DEVICE_EVENT* device_event;
    /** A device's status has changed.  @since 3.1.3 */
    const LEAP_DEVICE_STATUS_CHANGE_EVENT* device_status_change_event;
    /** A policy message. @since 3.0.0 */
    const LEAP_POLICY_EVENT* policy_event;
    /** A device failure message. @since 3.0.0 */
    const LEAP_DEVICE_FAILURE_EVENT* device_failure_event;
    /** A tracking message. @since 3.0.0 */
    const LEAP_TRACKING_EVENT* tracking_event;
    /** A log message. @since 3.0.0 */
    const LEAP_LOG_EVENT* log_event;
    /** A log messages. @since 4.0.0 */
    const LEAP_LOG_EVENTS* log_events;
    /** A get config value message. @since 3.0.0 */
    const LEAP_CONFIG_RESPONSE_EVENT* config_response_event;
    /** A set config value message. @since 3.0.0 */
    const LEAP_CONFIG_CHANGE_EVENT* config_change_event;
    const LEAP_DROPPED_FRAME_EVENT* dropped_frame_event;
    /** A streaming image message. @since 4.0.0 */
    const LEAP_IMAGE_EVENT* image_event;
    /** A point mapping message. @since 4.0.0 */
    const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event;
    const LEAP_HEAD_POSE_EVENT* head_pose_event;
  };
} LEAP_CONNECTION_MESSAGE;

/** \ingroup Functions
 * Polls the connection for a new event.
 *
 * The specific types of events that may be received are not configurable in this entrypoint. Configure
 * the device or connection object directly to change what events will be received.
 *
 * Pointers in the retrieved event message structure will be valid until the associated connection or device is
 * closed, or the next call to LeapPollConnection().
 *
 * Calling this method concurrently will return eLeapRS_ConcurrentCall.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param timeout The maximum amount of time to wait, in milliseconds. If this value is zero,
 * the evt pointer references the next queued message, if there is one, and returns immediately.
 * @param[out] evt A pointer to a structure that is filled with event information. This structure will be valid
 * as long as the LEAP_CONNECTION object is valid.
 *
 * @returns The operation result code, a member of the eLeapRS enumeration. If the operation
 * times out, this method will return eLeapRS_Timeout. The evt pointer will reference a
 * message of type eLeapEventType_None.
 * @since 3.0.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapPollConnection(LEAP_CONNECTION hConnection, uint32_t timeout, LEAP_CONNECTION_MESSAGE* evt);

/** \ingroup Functions
 * Retrieves the number of bytes required to allocate an interpolated frame at the specified time.
 *
 * Use this function to determine the size of the buffer to allocate when calling
 * LeapInterpolateFrame().
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param timestamp The timestamp of the frame whose size is to be queried.
 * @param[out] pncbEvent A pointer that receives the number of bytes required to store the specified frame.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.1.1
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapGetFrameSize(LEAP_CONNECTION hConnection, int64_t timestamp, uint64_t* pncbEvent);

/** \ingroup Functions
 * Constructs a frame at the specified timestamp by interpolating between measured
 * frames.
 *
 * Caller is responsible for allocating a buffer large enough to hold the data of the frame.
 * Use LeapGetFrameSize() to calculate the minimum size of this buffer.
 *
 * Use LeapCreateClockRebaser(), LeapUpdateRebase(), and LeapRebaseClock() to
 * synchronize time measurements in the application with time measurements in
 * the Leap Motion service. This process is required to achieve accurate, smooth
 * interpolation.
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param timestamp The timestamp at which to interpolate the frame data.
 * @param[out] pEvent A pointer to a flat buffer which is filled with an interpolated frame.
 * @param ncbEvent The number of bytes pointed to by pEvent.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.1.1
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapInterpolateFrame(LEAP_CONNECTION hConnection, int64_t timestamp, LEAP_TRACKING_EVENT* pEvent, uint64_t ncbEvent);

/** \ingroup Functions
* Constructs a frame at the specified timestamp by interpolating between a frame near the timestamp
* and a frame near the sourceTimestamp.
*
* Caller is responsible for allocating a buffer large enough to hold the data of the frame.
* Use LeapGetFrameSize() to calculate the minimum size of this buffer.
*
* Use LeapCreateClockRebaser(), LeapUpdateRebase(), and LeapRebaseClock() to
* synchronize time measurements in the application with time measurements in
* the Leap Motion service. This process is required to achieve accurate, smooth
* interpolation.
* @param hConnection The connection handle created by LeapCreateConnection().
* @param timestamp The timestamp to which to interpolate the frame data.
* @param sourceTimestamp The timestamp of the beginning frame from which to interpolate the frame data.
* @param[out] pEvent A pointer to a flat buffer which is filled with an interpolated frame.
* @param ncbEvent The number of bytes pointed to by pEvent.
* @returns The operation result code, a member of the eLeapRS enumeration.
* @since 3.1.1
*/
LEAP_EXPORT eLeapRS LEAP_CALL LeapInterpolateFrameFromTime(LEAP_CONNECTION hConnection, int64_t timestamp, int64_t sourceTimestamp, LEAP_TRACKING_EVENT* pEvent, uint64_t ncbEvent);

/** \ingroup Functions
* Gets the head tracking pose at the specified timestamp by interpolating between measured
* frames.
*
* Caller is responsible for allocating a buffer large enough to hold the data of the frame.
* Use LeapGetFrameSize() to calculate the minimum size of this buffer.
*
* Use LeapCreateClockRebaser(), LeapUpdateRebase(), and LeapRebaseClock() to
* synchronize time measurements in the application with time measurements in
* the Leap Motion service. This process is required to achieve accurate, smooth
* interpolation.
* @param hConnection The connection handle created by LeapCreateConnection().
* @param timestamp The timestamp at which to interpolate the frame data.
* @param[out] pEvent A pointer to a flat buffer which is filled with an interpolated frame.
* @returns The operation result code, a member of the eLeapRS enumeration.
* @since 3.1.1
*/
LEAP_EXPORT eLeapRS LEAP_CALL LeapInterpolateHeadPose(LEAP_CONNECTION hConnection, int64_t timestamp, LEAP_HEAD_POSE_EVENT* pEvent);

/** \ingroup Functions
 * Closes a device handle previously opened with LeapOpenDevice.
 *
 * @param hDevice The device handle to close.
 * @since 3.0.0
 */
LEAP_EXPORT void LEAP_CALL LeapCloseDevice(LEAP_DEVICE hDevice);

/** \ingroup Functions
 * Closes a previously opened connection.
 *
 * This method closes the specified connection object if it is opened
 *
 * This method never fails.
 *
 * @param hConnection A handle to the connection object to be closed.
 * @since 4.0.0
 */
LEAP_EXPORT void LEAP_CALL LeapCloseConnection(LEAP_CONNECTION hConnection);

/** \ingroup Functions
 * Destroys a previously opened connection.
 *
 * This method closes the specified connection object if it is opened, destroys the underlying
 * object, and releases all resources associated with it.
 *
 * This method never fails.
 *
 * Be sure that no other functions are accessing the connection when this function is called.
 *
 * @param hConnection A handle to the connection object to be destroyed.
 * @since 3.0.0
 */
LEAP_EXPORT void LEAP_CALL LeapDestroyConnection(LEAP_CONNECTION hConnection);

/** \ingroup Structs
 * \struct LEAP_CLOCK_REBASER
 * An opaque clock rebase state structure. @since 3.1.2
 */
typedef struct _LEAP_CLOCK_REBASER *LEAP_CLOCK_REBASER;

/** \ingroup Functions
 * Initializes a new Leap clock-rebaser handle object.
 *
 * Pass the filled-in LEAP_CLOCK_REBASER object to calls to LeapUpdateRebase(),
 * LeapRebaseClock(), and LeapDestroyClockRebaser().
 *
 * @param[out] phClockRebaser The pointer to the clock-rebaser object to be initialized.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.1.2
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapCreateClockRebaser(LEAP_CLOCK_REBASER* phClockRebaser);

/** \ingroup Functions
 * Updates the relationship between the Leap Motion clock and the user clock.
 *
 * When using LeapInterpolateFrame(), call this function for every graphics frame
 * rendered by your application. The function should be called as close to the
 * actual point of rendering as possible.
 *
 * The relationship between the application clock and the Leap Motion clock is
 * neither fixed nor stable. Simulation restarts can cause user clock values to change
 * instantaneously. Certain systems simulate slow motion, or respond to heavy load, by reducing the tick rate
 * of the user clock. As a result, the LeapUpdateRebase() function must be called for every rendered frame.
 *
 * @param hClockRebaser The handle to a rebaser object created by LeapCreateClockRebaser().
 * @param userClock A clock value supplied by the application, sampled at about the same time as LeapGetNow() was sampled.
 * @param leapClock The Leap Motion clock value sampled by a call to LeapGetNow().
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.1.2
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapUpdateRebase(LEAP_CLOCK_REBASER hClockRebaser, int64_t userClock, int64_t leapClock);

/** \ingroup Functions
 * Computes the Leap Motion clock corresponding to a specified application clock value.
 *
 * Use this function to translate your application clock to the Leap Motion clock
 * when interpolating frames. LeapUpdateRebase() must be called for every rendered
 * frame for the relationship between the two clocks to remain synchronized.
 *
 * @param hClockRebaser The handle to a rebaser object created by LeapCreateClockRebaser().
 * @param userClock The clock in microseconds referenced to the application clock.
 * @param[out] pLeapClock The corresponding Leap Motion clock value.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.1.2
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRebaseClock(LEAP_CLOCK_REBASER hClockRebaser, int64_t userClock, int64_t* pLeapClock);

/** \ingroup Functions
 * Destroys a previously created clock-rebaser object.
 *
 * This method destroys the specified clock-rebaser object, and releases all resources associated with it.
 *
 * @param hClockRebaser A handle to the clock-rebaser object to be destroyed.
 * @since 3.1.2
 */
LEAP_EXPORT void LEAP_CALL LeapDestroyClockRebaser(LEAP_CLOCK_REBASER hClockRebaser);

/** \ingroup Functions
 * Provides the corrected camera ray intercepting the specified point on the image.
 *
 * Given a point on the image, ``LeapPixelToRectilinear()`` corrects for camera distortion
 * and returns the true direction from the camera to the source of that image point
 * within the Leap Motion field of view.
 *
 * This direction vector has an x and y component [x, y, 1], with the third element
 * always 1. Note that this vector uses the 2D camera coordinate system
 * where the x-axis parallels the longer (typically horizontal) dimension and
 * the y-axis parallels the shorter (vertical) dimension. The camera coordinate
 * system does not correlate to the 3D Leap Motion coordinate system.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param camera The camera to use, a member of the eLeapPerspectiveType enumeration
 * @param pixel A Vector containing the position of a pixel in the image.
 * @returns A Vector containing the ray direction (the z-component of the vector is always 1).
 * @since 3.1.3
 */
LEAP_EXPORT LEAP_VECTOR LEAP_CALL LeapPixelToRectilinear(LEAP_CONNECTION hConnection, eLeapPerspectiveType camera, LEAP_VECTOR pixel);

/** \ingroup Functions
 * Provides the point in the image corresponding to a ray projecting
 * from the camera.
 *
 * Given a ray projected from the camera in the specified direction, ``LeapRectilinearToPixel()``
 * corrects for camera distortion and returns the corresponding pixel
 * coordinates in the image.
 *
 * The ray direction is specified in relationship to the camera. The first
 * vector element is the tangent of the "horizontal" view angle; the second
 * element is the tangent of the "vertical" view angle.
 *
 * The ``LeapRectilinearToPixel()`` function returns pixel coordinates outside of the image bounds
 * if you project a ray toward a point for which there is no recorded data.
 *
 * ``LeapRectilinearToPixel()`` is typically not fast enough for realtime distortion correction.
 * For better performance, use a shader program executed on a GPU.
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param camera The camera to use, a member of the eLeapPerspectiveType enumeration
 * @param rectilinear A Vector containing the ray direction.
 * @returns A Vector containing the pixel coordinates [x, y, 1] (with z always 1).
 * @since 3.1.3
 */
LEAP_EXPORT LEAP_VECTOR LEAP_CALL LeapRectilinearToPixel(LEAP_CONNECTION hConnection, eLeapPerspectiveType camera, LEAP_VECTOR rectilinear);

/** \ingroup Functions
 * Returns an OpenCV-compatible camera matrix
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param camera The camera to use, a member of the eLeapPerspectiveType enumeration
 * @param[out] dest A pointer to a single-precision float array of size 9
 * @since 3.2.1
 */
LEAP_EXPORT void LEAP_CALL LeapCameraMatrix(LEAP_CONNECTION hConnection, eLeapPerspectiveType camera, float* dest);

/** \ingroup Functions
 * Returns an OpenCV-compatible lens distortion using the 8-parameter rational model
 *
 * The order of the returned array is: [k1, k2, p1, p2, k3, k4, k5, k6]
 *
 * @param hConnection The connection handle created by LeapCreateConnection().
 * @param camera The camera to use, a member of the eLeapPerspectiveType enumeration
 * @param[out] dest A pointer to a single-precision float array of size 8.
 * @since 3.2.1
 */
LEAP_EXPORT void LEAP_CALL LeapDistortionCoeffs(LEAP_CONNECTION hConnection, eLeapPerspectiveType camera, float* dest);

/** \ingroup Functions
 * Provides the human-readable canonical name of the specified device model.
 *
 * This method is guaranteed to never return null for the LEAP_DEVICE_INFO.pid field
 * returned by a successful call to LeapGetDeviceInfo
 *
 * @param pid The pid of the device
 * @returns The string name of the device model, or null if the device type string is invalid.
 *
 */
LEAP_EXPORT const char* LEAP_CALL LeapDevicePIDToString(eLeapDevicePID pid);

LEAP_EXPORT eLeapRS LEAP_CALL LeapGetPointMappingSize(LEAP_CONNECTION hConnection, uint64_t* pSize);
LEAP_EXPORT eLeapRS LEAP_CALL LeapGetPointMapping(LEAP_CONNECTION hConnection, LEAP_POINT_MAPPING* pointMapping, uint64_t* pSize);

/** \ingroup Enum
  * Defines the recording mode provided to the LeapRecordingOpen()
  * function. Also used in members of LEAP_RECORDING_PARAMETERS and LEAP_RECORDING_STATUS.
  * @since 3.2.0
  */
typedef enum _eLeapRecordingFlags {
  eLeapRecordingFlags_Error = 0x00000000,
  eLeapRecordingFlags_Reading = 0x00000001,
  eLeapRecordingFlags_Writing = 0x00000002,
  eLeapRecordingFlags_Flushing = 0x00000004,
  eLeapRecordingFlags_Compressed = 0x00000008
} eLeapRecordingFlags;

/** \ingroup Structs
  * A Leap recording.
  * @since 3.2.0
  */
typedef struct _LEAP_RECORDING *LEAP_RECORDING;

/**  \ingroup Structs
  * Read/Write mode for opening a LEAP_RECORDING.
  * since 3.2.0
  */
typedef struct _LEAP_RECORDING_PARAMETERS {
  /** A combination of eLeapRecordingFlags indicating the desired operations. @since 3.2.0 */
  uint32_t mode;
} LEAP_RECORDING_PARAMETERS;

/**  \ingroup Structs
  * Information about a current LEAP_RECORDING.
  * Filled in by a call to LeapRecordingGetStatus().
  * @since 3.2.0
  */
typedef struct _LEAP_RECORDING_STATUS {
  /** Some combination of eLeapRecordingFlags indicating the status of the recording. @since 3.2.0 */
  uint32_t mode;
} LEAP_RECORDING_STATUS;

/** \ingroup Functions
  * Opens or creates a LEAP_RECORDING.
  *
  * Pass the LEAP_RECORDING pointer to LeapRecordingOpen() to initiate reading from
  * or writing to a recording. The recording path is relative to the "user path"
  * which is the SD card on Android.
  *
  * @param ppRecording The recording being opened.
  * @param filePath The file path. This will be passed directly to the OS without modification. An ".lmt" suffix is suggested.
  * @param params The LEAP_RECORDING_PARAMETERS describing what operations are requested.
  * @returns The operation result code, a member of the eLeapRS enumeration.
  * @since 3.2.0
  */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingOpen(LEAP_RECORDING* ppRecording, const char* filePath, LEAP_RECORDING_PARAMETERS params);

/** \ingroup Functions
  * Closes a LEAP_RECORDING.
  *
  * @param[out] ppRecording The recording being closed. Will modify *ppRecording to be null.
  * @returns The operation result code, a member of the eLeapRS enumeration.
  * @since 3.2.0
  */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingClose(LEAP_RECORDING* ppRecording);

/** \ingroup Functions
  * Fills in a LEAP_RECORDING_STATUS struct for an open recording.
  * This struct provides the applicable eLeapRecordingFlags.
  *
  * @param pRecording The open recording.
  * @param[out] pstatus A LEAP_RECORDING_STATUS struct to receive the recording status.
  * @returns The operation result code, a member of the eLeapRS enumeration.
  * @since 3.2.0
  */

LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingGetStatus(LEAP_RECORDING pRecording, LEAP_RECORDING_STATUS* pstatus);

/** \ingroup Functions
 * Retrieves the number of bytes required to allocate the next frame in a recording.
 *
 * Use this function to determine the size of the buffer to allocate before calling
 * LeapRecordingRead().
 *
 * @param pRecording The recording being read from.
 * @param[out] pncbEvent A pointer that receives the number of bytes required to store the next frame.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.2.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingReadSize(LEAP_RECORDING pRecording, uint64_t* pncbEvent);

/** \ingroup Functions
 * Reads a tracking frame from a LEAP_RECORDING file.
 *
 * Caller is responsible for allocating a buffer large enough to hold the data of the frame.
 * Use LeapGetFrameSize() to calculate the minimum size of this buffer.
 *
 * @param pRecording The recording being read from.
 * @param[out] pEvent A pointer to a flat buffer which is filled with the next recorded frame.
 * @param ncbEvent The number of bytes pointed to by pEvent.
 * @returns The operation result code, a member of the eLeapRS enumeration.
 * @since 3.2.0
 */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingRead(LEAP_RECORDING pRecording, LEAP_TRACKING_EVENT* pEvent, uint64_t ncbEvent);

/** \ingroup Functions
  * Writes a tracking frame to a LEAP_RECORDING file.
  *
  * @param pRecording The recording being written to.
  * @param[out] pEvent A pointer to a flat buffer which is filled with an interpolated frame.
  * @param pnBytesWritten If non-null the number of bytes written.
  * @returns The operation result code, a member of the eLeapRS enumeration.
  * @since 3.2.0
  */
LEAP_EXPORT eLeapRS LEAP_CALL LeapRecordingWrite(LEAP_RECORDING pRecording, LEAP_TRACKING_EVENT* pEvent, uint64_t* pnBytesWritten);

typedef struct _LEAP_TELEMETRY_DATA {
  uint32_t thread_id;
  uint64_t start_time;
  uint64_t end_time;
  uint32_t zone_depth;
  const char* file_name;
  uint32_t line_number;
  const char* zone_name;
} LEAP_TELEMETRY_DATA;

LEAP_EXPORT eLeapRS LEAP_CALL LeapTelemetryProfiling(LEAP_CONNECTION hConnection, const LEAP_TELEMETRY_DATA* telemetryData);

LEAP_EXPORT uint64_t LEAP_CALL LeapTelemetryGetNow();

#ifdef __cplusplus
}
#endif

#pragma pack()

#endif
