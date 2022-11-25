/// <reference path="../types/fastly:dictionary.d.ts" />
import { Dictionary } from "fastly:dictionary";
import {expectError, expectType} from 'tsd';

// Dictionary
{
  expectError(new Dictionary())
  expectError(Dictionary('example'))
  expectError(Dictionary())
  expectType<Dictionary>(new Dictionary('example'))
  expectType<(key:string) => string>(new Dictionary('example').get)
}