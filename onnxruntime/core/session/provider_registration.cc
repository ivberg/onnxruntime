// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <string>

#include "core/common/common.h"
#include "core/framework/error_code_helper.h"
#include "core/session/onnxruntime_c_api.h"
#include "core/session/ort_apis.h"

/**
 * Implementation of OrtApis functions for provider registration.
 *
 * EPs that use the provider bridge are handled in provider_bridge_ort.cc
 */
#if defined(USE_XNNPACK)
#include "core/providers/xnnpack/xnnpack_provider_factory.h"

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_Xnnpack,
                    _In_ OrtSessionOptions* options,
                    _In_ const OrtProviderOptions* provider_options) {
  API_IMPL_BEGIN
  return OrtSessionOptionsAppendExecutionProvider_Xnnpack(options, provider_options);
  API_IMPL_END
}
#endif

/**
 * Stubs for the publicly exported static registration functions for EPs that are referenced in the C# bindings.
 *
 * These are required when building an iOS app using Xamarin as all external symbols must be defined at compile time.
 * In that case a static ORT library is used and the symbol needs to exist but doesn't need to be publicly exported.
 * TODO: Not sure if we need to purely limit to iOS builds, so limit to __APPLE__ for now
 */
#if defined(__APPLE__) || defined(ORT_MINIMAL_BUILD)
static OrtStatus* CreateNotEnabledStatus(const std::string& ep) {
  return OrtApis::CreateStatus(ORT_FAIL, (ep + " execution provider is not enabled in this build. ").c_str());
}
#endif

#ifdef __APPLE__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_DML
ORT_API_STATUS_IMPL(OrtSessionOptionsAppendExecutionProvider_DML, _In_ OrtSessionOptions* options, int device_id) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(device_id);
  return CreateNotEnabledStatus("DML");
}
#endif

#ifndef USE_MIGRAPHX
ORT_API_STATUS_IMPL(OrtSessionOptionsAppendExecutionProvider_MIGraphX,
                    _In_ OrtSessionOptions* options, int device_id) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(device_id);
  return CreateNotEnabledStatus("MIGraphX");
}
#endif

#ifndef USE_NNAPI
ORT_API_STATUS_IMPL(OrtSessionOptionsAppendExecutionProvider_Nnapi,
                    _In_ OrtSessionOptions* options, uint32_t nnapi_flags) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(nnapi_flags);
  return CreateNotEnabledStatus("NNAPI");
}
#endif

#ifndef USE_NUPHAR
ORT_API_STATUS_IMPL(OrtSessionOptionsAppendExecutionProvider_Nuphar,
                    _In_ OrtSessionOptions* options, int allow_unaligned_buffers, _In_ const char* settings) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(allow_unaligned_buffers);
  ORT_UNUSED_PARAMETER(settings);
  return CreateNotEnabledStatus("Nuphar");
}
#endif

#ifndef USE_TVM
ORT_API_STATUS_IMPL(OrtSessionOptionsAppendExecutionProvider_Tvm,
                    _In_ OrtSessionOptions* options, _In_ const char* settings) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(settings);
  return CreateNotEnabledStatus("Tvm");
}
#endif

#ifdef __cplusplus
}
#endif

#endif  // __APPLE__

/**
 * Stubs EP functions from OrtApis that are implemented in provider_bridge_ort.cc in a full build.
 * That file is not included in a minimal build.
 */
#if defined(ORT_MINIMAL_BUILD)
ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_CUDA,
                    _In_ OrtSessionOptions* options, _In_ const OrtCUDAProviderOptions* provider_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(provider_options);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_CUDA_V2,
                    _In_ OrtSessionOptions* options, _In_ const OrtCUDAProviderOptionsV2* cuda_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(cuda_options);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::CreateCUDAProviderOptions, _Outptr_ OrtCUDAProviderOptionsV2** out) {
  ORT_UNUSED_PARAMETER(out);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::UpdateCUDAProviderOptions,
                    _Inout_ OrtCUDAProviderOptionsV2* cuda_options,
                    _In_reads_(num_keys) const char* const* provider_options_keys,
                    _In_reads_(num_keys) const char* const* provider_options_values,
                    size_t num_keys) {
  ORT_UNUSED_PARAMETER(cuda_options);
  ORT_UNUSED_PARAMETER(provider_options_keys);
  ORT_UNUSED_PARAMETER(provider_options_values);
  ORT_UNUSED_PARAMETER(num_keys);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::GetCUDAProviderOptionsAsString, _In_ const OrtCUDAProviderOptionsV2* cuda_options, _Inout_ OrtAllocator* allocator,
                    _Outptr_ char** ptr) {
  ORT_UNUSED_PARAMETER(cuda_options);
  ORT_UNUSED_PARAMETER(allocator);
  ORT_UNUSED_PARAMETER(ptr);
  return CreateStatus(ORT_FAIL, "CUDA execution provider is not enabled in this build.");
}

ORT_API(void, OrtApis::ReleaseCUDAProviderOptions, _Frees_ptr_opt_ OrtCUDAProviderOptionsV2* ptr) {
  ORT_UNUSED_PARAMETER(ptr);
}

ORT_API_STATUS_IMPL(OrtApis::GetCurrentGpuDeviceId, _In_ int* device_id) {
  ORT_UNUSED_PARAMETER(device_id);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::SetCurrentGpuDeviceId, _In_ int device_id) {
  ORT_UNUSED_PARAMETER(device_id);
  return CreateNotEnabledStatus("CUDA");
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_ROCM,
                    _In_ OrtSessionOptions* options, _In_ const OrtROCMProviderOptions* provider_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(provider_options);
  return CreateNotEnabledStatus("ROCM");
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_OpenVINO,
                    _In_ OrtSessionOptions* options, _In_ const OrtOpenVINOProviderOptions* provider_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(provider_options);
  return CreateNotEnabledStatus("OpenVINO");
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_TensorRT,
                    _In_ OrtSessionOptions* options, _In_ const OrtTensorRTProviderOptions* tensorrt_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(tensorrt_options);
  return CreateNotEnabledStatus("TensorRT");
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_TensorRT_V2,
                    _In_ OrtSessionOptions* options, _In_ const OrtTensorRTProviderOptionsV2* tensorrt_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(tensorrt_options);
  return CreateNotEnabledStatus("TensorRT");
}

ORT_API_STATUS_IMPL(OrtApis::CreateTensorRTProviderOptions, _Outptr_ OrtTensorRTProviderOptionsV2** out) {
  ORT_UNUSED_PARAMETER(out);
  return CreateNotEnabledStatus("TensorRT");
}

ORT_API_STATUS_IMPL(OrtApis::UpdateTensorRTProviderOptions,
                    _Inout_ OrtTensorRTProviderOptionsV2* tensorrt_options,
                    _In_reads_(num_keys) const char* const* provider_options_keys,
                    _In_reads_(num_keys) const char* const* provider_options_values,
                    size_t num_keys) {
  ORT_UNUSED_PARAMETER(tensorrt_options);
  ORT_UNUSED_PARAMETER(provider_options_keys);
  ORT_UNUSED_PARAMETER(provider_options_values);
  ORT_UNUSED_PARAMETER(num_keys);
  return CreateNotEnabledStatus("TensorRT");
}

ORT_API_STATUS_IMPL(OrtApis::GetTensorRTProviderOptionsAsString,
                    _In_ const OrtTensorRTProviderOptionsV2* tensorrt_options,
                    _Inout_ OrtAllocator* allocator,
                    _Outptr_ char** ptr) {
  ORT_UNUSED_PARAMETER(tensorrt_options);
  ORT_UNUSED_PARAMETER(allocator);
  ORT_UNUSED_PARAMETER(ptr);
  return CreateNotEnabledStatus("TensorRT");
}

ORT_API(void, OrtApis::ReleaseTensorRTProviderOptions, _Frees_ptr_opt_ OrtTensorRTProviderOptionsV2* ptr) {
  ORT_UNUSED_PARAMETER(ptr);
}

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_MIGraphX,
                    _In_ OrtSessionOptions* options, _In_ const OrtMIGraphXProviderOptions* migraphx_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(migraphx_options);
  return CreateNotEnabledStatus("MIGraphX");
}

#ifndef USE_SNPE
ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_SNPE,
                    _In_ OrtSessionOptions* options,
                    _In_reads_(num_keys) const char* const* provider_options_keys,
                    _In_reads_(num_keys) const char* const* provider_options_values,
                    _In_ size_t num_keys) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(provider_options_keys);
  ORT_UNUSED_PARAMETER(provider_options_values);
  ORT_UNUSED_PARAMETER(num_keys);
  return CreateNotEnabledStatus("SNPE");
}
#endif

ORT_API_STATUS_IMPL(OrtApis::SessionOptionsAppendExecutionProvider_Xnnpack,
                    _In_ OrtSessionOptions* options, _In_ const OrtProviderOptions* xnnpack_options) {
  ORT_UNUSED_PARAMETER(options);
  ORT_UNUSED_PARAMETER(xnnpack_options);
  return CreateNotEnabledStatus("XNNPACK");
}
#endif
