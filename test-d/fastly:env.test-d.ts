/// <reference path="../types/fastly:env.d.ts" />
import { Env } from "fastly:env";
import {expectError, expectType} from 'tsd';

// Env
{
  expectError(Env())
  expectError(Env('example'))
  expectError(new Env('example'))
  expectType<Env>(new Env())
  expectType<(key:string) => string>(new Env().get)
}