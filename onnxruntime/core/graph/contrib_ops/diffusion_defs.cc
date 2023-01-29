// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/graph/constants.h"
#include "core/graph/contrib_ops/contrib_defs.h"
#include "core/graph/contrib_ops/onnx_function_util.h"
#include "core/graph/contrib_ops/shape_inference_functions.h"

// Suppress a warning: global initializer calls a non-constexpr function 'symbol' which is from
// ONNX_OPERATOR_SET_SCHEMA_EX macro and only happens in debug build
#if defined(_WIN32) && !defined(NDEBUG)
#pragma warning(disable : 26426)
#endif

namespace onnxruntime {
namespace contrib {
using ONNX_NAMESPACE::AttributeProto;
using ONNX_NAMESPACE::OpSchema;
using ONNX_NAMESPACE::TensorShapeProto;
#ifndef NDEBUG
using ONNX_NAMESPACE::DbgOperatorSetTracker;
#endif

constexpr const char* GroupNorm_ver1_doc = R"DOC(
Applies Group Normalization over a mini-batch of inputs as described in the paper Group Normalization (https://arxiv.org/abs/1803.08494).

This operator transforms input according to
  y = gamma * (x - mean) / sqrt(variance + epsilon) + beta

The input channels are separated into num_groups groups, each containing num_channels / num_groups channels. num_channels must be divisible by num_groups. The mean and standard-deviation are calculated separately over the each group.
The weight and bias are per-channel affine transform parameter vectors of size num_channels.

The activation attribute can be used to enable activation after group normalization.
)DOC";

ONNX_MS_OPERATOR_SET_SCHEMA(
    GroupNorm, 1,
    OpSchema()
        .SetDoc(GroupNorm_ver1_doc)
        .Attr("epsilon", "The epsilon value to use to avoid division by zero", AttributeProto::FLOAT, static_cast<float>(1e-5))
        .Attr("groups",
              "The number of groups of channels. It should be a divisor of the number of channels C",
              AttributeProto::INT)
        .Attr("activation",
              "Activation after group normalization: 0 for None, 1 for Swish",
              AttributeProto::INT)
        .Input(0,
               "X",
               "Input data tensor. Dimensions are (N x C x H x W), where N is the batch size, C is the number of channels, and H and W are the height and width of the data",
               "T")
        .Input(1,
               "gamma",
               "1D gamma tensor for normalization with shape (C), where C is number of channels",
               "M")
        .Input(2,
               "beta",
               "1D beta tensor for normalization  with shape (C), where C is number of channels",
               "M")
        .Output(0,
                "Y",
                "The output tensor of the same shape as X",
                "T")
        .TypeConstraint("T", {"tensor(float16)", "tensor(float)"}, "Constrain input X and output Y types to float tensors.")
        .TypeConstraint("M", {"tensor(float)"}, "Constrain gamma and beta to float tensors.")
        .TypeAndShapeInferenceFunction(ONNX_NAMESPACE::propagateShapeAndTypeFromFirstInput));

constexpr const char* SplitGelu_ver1_doc = R"DOC(
A fusion used in diffusion model that hidden state is sliced into two parts, one part applied Gelu actication, then these
two parts are multiplied.
)DOC";

ONNX_MS_OPERATOR_SET_SCHEMA(
    SplitGelu, 1,
    OpSchema()
        .SetDoc(SplitGelu_ver1_doc)
        .Input(0,
               "X",
               "Input data tensor. Dimensions are (N, H*W, D), where N is the batch size, H and W are the height and width of the data, and D is hidden dimension",
               "T")
        .Output(0,
                "Y",
                "The output tensor with dimensions (N, H*W, D/2)",
                "T")
        .TypeConstraint("T", {"tensor(float16)", "tensor(float)"}, "Constrain input X and output Y types to half tensors.")
        .TypeAndShapeInferenceFunction([](ONNX_NAMESPACE::InferenceContext& ctx) {
          propagateElemTypeFromInputToOutput(ctx, 0, 0);
          if (hasInputShape(ctx, 0)) {
            auto& input_shape = getInputShape(ctx, 0);
            if (input_shape.dim().size() != 3) {
              fail_shape_inference("input shall be 3 dimensions");
            }

            TensorShapeProto output_shape;
            *output_shape.add_dim() = input_shape.dim(0);
            *output_shape.add_dim() = input_shape.dim(1);
            if (input_shape.dim(2).has_dim_value()) {
              output_shape.add_dim()->set_dim_value(input_shape.dim(2).dim_value() / 2);
            } else {
              output_shape.add_dim();
            }

            updateOutputShape(ctx, 0, output_shape);
          }
        }));
}  // namespace contrib
}  // namespace onnxruntime