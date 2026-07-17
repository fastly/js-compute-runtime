/// <reference path="../types/shielding.d.ts" />
import { Shield } from 'fastly:shielding';
import { expectType, expectError } from 'tsd';

// Shielding
{
  expectError(Shield());
  expectType<Shield>(new Shield('dub-dublin-ie'));
}
