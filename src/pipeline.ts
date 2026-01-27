// From https://github.com/harmony7/js-async-pipeline

import { resolve } from 'node:path';

export type PipelineOpts<TValue> = {
  beforeStep?: (
    args: TValue,
    index: number,
    arr: PipelineStep<TValue>[],
  ) => void | PromiseLike<void>;
  afterStep?: (
    args: TValue,
    index: number,
    arr: PipelineStep<TValue>[],
  ) => void | PromiseLike<void>;
};

export type PipelineStep<TValue> = (
  acc: TValue,
  index: number,
  arr: PipelineStep<TValue>[],
) => TValue | PromiseLike<TValue>;

export async function pipeline<TValue>(
  steps: PipelineStep<TValue>[],
  initialValue: TValue,
  opts?: PipelineOpts<TValue>,
): Promise<TValue> {
  let val = initialValue;
  for (const [index, step] of steps.entries()) {
    await opts?.beforeStep?.call(opts, val, index, steps);
    val = await step(val, index, steps);
    await opts?.afterStep?.call(opts, val, index, steps);
  }
  return val;
}
