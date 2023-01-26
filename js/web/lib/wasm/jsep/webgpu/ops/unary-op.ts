// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

import {TensorView} from '../../tensor';
import {MAX_CLIP, MIN_CLIP, ShapeUtil} from '../../util';
import {AttributeWithCacheKey, createAttributeWithCacheKey} from '../attribute-with-cache-key';
import {ComputeContext, GpuDataType, ProgramInfo, ProgramInfoLoader, ProgramMetadata} from '../types';

import {WORKGROUP_SIZE} from './common';

type BuiltinFunctionName = string;
type ElementwiseCustomExpression = (expression: string) => string;
type ElementwiseFunctionCall = BuiltinFunctionName|ElementwiseCustomExpression;

const createElementwiseProgramShader =
    (datasize: number, funcCall: ElementwiseFunctionCall, additionalImplementation?: string): string => {
      const vecSize = Math.ceil(datasize / 4);

      let expression = '';
      if (typeof funcCall === 'string') {
        expression = `${funcCall}(a)`;
      } else {
        expression = funcCall('a');
      }
      return `
  const WORKGROUP_SIZE: u32 = ${WORKGROUP_SIZE}u;

  @group(0) @binding(0) var<storage, read> inputData : array<vec4<f32>>;
  @group(0) @binding(1) var<storage, read_write> outputData : array<vec4<f32>>;

  ${additionalImplementation ?? ''}

  @compute @workgroup_size(WORKGROUP_SIZE)
  fn main(@builtin(global_invocation_id) global_id : vec3<u32>) {

    // Guard against out-of-bounds work group sizes
    if (global_id.x >= ${vecSize}u) {
      return;
    }

    let a = inputData[global_id.x];
    outputData[global_id.x] = ${expression};
  }`;
    };

const createElementwiseProgramInfo =
    (metadata: ProgramMetadata, input: TensorView, funcCall: ElementwiseFunctionCall,
     additionalImplementation?: string): ProgramInfo => ({
      ...metadata,
      shaderSource: createElementwiseProgramShader(ShapeUtil.size(input.dims), funcCall, additionalImplementation),
      outputs: [{dims: input.dims, dataType: input.dataType, gpuDataType: GpuDataType.default}],
      dispatchGroup: (inputTensors) =>
          ({x: Math.ceil(ShapeUtil.size(inputTensors[0].dims) / 64 /* workgroup size */ / 4 /* vec size */)})
    });

const createElementwiseProgramInfoLoader =
    (input: TensorView, name: string, funcCall: ElementwiseFunctionCall, additionalImplementation?: string,
     cacheKey?: string): ProgramInfoLoader => {
      const metadata: ProgramMetadata = {name, inputTypes: [GpuDataType.default], cacheHint: cacheKey};
      return {
        ...metadata,
        get: () => createElementwiseProgramInfo(metadata, input, funcCall, additionalImplementation)
      };
    };

export const abs = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Abs', 'abs'));
  return 0;
};

export const acos = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Acos', 'acos'));
  return 0;
};

export const acosh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Acosh', 'acosh'));
  return 0;
};

export const asin = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Asin', 'asin'));
  return 0;
};

export const asinh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Asinh', 'asinh'));
  return 0;
};

export const atan = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Atan', 'atan'));
  return 0;
};
export const atanh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Atanh', 'atanh'));
  return 0;
};

export interface ClipAttributes extends AttributeWithCacheKey {
  readonly min: number;
  readonly max: number;
}

export const clip = (context: ComputeContext, attributes: ClipAttributes): number => {
  context.compute(
      createElementwiseProgramInfoLoader(
          context.inputs[0], 'Clip', a => `clamp(${a}, clip_min_, clip_max_)`, `
    const clip_min_: vec4<f32> = vec4(f32(${attributes.min}));
    const clip_max_: vec4<f32> = vec4(f32(${attributes.max}));
`,
          attributes.cacheKey),
      {inputs: [0]});
  return 0;
};
const generateClipAttributesFromInputs = (inputs: readonly TensorView[]): ClipAttributes => {
  const min = (inputs.length >= 2) ? inputs[1].getFloat32Array()[0] : MIN_CLIP;
  const max = (inputs.length >= 3) ? inputs[2].getFloat32Array()[0] : MAX_CLIP;
  return createAttributeWithCacheKey({min, max});
};

export const clipV11 = (context: ComputeContext): number => {
  const attributes = generateClipAttributesFromInputs(context.inputs);
  return clip(context, attributes);
};

export const ceil = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Ceil', 'ceil'));
  return 0;
};

export const cos = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Cos', 'cos'));
  return 0;
};

export const cosh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Cosh', 'cosh'));
  return 0;
};

export interface EluAttributes extends AttributeWithCacheKey {
  readonly alpha: number;
}

export const elu = (context: ComputeContext, attributes: EluAttributes): number => {
  context.compute(createElementwiseProgramInfoLoader(
      context.inputs[0], 'Elu', a => `elu_vf32(${a})`, `
  const elu_alpha_: f32 = f32(${attributes.alpha});

  fn elu_f32(a: f32) -> f32 {
  return select((exp(a) - 1.0) * elu_alpha_, a, a >= 0.0);
  }

  fn elu_vf32(v: vec4<f32>) -> vec4<f32> {
  return vec4(elu_f32(v.x), elu_f32(v.y), elu_f32(v.z), elu_f32(v.w));
  }`,
      attributes.cacheKey));
  return 0;
};

export const parseEluAttributes = (attributes: Record<string, unknown>): EluAttributes =>
    createAttributeWithCacheKey(attributes as {alpha: number});

// export const exp = async(handler: WebGpuInferenceHandler, inputs: Tensor[]): Promise<Tensor[]> =>
//     handler.run(createElementwiseProgramInfoLoader(inputs[0], 'Exp', 'exp'), inputs);

export const floor = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Floor', 'floor'));
  return 0;
};

// export interface LeakyReluAttributes extends AttributeWithCacheKey {
//   readonly alpha: number;
// }

// export const leakyRelu = async(handler: WebGpuInferenceHandler, inputs: Tensor[], attributes: EluAttributes):
//                              Promise<Tensor[] >=>handler.run(
//                                  createElementwiseProgramInfoLoader(
//                                      inputs[0], 'LeakyRelu', a => `leaky_relu_vf32(${a})`, `
//     let leaky_relu_alpha_: f32 = f32(${attributes.alpha});

//     fn leaky_relu_f32(a: f32) -> f32 {
//       return select(a, a * leaky_relu_alpha_, a < 0.0);
//     }

//     fn leaky_relu_vf32(v: vec4<f32>) -> vec4<f32> {
//       return vec4(leaky_relu_f32(v.x), leaky_relu_f32(v.y), leaky_relu_f32(v.z), leaky_relu_f32(v.w));
//     }`,
//                                      attributes.cacheKey),
//                                  inputs);

// export const parseLeakyReluAttributes = (node: Graph.Node): LeakyReluAttributes =>
//     createAttributeWithCacheKey({alpha: node.attributes.getFloat('alpha', 0.01)});

// export const log = async(handler: WebGpuInferenceHandler, inputs: Tensor[]): Promise<Tensor[]> =>
//     handler.run(createElementwiseProgramInfoLoader(inputs[0], 'Log', 'log'), inputs);

export const neg = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Neg', a => `-${a}`));
  return 0;
};

// // export const not = (handler: WebGLInferenceHandler, inputs: Tensor[]):
// //     Tensor[] => [handler.run(createElementwiseProgramInfoLoader(handler, inputs[0], glslNot()), inputs)];

export const reciprocal = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Reciprocal', a => `1.0/${a}`));
  return 0;
};

// export const relu = async(handler: WebGpuInferenceHandler, inputs: Tensor[]): Promise<Tensor[] >=>handler.run(
//     createElementwiseProgramInfoLoader(inputs[0], 'Relu', a => `max(${a}, vec4(0.0))`), inputs);

export const sigmoid = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Sigmoid', a => `(1.0 / (1.0 + exp(-${a})))`));
  return 0;
};

export const sin = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Sin', 'sin'));
  return 0;
};

export const sinh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Sinh', 'sinh'));
  return 0;
};

export const sqrt = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Sqrt', 'sqrt'));
  return 0;
};

export const tan = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Tan', 'tan'));
  return 0;
};

export const tanh = (context: ComputeContext): number => {
  context.compute(createElementwiseProgramInfoLoader(context.inputs[0], 'Tanh', 'tanh'));
  return 0;
};
