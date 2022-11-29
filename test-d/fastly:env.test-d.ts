/// <reference path="../types/fastly:env.d.ts" />
import { env } from "fastly:env";
import { expectType } from 'tsd';

expectType<(key: string) => string>(env)