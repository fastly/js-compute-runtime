/// <reference path="../types/fastly:fanout.d.ts" />
import { createFanoutHandoff } from "fastly:fanout";
import { expectType } from 'tsd';

expectType<(request: Request, backend: string) => Response>(createFanoutHandoff)