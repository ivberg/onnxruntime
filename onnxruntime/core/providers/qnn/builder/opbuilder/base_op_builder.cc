// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/providers/qnn/builder/opbuilder/base_op_builder.h"

#include <core/providers/common.h>

#include "core/providers/shared/utils/utils.h"
#include "core/framework/tensorprotoutils.h"
#include "core/providers/cpu/tensor/transpose.h"
#include "core/common/safeint.h"

namespace onnxruntime {
namespace qnn {

std::string BaseOpBuilder::GetOpBuilderType() const {
  return op_builder_type_;
}

// Add operator related
Status BaseOpBuilder::IsOpSupported(QnnModelWrapper* qnn_model_wrapper,
                                    const NodeUnit& node_unit,
                                    const logging::Logger& logger,
                                    bool is_quantized_model) const {
  return AddToModelBuilder(qnn_model_wrapper, node_unit, logger, is_quantized_model, true);
}

// Add operator related
Status BaseOpBuilder::AddToModelBuilder(QnnModelWrapper* qnn_model_wrapper,
                                        const NodeUnit& node_unit,
                                        const logging::Logger& logger,
                                        bool is_quantized_model,
                                        bool do_op_validation) const {
  LOGS(logger, VERBOSE) << "QNN node builder is trying to add node. Onnx node name: [" << node_unit.Name()
                        << "] onnx node type: [" << node_unit.OpType() << "].";

  std::vector<std::string> input_names;
  // Inputs & output handling mostly same for most of the Ops, just node attributes are different
  ORT_RETURN_IF_ERROR(ProcessInputs(qnn_model_wrapper, node_unit, logger,
                                    is_quantized_model, input_names, do_op_validation));

  ORT_RETURN_IF_ERROR(ProcessAttributesAndOutputs(qnn_model_wrapper, node_unit, input_names,
                                                  logger, is_quantized_model, do_op_validation));

  return Status::OK();
}

bool BaseOpBuilder::OnnxDataTypeToQnnDataType(const int32_t onnx_data_type, Qnn_DataType_t& qnn_data_type, bool is_quantized) const {
  const std::unordered_map<int32_t, Qnn_DataType_t> onnx_to_qnn_data_type = {
      {ONNX_NAMESPACE::TensorProto_DataType_INT8, QNN_DATATYPE_INT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_INT16, QNN_DATATYPE_INT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_INT32, QNN_DATATYPE_INT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_INT64, QNN_DATATYPE_INT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT8, QNN_DATATYPE_UINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT16, QNN_DATATYPE_UINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT32, QNN_DATATYPE_UINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT64, QNN_DATATYPE_UINT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT16, QNN_DATATYPE_FLOAT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT, QNN_DATATYPE_FLOAT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_BOOL, QNN_DATATYPE_BOOL_8},
  };

  const std::unordered_map<int32_t, Qnn_DataType_t> onnx_to_qnn_data_type_quantized = {
      {ONNX_NAMESPACE::TensorProto_DataType_INT8, QNN_DATATYPE_SFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_INT16, QNN_DATATYPE_SFIXED_POINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_INT32, QNN_DATATYPE_SFIXED_POINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_INT64, QNN_DATATYPE_INT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT8, QNN_DATATYPE_UFIXED_POINT_8},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT16, QNN_DATATYPE_UFIXED_POINT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT32, QNN_DATATYPE_UFIXED_POINT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_UINT64, QNN_DATATYPE_UINT_64},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT16, QNN_DATATYPE_FLOAT_16},
      {ONNX_NAMESPACE::TensorProto_DataType_FLOAT, QNN_DATATYPE_FLOAT_32},
      {ONNX_NAMESPACE::TensorProto_DataType_BOOL, QNN_DATATYPE_BOOL_8},
  };

  const auto do_type_mapping = [](const std::unordered_map<int32_t, Qnn_DataType_t>& mapping_table,
                                  const int32_t onnx_data_type,
                                  Qnn_DataType_t& qnn_data_type) -> bool {
    auto pos = mapping_table.find(onnx_data_type);
    if (pos == mapping_table.end()) {
      LOGS_DEFAULT(INFO) << "Onnx data type not supported by Qnn, onnx data type: " << onnx_data_type;
      return false;
    }
    qnn_data_type = pos->second;
    return true;
  };

  if (is_quantized) {
    return do_type_mapping(onnx_to_qnn_data_type_quantized, onnx_data_type, qnn_data_type);
  } else {
    return do_type_mapping(onnx_to_qnn_data_type, onnx_data_type, qnn_data_type);
  }
}

Status BaseOpBuilder::ProcessInputs(QnnModelWrapper* qnn_model_wrapper,
                                    const NodeUnit& node_unit,
                                    const logging::Logger& logger,
                                    bool is_quantized_model,
                                    std::vector<std::string>& input_names,
                                    bool do_op_validation) const {
  ORT_UNUSED_PARAMETER(do_op_validation);
  Qnn_QuantizeParams_t quantize_param = QNN_QUANTIZE_PARAMS_INIT;
  InitializeQuantizeParam(quantize_param, is_quantized_model);
  Qnn_DataType_t qnn_data_type = QNN_DATATYPE_FLOAT_32;

  auto inputs = node_unit.Inputs();
  for (size_t input_i = 0; input_i < inputs.size(); ++input_i) {
    auto& input_name = inputs[input_i].node_arg.Name();

    if (qnn_model_wrapper->QnnContainsTensor(input_name)) {
      LOGS(logger, VERBOSE) << "Tensor already added, skip it: " << input_name;
      input_names.push_back(input_name);
      continue;
    }

    const auto* type_proto = inputs[input_i].node_arg.TypeAsProto();
    int32_t onnx_data_type;
    ORT_RETURN_IF_ERROR(GetQnnDataType(is_quantized_model, type_proto, onnx_data_type, qnn_data_type));

    std::vector<uint32_t> input_shape;
    ORT_RETURN_IF_NOT(qnn_model_wrapper->GetOnnxShape(inputs[input_i].node_arg, input_shape), "Cannot get shape");
    ORT_RETURN_IF_NOT(qnn_model_wrapper->ProcessQuantizationParameter(inputs[input_i].quant_param,
                                                                      quantize_param.scaleOffsetEncoding.scale,
                                                                      quantize_param.scaleOffsetEncoding.offset),
                      "Cannot get quantization parameter");

    std::vector<uint8_t> unpacked_tensor;
    bool is_initializer_input = qnn_model_wrapper->IsInitializerInput(input_name);
    if (is_initializer_input) {
      const auto& input_tensor = qnn_model_wrapper->GetInitializerTensors().at(input_name);
      ORT_RETURN_IF_ERROR(onnxruntime::utils::UnpackInitializerData(*input_tensor, unpacked_tensor));
    }

    input_names.push_back(input_name);

    Qnn_TensorType_t tensor_type = is_initializer_input ? QNN_TENSOR_TYPE_STATIC : QNN_TENSOR_TYPE_APP_WRITE;
    Qnn_TensorDataFormat_t data_format = 0;
    QnnTensorWrapper input_tensorwrapper(qnn_model_wrapper->GetAllocator(),
                                         input_name, tensor_type, data_format, qnn_data_type, quantize_param,
                                         std::move(input_shape), std::move(unpacked_tensor));
    ORT_RETURN_IF_NOT(qnn_model_wrapper->AddTensor(input_name, std::move(input_tensorwrapper)), "Failed to add tensor.");
  }

  return Status::OK();
}

Status BaseOpBuilder::ProcessAttributesAndOutputs(QnnModelWrapper* qnn_model_wrapper,
                                                  const NodeUnit& node_unit,
                                                  const std::vector<std::string>& input_names,
                                                  const logging::Logger& logger,
                                                  bool is_quantized_model,
                                                  bool do_op_validation) const {
  if (input_names.size() < 1) {
    return Status::OK();
  }

  ORT_RETURN_IF_ERROR(ProcessOutputs(qnn_model_wrapper, node_unit, input_names, {}, logger, is_quantized_model, do_op_validation));
  return Status::OK();
}

Status BaseOpBuilder::ProcessOutputs(QnnModelWrapper* qnn_model_wrapper,
                                     const NodeUnit& node_unit,
                                     const std::vector<std::string>& input_names,
                                     std::vector<QnnParamWrapper>&& node_params,
                                     const logging::Logger& logger,
                                     bool is_quantized_model,
                                     bool do_op_validation) const {
  ORT_UNUSED_PARAMETER(logger);
  // Add output
  // Output part is common for all Ops, only difference is the Op attribute
  const auto& outputs = node_unit.Outputs();
  auto output_size = outputs.size();
  std::vector<QnnTensorWrapper> qnn_outputs;

  for (size_t output_i = 0; output_i < output_size && output_i < output_count_; ++output_i) {
    const auto& output_name = outputs[output_i].node_arg.Name();

    Qnn_QuantizeParams_t quantize_param = QNN_QUANTIZE_PARAMS_INIT;
    InitializeQuantizeParam(quantize_param, is_quantized_model);

    const auto* type_proto = outputs[output_i].node_arg.TypeAsProto();
    int32_t onnx_data_type;
    Qnn_DataType_t qnn_data_type = QNN_DATATYPE_FLOAT_32;
    ORT_RETURN_IF_ERROR(GetQnnDataType(is_quantized_model, type_proto, onnx_data_type, qnn_data_type));
    ORT_RETURN_IF_NOT(qnn_model_wrapper->ProcessQuantizationParameter(outputs[output_i].quant_param,
                                                                      quantize_param.scaleOffsetEncoding.scale,
                                                                      quantize_param.scaleOffsetEncoding.offset),
                      "Cannot get quantization parameter");
    std::vector<uint32_t> output_shape;
    ORT_RETURN_IF_NOT(qnn_model_wrapper->GetOnnxShape(outputs[output_i].node_arg, output_shape),
                      "Cannot get shape");

    bool is_graph_output = qnn_model_wrapper->IsGraphOutput(output_name);
    Qnn_TensorType_t tensor_type = is_graph_output ? QNN_TENSOR_TYPE_APP_READ : QNN_TENSOR_TYPE_NATIVE;
    Qnn_TensorDataFormat_t data_format = 0;
    QnnTensorWrapper qnn_output(qnn_model_wrapper->GetAllocator(),
                                output_name, tensor_type, data_format, qnn_data_type, quantize_param,
                                std::move(output_shape));
    qnn_outputs.push_back(std::move(qnn_output));
  }

  ORT_RETURN_IF_NOT(qnn_model_wrapper->AddNode(GetNodeName(node_unit),            // Node Name
                                               qnn_def::package_name,             // Package Name
                                               GetQnnOpType(node_unit.OpType()),  // Qnn Node Type
                                               std::move(node_params),            //
                                               input_names,                       // Input Tensor Names
                                               std::move(qnn_outputs),            // Output Tensors
                                               do_op_validation),
                    "Failed to add node.");

  return Status::OK();
}

Status BaseOpBuilder::TransposeInitializer(const onnx::TensorProto& initializer,
                                           const std::vector<size_t>& perm,
                                           const AllocatorPtr& cpu_allocator,
                                           std::vector<uint8_t>& transposed_data) const {
  const DataTypeImpl* tensor_dtype = DataTypeImpl::TensorTypeFromONNXEnum(initializer.data_type())->GetElementType();
  const auto tensor_shape_dims = onnxruntime::utils::GetTensorShapeFromTensorProto(initializer);
  TensorShape tensor_shape{tensor_shape_dims};
  Tensor in_tensor = Tensor(tensor_dtype, tensor_shape, cpu_allocator);

  auto rank = perm.size();
  std::vector<int64_t> new_tensor_shape_dims;
  std::vector<size_t> permutations;
  new_tensor_shape_dims.reserve(rank);
  permutations.reserve(rank);
  for (int64_t p : perm) {
    permutations.push_back(p);
    new_tensor_shape_dims.push_back(tensor_shape_dims[p]);
  }

  TensorShape new_tensor_shape(new_tensor_shape_dims);
  Tensor out_tensor = Tensor(tensor_dtype, new_tensor_shape, cpu_allocator);
  ORT_RETURN_IF_ERROR(onnxruntime::utils::TensorProtoToTensor(Env::Default(), nullptr, initializer, in_tensor));
  ORT_RETURN_IF_ERROR(Transpose::DoTranspose(permutations, in_tensor, out_tensor));
  onnx::TensorProto new_tensor_proto = onnxruntime::utils::TensorToTensorProto(out_tensor, "test");
  ORT_RETURN_IF_ERROR(onnxruntime::utils::UnpackInitializerData(new_tensor_proto, transposed_data));

  return Status::OK();
}

Status BaseOpBuilder::ProcessAxisAttribute(QnnModelWrapper* qnn_model_wrapper,
                                           const NodeUnit& node_unit,
                                           std::vector<QnnParamWrapper>& node_params,
                                           int32_t& default_axis_value) const {
  const auto& inputs = node_unit.Inputs();
  std::vector<uint32_t> input_shape;
  ORT_RETURN_IF_NOT(qnn_model_wrapper->GetOnnxShape(inputs[0].node_arg, input_shape), "Cannot get shape");

  auto rank = static_cast<int32_t>(input_shape.size());
  NodeAttrHelper node_helper(node_unit);
  int32_t onnx_axis = node_helper.Get("axis", default_axis_value);
  if (onnx_axis < 0) {
    onnx_axis += rank;
  }
  ORT_ENFORCE((onnx_axis >= 0 && onnx_axis < static_cast<int32_t>(input_shape.size())), "QNN requires axis range [0, rank-1].");
  default_axis_value = onnx_axis;

  bool is_gather_op = (node_unit.OpType() == "Gather");
  Qnn_Scalar_t axis_qnn_scalar = QNN_SCALAR_INIT;
  if (is_gather_op) {
    axis_qnn_scalar.dataType = QNN_DATATYPE_INT_32;
    axis_qnn_scalar.int32Value = onnx_axis;
  } else {
    axis_qnn_scalar.dataType = QNN_DATATYPE_UINT_32;
    axis_qnn_scalar.uint32Value = static_cast<uint32_t>(onnx_axis);
  }
  QnnParamWrapper axis_param(qnn_model_wrapper->GetAllocator(),
                             qnn_def::axis, axis_qnn_scalar);
  node_params.push_back(std::move(axis_param));

  return Status::OK();
}

}  // namespace qnn
}  // namespace onnxruntime
