// Copyright(C) 2019 Intel Corporation
// Licensed under the MIT License

#pragma once

#include "core/framework/execution_provider.h"
#include <string>
#include "core/providers/qnn/builder/qnn_backend_manager.h"
#include "core/providers/qnn/builder/qnn_model.h"

namespace onnxruntime {
// namespace qnn {
// class QnnBackendManager;
// class QnnModel;
// }

// Logical device representation.
class QNNExecutionProvider : public IExecutionProvider {
 public:
  explicit QNNExecutionProvider(const ProviderOptions& provider_options_map);
  virtual ~QNNExecutionProvider() = default;
  ORT_DISALLOW_COPY_ASSIGNMENT_AND_MOVE(QNNExecutionProvider);

  // we implement the Compile that takes FusedNodeAndGraph instances
  FusionStyle GetFusionStyle() const override { return FusionStyle::FilteredGraphViewer; }

  std::vector<std::unique_ptr<ComputeCapability>>
  GetCapability(const GraphViewer& graph_viewer,
                const std::vector<const KernelRegistry*>& kernel_registries) const override;

  Status Compile(const std::vector<FusedNodeAndGraph>& fused_nodes_and_graphs,
                 std::vector<NodeComputeInfo>& node_compute_funcs) override;

  const void* GetExecutionHandle() const noexcept override {
    return nullptr;
  }

  DataLayout GetPreferredLayout() const override;

 private:
  void ParseProfilingLevel(std::string profiling_level_string) {
    std::transform(profiling_level_string.begin(),
                   profiling_level_string.end(),
                   profiling_level_string.begin(),
                   [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
    profiling_level_ = qnn::ProfilingLevel::OFF;
    if (profiling_level_string == "off") {
      profiling_level_ = qnn::ProfilingLevel::OFF;
    } else if (profiling_level_string == "basic") {
      profiling_level_ = qnn::ProfilingLevel::BASIC;
    } else if (profiling_level_string == "detailed") {
      profiling_level_ = qnn::ProfilingLevel::DETAILED;
    } else {
      LOGS_DEFAULT(WARNING) << "Profiling level not valid.";
    }
  }

  bool IsNodeSupported(qnn::QnnModelWrapper* qnn_model_wrapper, const NodeUnit& node_unit,
                       std::unordered_map<const NodeUnit*, bool>& node_unit_supported_result,
                       std::unordered_set<std::string> initializer_input_lookup,
                       const logging::Logger& logger) const;

  std::unordered_set<const Node*> GetSupportedNodes(const GraphViewer& graph_viewer,
                                                    const std::unordered_map<const Node*, const NodeUnit*>& node_unit_map,
                                                    const size_t node_unit_size,
                                                    const logging::Logger& logger) const;

 private:
  ProviderOptions runtime_options_;
  std::string backend_path_;
  bool is_quantized_model_;
  qnn::ProfilingLevel profiling_level_;
  std::unique_ptr<qnn::QnnBackendManager> qnn_backend_manager_;
  std::unordered_map<std::string, std::unique_ptr<qnn::QnnModel>> qnn_models_;
  AllocatorPtr cpu_allocator_;
  uint32_t rpc_control_latency_;
};

}  // namespace onnxruntime
