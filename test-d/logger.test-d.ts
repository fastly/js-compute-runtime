/// <reference path="../types/logger.d.ts" />
import { Logger } from 'fastly:logger';
import { expectError, expectType } from 'tsd';

expectError(new Logger());
expectError(Logger('example'));
expectError(Logger());
expectType<Logger>(new Logger('example'));
expectType<(message: any) => void>(new Logger('example').log);
