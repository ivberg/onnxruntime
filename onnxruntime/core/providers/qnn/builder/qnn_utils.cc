// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <functional>
#include <numeric>
#include <string>
#include <vector>
#include <map>

#include "core/common/common.h"
#include "qnn_utils.h"

namespace onnxruntime {
namespace qnn {
namespace utils {
std::hash<std::string> g_strToUInt32;
uint32_t GetTensorIdFromName(const std::string& name) { return static_cast<uint32_t>(g_strToUInt32(name)); }

int64_t GetDataSize(const std::vector<uint32_t>& dims) {
  if (dims.size() == 0) {
    return 0;
  }
  return std::accumulate(dims.begin(), dims.end(), static_cast<uint32_t>(1), std::multiplies<uint32_t>());
}

size_t GetElementSizeByType(const Qnn_DataType_t& data_type) {
  const static std::unordered_map<Qnn_DataType_t, size_t> data_type_to_size = {
      {QNN_DATATYPE_INT_8, 1},
      {QNN_DATATYPE_INT_16, 2},
      {QNN_DATATYPE_INT_32, 4},
      {QNN_DATATYPE_INT_64, 8},
      {QNN_DATATYPE_UINT_8, 1},
      {QNN_DATATYPE_UINT_16, 2},
      {QNN_DATATYPE_UINT_32, 4},
      {QNN_DATATYPE_UINT_64, 8},
      {QNN_DATATYPE_FLOAT_16, 2},
      {QNN_DATATYPE_FLOAT_32, 4},
      {QNN_DATATYPE_BOOL_8, 1},
      {QNN_DATATYPE_SFIXED_POINT_8, 1},
      {QNN_DATATYPE_SFIXED_POINT_16, 2},
      {QNN_DATATYPE_SFIXED_POINT_32, 4},
      {QNN_DATATYPE_UFIXED_POINT_8, 1},
      {QNN_DATATYPE_UFIXED_POINT_16, 2},
      {QNN_DATATYPE_UFIXED_POINT_32, 4},
  };

  auto pos = data_type_to_size.find(data_type);
  if (pos == data_type_to_size.end()) {
    return static_cast<size_t>(0);
  }

  return pos->second;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Scalar_t& scalar) {
  switch (scalar.dataType) {
    case QNN_DATATYPE_INT_8:
      out << static_cast<int32_t>(scalar.int8Value);
      break;
    case QNN_DATATYPE_INT_16:
      out << scalar.int16Value;
      break;
    case QNN_DATATYPE_INT_32:
      out << scalar.int32Value;
      break;
    case QNN_DATATYPE_INT_64:
      out << "int64_t is not supported";
      break;
    case QNN_DATATYPE_UINT_8:
      out << static_cast<int32_t>(scalar.uint8Value);
      break;
    case QNN_DATATYPE_UINT_16:
      out << scalar.uint16Value;
      break;
    case QNN_DATATYPE_UINT_32:
      out << scalar.uint32Value;
      break;
    case QNN_DATATYPE_UINT_64:
      out << "uint64_t is not supported";
      break;
    case QNN_DATATYPE_FLOAT_16:
      break;
    case QNN_DATATYPE_FLOAT_32:
      out << scalar.floatValue;
      break;
    case QNN_DATATYPE_SFIXED_POINT_8:
    case QNN_DATATYPE_SFIXED_POINT_16:
    case QNN_DATATYPE_SFIXED_POINT_32:
    case QNN_DATATYPE_UFIXED_POINT_8:
    case QNN_DATATYPE_UFIXED_POINT_16:
    case QNN_DATATYPE_UFIXED_POINT_32:
      out << "usigned fixedpoint data is not supported";
      break;
    case QNN_DATATYPE_BOOL_8:
      out << static_cast<int32_t>(scalar.bool8Value);
      break;
    default:
      ORT_THROW("Unknown Qnn Data type");
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_DataType_t& data_type) {
  switch (data_type) {
    case QNN_DATATYPE_INT_8:
      out << "QNN_DATATYPE_INT_8";
      break;
    case QNN_DATATYPE_INT_16:
      out << "QNN_DATATYPE_INT_16";
      break;
    case QNN_DATATYPE_INT_32:
      out << "QNN_DATATYPE_INT_32";
      break;
    case QNN_DATATYPE_INT_64:
      out << "QNN_DATATYPE_INT_64";
      break;
    case QNN_DATATYPE_UINT_8:
      out << "QNN_DATATYPE_UINT_8";
      break;
    case QNN_DATATYPE_UINT_16:
      out << "QNN_DATATYPE_UINT_16";
      break;
    case QNN_DATATYPE_UINT_32:
      out << "QNN_DATATYPE_UINT_32";
      break;
    case QNN_DATATYPE_UINT_64:
      out << "QNN_DATATYPE_UINT_64";
      break;
    case QNN_DATATYPE_FLOAT_16:
      out << "QNN_DATATYPE_FLOAT_16";
      break;
    case QNN_DATATYPE_FLOAT_32:
      out << "QNN_DATATYPE_FLOAT_32";
      break;
    case QNN_DATATYPE_SFIXED_POINT_8:
      out << "QNN_DATATYPE_SFIXED_POINT_8";
      break;
    case QNN_DATATYPE_SFIXED_POINT_16:
      out << "QNN_DATATYPE_SFIXED_POINT_16";
      break;
    case QNN_DATATYPE_SFIXED_POINT_32:
      out << "QNN_DATATYPE_SFIXED_POINT_32";
      break;
    case QNN_DATATYPE_UFIXED_POINT_8:
      out << "QNN_DATATYPE_UFIXED_POINT_8";
      break;
    case QNN_DATATYPE_UFIXED_POINT_16:
      out << "QNN_DATATYPE_UFIXED_POINT_16";
      break;
    case QNN_DATATYPE_UFIXED_POINT_32:
      out << "QNN_DATATYPE_UFIXED_POINT_32";
      break;
    case QNN_DATATYPE_BOOL_8:
      out << "QNN_DATATYPE_BOOL_8";
      break;
    default:
      ORT_THROW("Unknown Qnn Data type");
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Definition_t& definition) {
  switch (definition) {
    case QNN_DEFINITION_IMPL_GENERATED:
      out << "QNN_DEFINITION_IMPL_GENERATED";
      break;
    case QNN_DEFINITION_DEFINED:
      out << "QNN_DEFINITION_DEFINED";
      break;
    case QNN_DEFINITION_UNDEFINED:
      out << "QNN_DEFINITION_UNDEFINED";
      break;
    default:
      out << "Undefined";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_QuantizationEncoding_t& encoding) {
  switch (encoding) {
    case QNN_QUANTIZATION_ENCODING_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_SCALE_OFFSET";
      break;
    case QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET:
      out << "QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET";
      break;
    default:
      out << "Uknown quantization encoding";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_QuantizeParams_t& quantize_params) {
  out << " encodingDefinition=" << quantize_params.encodingDefinition;
  if (quantize_params.encodingDefinition == QNN_DEFINITION_IMPL_GENERATED ||
      quantize_params.encodingDefinition == QNN_DEFINITION_DEFINED) {
    out << " encoding=" << quantize_params.quantizationEncoding;
    if (quantize_params.quantizationEncoding == QNN_QUANTIZATION_ENCODING_SCALE_OFFSET) {
      out << " scale=" << quantize_params.scaleOffsetEncoding.scale;
      out << " offset=" << quantize_params.scaleOffsetEncoding.offset;
    } else {
    }
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_TensorType_t& tensor_type) {
  switch (tensor_type) {
    case QNN_TENSOR_TYPE_APP_WRITE:
      out << "QNN_TENSOR_TYPE_APP_WRITE";
      break;
    case QNN_TENSOR_TYPE_APP_READ:
      out << "QNN_TENSOR_TYPE_APP_READ";
      break;
    case QNN_TENSOR_TYPE_APP_READWRITE:
      out << "QNN_TENSOR_TYPE_APP_READWRITE";
      break;
    case QNN_TENSOR_TYPE_NATIVE:
      out << "QNN_TENSOR_TYPE_NATIVE";
      break;
    case QNN_TENSOR_TYPE_STATIC:
      out << "QNN_TENSOR_TYPE_STATIC";
      break;
    case QNN_TENSOR_TYPE_NULL:
      out << "QNN_TENSOR_TYPE_NULL";
      break;
    default:
      out << "Unsupported type";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_TensorMemType_t& mem_type) {
  switch (mem_type) {
    case QNN_TENSORMEMTYPE_RAW:
      out << "QNN_TENSORMEMTYPE_RAW";
      break;
    case QNN_TENSORMEMTYPE_MEMHANDLE:
      out << "QNN_TENSORMEMTYPE_MEMHANDLE";
      break;
    default:
      out << "Unsupported mem type";
  }
  return out;
}
template <typename T>
std::ostream& operator<<(std::ostream& out, const Qnn_ClientBuffer_t& client_bufer) {
  T* data = reinterpret_cast<T*>(client_bufer.data);
  uint32_t count = client_bufer.dataSize / sizeof(T);
  count = count > 100 ? 100 : count;  // limit to 100 data
  out << " clientBuf=(";
  for (uint32_t i = 0; i < count; i++) {
    out << data[i] << " ";
  }
  out << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Tensor_t& tensor) {
  out << " id=" << tensor.id;
  out << " type=" << tensor.type;
  out << " dataFormat=" << tensor.dataFormat;
  out << " dataType=" << tensor.dataType;
  out << " rank=" << tensor.rank;
  out << " maxDimensions=(";
  for (uint32_t i = 0; i < tensor.rank; i++) {
    out << tensor.maxDimensions[i] << " ";
  }
  out << ")";
  out << " memType=" << tensor.memType;
  if (tensor.memType == QNN_TENSORMEMTYPE_RAW) {
    if (tensor.dataType == QNN_DATATYPE_UINT_32) {
      operator<<<uint32_t>(out, tensor.clientBuf);
    } else {
      operator<<<int32_t>(out, tensor.clientBuf);
    }
  }
  out << " quantizeParams:" << tensor.quantizeParams;
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_ParamType_t& param_type) {
  switch (param_type) {
    case QNN_PARAMTYPE_SCALAR:
      out << "QNN_PARAMTYPE_SCALAR";
      break;
    case QNN_PARAMTYPE_TENSOR:
      out << "QNN_PARAMTYPE_TENSOR";
      break;
    default:
      out << "Unknown type";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_Param_t& qnn_param) {
  out << " type=" << qnn_param.paramType;
  out << " name=" << qnn_param.name;
  if (qnn_param.paramType == QNN_PARAMTYPE_TENSOR) {
    out << qnn_param.tensorParam;
  } else {
    out << " value=" << qnn_param.scalarParam;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Qnn_OpConfig_t& op_definition) {
  out << "Qnn_OpConfig node name: " << op_definition.name
      << " package_name: " << op_definition.packageName
      << " QNN_op_type: " << op_definition.typeName
      << " num_of_params: " << op_definition.numOfParams
      << " num_of_inputs: " << op_definition.numOfInputs
      << " num_of_outputs: " << op_definition.numOfOutputs;

  out << std::endl
      << " node_inputs:" << std::endl;
  for (uint32_t i = 0; i < op_definition.numOfInputs; i++) {
    out << op_definition.inputTensors[i] << std::endl;
  }
  out << " node_outputs:" << std::endl;
  for (uint32_t i = 0; i < op_definition.numOfOutputs; i++) {
    out << op_definition.outputTensors[i] << std::endl;
  }
  out << " node_params:" << std::endl;
  for (uint32_t i = 0; i < op_definition.numOfParams; i++) {
    out << op_definition.params[i] << std::endl;
  }
  return out;
}

}  // namespace utils
}  // namespace qnn
}  // namespace onnxruntime
