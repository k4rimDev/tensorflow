/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/compiler/jit/xla_compile_util.h"

#include <vector>

#include "tensorflow/core/util/determinism.h"

namespace tensorflow {

xla::ExecutableBuildOptions GetExecutableBuildOptions(
    const XlaCompiler::Options& options,
    const XlaCompiler::CompilationResult& result, int default_device_ordinal) {
  xla::ExecutableBuildOptions build_options;
  if (result.collective_info) {
    build_options.set_num_replicas(result.collective_info->group_size);
  }
  if (options.device_ordinal != -1) {
    build_options.set_device_ordinal(options.device_ordinal);
  } else if (default_device_ordinal != -1) {
    build_options.set_device_ordinal(default_device_ordinal);
  }
  build_options.set_result_layout(result.xla_output_shape);
  build_options.set_device_allocator(options.device_allocator.get());
  build_options.set_alias_passthrough_params(options.alias_passthrough_params);
  build_options.mutable_debug_options()->set_xla_detailed_logging_and_dumping(
      options.detailed_logging);
  if (tensorflow::OpDeterminismRequired()) {
    build_options.mutable_debug_options()->set_xla_gpu_deterministic_ops(true);
  }
  return build_options;
}

XlaCompiler::SingleOpCompileArgument BuildSingleOpCompileArgument(
    OpKernelContext* ctx) {
  XlaCompiler::SingleOpCompileArgument single_op_arg;
  std::vector<DataType> output_dtypes(ctx->num_outputs());
  for (int i = 0; i < output_dtypes.size(); ++i) {
    output_dtypes[i] = ctx->expected_output_dtype(i);
  }
  single_op_arg.output_dtypes = output_dtypes;
  single_op_arg.node_def = ctx->op_kernel().def();
  auto* config_proto = ctx->function_library()->config_proto();
  if (config_proto != nullptr) {
    single_op_arg.config_proto = *config_proto;
  }
  return single_op_arg;
}

}  // namespace tensorflow
