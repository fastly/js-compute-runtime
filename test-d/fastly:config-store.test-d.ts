/// <reference path="../types/fastly:config-store.d.ts" />
import { ConfigStore } from "fastly:config-store";
import {expectError, expectType} from 'tsd';

// ConfigStore
{
    expectError(new ConfigStore())
    expectError(ConfigStore('example'))
    expectError(ConfigStore())
    expectType<ConfigStore>(new ConfigStore('example'))
    expectType<(key:string) => string|null>(new ConfigStore('example').get)
}