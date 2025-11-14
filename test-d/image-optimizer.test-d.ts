/// <reference path="../types/image-optimizer.d.ts" />
import {
  ImageOptimizerOptions,
  optionsToQueryString,
} from 'fastly:image-optimizer';
import { expectType } from 'tsd';

expectType<(options: ImageOptimizerOptions) => string>(optionsToQueryString);
