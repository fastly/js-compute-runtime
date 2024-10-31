#!/usr/bin/env node

import { $ as zx } from 'zx';
import { argv } from 'node:process';

const serviceName = argv[2];

const startTime = Date.now();

if (process.env.FASTLY_API_TOKEN === undefined) {
  zx.verbose = false;
  try {
    process.env.FASTLY_API_TOKEN = String(
      await zx`fastly profile token --quiet`,
    ).trim();
  } catch {
    console.error(
      'No environment variable named FASTLY_API_TOKEN has been set and no default fastly profile exists.',
    );
    console.error(
      'In order to run the tests, either create a fastly profile using `fastly profile create` or export a fastly token under the name FASTLY_API_TOKEN',
    );
    process.exit(1);
  }
  zx.verbose = true;
}

// Setup KV Stores
{
  let stores = await (async function () {
    try {
      return JSON.parse(
        await zx`fastly kv-store list --quiet --json --token $FASTLY_API_TOKEN`,
      );
    } catch {
      return [];
    }
  })();

  const existing = stores.Data.find(
    ({ Name }) => Name === `example-test-kv-store`,
  );
  // For somereason the StarlingMonkey version of this contains "ID" instead of "StoreID"
  const STORE_ID = existing?.StoreID || existing?.ID;
  if (!STORE_ID) {
    process.env.STORE_ID = JSON.parse(
      await zx`fastly kv-store create --quiet --name='example-test-kv-store' --json --token $FASTLY_API_TOKEN`,
    ).id;
  } else {
    process.env.STORE_ID = STORE_ID;
  }
  try {
    await zx`fastly resource-link create --service-name ${serviceName} --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`;
  } catch (e) {
    if (!e.message.includes('Duplicate record')) throw e;
  }
}

await zx`fastly service-version activate --service-name ${serviceName} --version latest --token $FASTLY_API_TOKEN`;

console.log(
  `Set up has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
);
