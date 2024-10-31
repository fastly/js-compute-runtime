/// <reference path="../types/compute.d.ts" />
import { vCpuTime, purgeSurrogateKey } from 'fastly:compute';
import { expectType } from 'tsd';

// Compute
{
  expectType<number>(vCpuTime());
  expectType<boolean>(purgeSurrogateKey('boo'));
}
