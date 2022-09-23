// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/providers/common.h"
#include "core/providers/shared/utils/utils.h"
#include "core/framework/tensorprotoutils.h"
#include "core/providers/qnn/builder/qnn_model_wrapper.h"
#include "core/providers/qnn/builder/op_builder_factory.h"

#include "base_op_builder.h"

namespace onnxruntime {
namespace qnn {

class ArgMaxMinOpBuilder : public BaseOpBuilder {
 public:
  ArgMaxMinOpBuilder() : BaseOpBuilder("ArgMaxMinOpBuilder") {}

 protected:
  Status ProcessAttributesAndOutputs(QnnModelWrapper* qnn_model_wrapper,
                                     const NodeUnit& node_unit,
                                     const std::vector<std::string>& input_names,
                                     const logging::Logger& logger,
                                     bool is_quantized_model,
                                     bool do_op_validation) const override ORT_MUST_USE_RESULT;
};

Status ArgMaxMinOpBuilder::ProcessAttributesAndOutputs(QnnModelWrapper* qnn_model_wrapper,
                                                       const NodeUnit& node_unit,
                                                       const std::vector<std::string>& input_names,
                                                       const logging::Logger& logger,
                                                       bool is_quantized_model,
                                                       bool do_op_validation) const {
  std::vector<QnnParamWrapper> node_params;
  int32_t default_axis_value = 0;
  ORT_RETURN_IF_ERROR(ProcessAxisAttribute(qnn_model_wrapper, node_unit, node_params, default_axis_value));

  NodeAttrHelper node_helper(node_unit);
  auto select_last_index = node_helper.Get("select_last_index", static_cast<int32_t>(0));
  if (select_last_index != 0) {
    return ORT_MAKE_STATUS(ONNXRUNTIME, FAIL, "QNN ArgMax/ArgMin only support select_last_index=0.");
  }
  auto onnx_keepdims = node_helper.Get("keepdims", static_cast<int32_t>(1));
  Qnn_Scalar_t keep_dims_scalar = QNN_SCALAR_INIT;
  keep_dims_scalar.dataType = QNN_DATATYPE_BOOL_8;
  keep_dims_scalar.bool8Value = static_cast<uint8_t>(onnx_keepdims == 0 ? 0 : 1);
  QnnParamWrapper keep_dims_param(qnn_model_wrapper->GetAllocator(),
                                  qnn_def::keep_dims, keep_dims_scalar);
  node_params.push_back(std::move(keep_dims_param));

  ORT_RETURN_IF_ERROR(ProcessOutputs(qnn_model_wrapper, node_unit, input_names,
                                     std::move(node_params), logger, is_quantized_model, do_op_validation));

  return Status::OK();
}

void CreateArgMaxMinOpBuilder(const std::string& op_type, OpBuilderRegistrations& op_registrations) {
  op_registrations.AddOpBuilder(op_type, std::make_unique<ArgMaxMinOpBuilder>());
}

}  // namespace qnn
}  // namespace onnxruntime
