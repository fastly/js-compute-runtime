/// <reference path="../types/fastly:cache-override.d.ts" />
import { CacheOverride } from "fastly:cache-override";
import { expectType } from 'tsd';


// CacheOverride
{
  expectType<CacheOverride>(new CacheOverride("none"))
  expectType<CacheOverride>(new CacheOverride("pass"))
  expectType<CacheOverride>(new CacheOverride("override"))
  expectType<CacheOverride>(new CacheOverride("none", {ttl: undefined}))
  expectType<CacheOverride>(new CacheOverride("none", {ttl: 1}))
  expectType<CacheOverride>(new CacheOverride("none", {swr: undefined}))
  expectType<CacheOverride>(new CacheOverride("none", {swr: 1}))
  expectType<CacheOverride>(new CacheOverride("none", {surrogateKey: undefined}))
  expectType<CacheOverride>(new CacheOverride("none", {surrogateKey: 'undefined'}))
  expectType<CacheOverride>(new CacheOverride("none", {pci: undefined}))
  expectType<CacheOverride>(new CacheOverride("none", {pci: true}))
  expectType<CacheOverride>(new CacheOverride("pass", {}))
  expectType<CacheOverride>(new CacheOverride("override", {}))
  const cacheOverride = new CacheOverride('none');
  expectType<"none" | "pass" | "override">(cacheOverride.mode)
  expectType<boolean | undefined>(cacheOverride.pci)
  expectType<number | undefined>(cacheOverride.ttl)
  expectType<number | undefined>(cacheOverride.swr)
  expectType<string | undefined>(cacheOverride.surrogateKey)
}

