/// <reference path="../types/fastly:experimental.d.ts" />
import { setBaseURL, setDefaultBackend, enableDebugLogging, includeBytes } from "fastly:experimental";
import { expectType } from 'tsd';

expectType<(path: string) => Uint8Array>(includeBytes)
expectType<(enabled: boolean) => void>(enableDebugLogging)
expectType<(base: URL | null | undefined) => void>(setBaseURL)
expectType<(backend: string) => void>(setDefaultBackend)
