# -------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------

from functorch.compile import min_cut_rematerialization_partition
from torch._dynamo.optimizations.training import aot_autograd

from .ort_backend import OrtBackend

# This should be the underlying compiler for ALL graphs if
# the user uses ORT to accelerate PyTorch via Dynamo.
# By using a global compiler for all graphs, cached compilation
# results can be reused when encountering the identical graphs.
DEFAULT_BACKEND = OrtBackend()

# Wrap ORT as a compiler in Dynamo for training (i.e., when .backward is called).
#
# Under the hood, OrtBackend.compile is called inside functorch. See aot_function
# and aot_module in aot_autograd.py in PyTorch repo for more details. Basically,
# OrtBackend.compile is mapped to forward graph compiler, fw_compile, and backward
# graph compiler, bw_compile, in aot_autograd.py.
#
# Example usage:
#  import torch
#  from onnxruntime.training.torchdynamo.register_backend import aot_ort
#  model = torch.nn.Linear(2, 2)
#  compiled_model = torch._dynamo.optimize(aot_ort)(model)
#  result = compiled_model(torch.rand(2, 2, dtype=torch.float)
#  result.sum().backward()
aot_ort = aot_autograd(fw_compiler=DEFAULT_BACKEND, partition_fn=min_cut_rematerialization_partition)

# Declare ORT as a compiler in Dynamo for inference (i.e., when .backward is NOT called).
#
# ort is usually faster than aot_ort for inference because the graphs generated by aot_autograd
# mechanism are very different than the original graphs. Therefore, some ORT's graph transformers
# are not applicable.
#
# Example usage:
#  import torch
#  from onnxruntime.training.torchdynamo.register_backend import ort
#  model = torch.nn.Linear(2, 2)
#  compiled_model = torch._dynamo.optimize(ort)(model)
ort = DEFAULT_BACKEND
