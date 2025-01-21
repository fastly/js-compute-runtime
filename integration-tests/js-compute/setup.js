#!/usr/bin/env node

import { $ as zx } from 'zx';
import { argv } from 'node:process';

const serviceName = argv[2];

const startTime = Date.now();

const CONFIG_STORE_NAME_1 = `aZ1 __ 2${serviceName ? '__' + serviceName.replace(/-/g, '_') : ''}`;
const CONFIG_STORE_NAME_2 = `testconfig${serviceName ? '__' + serviceName.replace(/-/g, '_') : ''}`;
const KV_STORE_NAME = `example-test-kv-store${serviceName ? '--' + serviceName : ''}`;
const SECRET_STORE_NAME = `example-test-secret-store${serviceName ? '--' + serviceName : ''}`;

function existingStoreId(stores, existingName) {
  const existing = stores.find(
    ({ Name, name }) => name === existingName || Name === existingName,
  );
  return existing?.id || existing?.StoreID;
}

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
const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;

async function setupConfigStores() {
  let stores = await (async function () {
    try {
      return JSON.parse(
        await zx`fastly config-store list --quiet --json --token $FASTLY_API_TOKEN`,
      );
    } catch {
      return [];
    }
  })();

  let STORE_ID = existingStoreId(stores, CONFIG_STORE_NAME_1);
  if (!STORE_ID) {
    console.log(`Creating new config store ${CONFIG_STORE_NAME_1}`);
    process.env.STORE_ID = JSON.parse(
      await zx`fastly config-store create --quiet --name=${CONFIG_STORE_NAME_1} --json --token $FASTLY_API_TOKEN`,
    ).id;
  } else {
    console.log(`Using existing config store ${CONFIG_STORE_NAME_1}`);
    process.env.STORE_ID = STORE_ID;
  }
  await zx`echo -n 'https://twitter.com/fastly' | fastly config-store-entry update --upsert --key twitter --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`;
  try {
    await zx`fastly resource-link create --service-name ${serviceName} --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`;
  } catch (e) {
    if (!e.message.includes('Duplicate record')) throw e;
  }

  STORE_ID = existingStoreId(stores, CONFIG_STORE_NAME_2);
  if (!STORE_ID) {
    console.log(`Creating new config store ${CONFIG_STORE_NAME_2}`);
    process.env.STORE_ID = JSON.parse(
      await zx`fastly config-store create --quiet --name=${CONFIG_STORE_NAME_2} --json --token $FASTLY_API_TOKEN`,
    ).id;
  } else {
    console.log(`Using existing config store ${CONFIG_STORE_NAME_2}`);
    process.env.STORE_ID = STORE_ID;
  }
  await zx`echo -n 'https://twitter.com/fastly' | fastly config-store-entry update --upsert --key twitter --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`;
  try {
    await zx`fastly resource-link create --service-name ${serviceName} --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`;
  } catch (e) {
    if (!e.message.includes('Duplicate record')) throw e;
  }
}

async function setupKVStore() {
  let stores = await (async function () {
    try {
      return JSON.parse(
        await zx`fastly kv-store list --quiet --json --token $FASTLY_API_TOKEN`,
      ).Data;
    } catch {
      return [];
    }
  })();

  let STORE_ID = existingStoreId(stores, KV_STORE_NAME);
  if (STORE_ID) {
    // it is possible for KV store to return a KV store that actually isn't available
    // so test the KV store works before continuing
    const res = await fetch(
      `https://api.fastly.com/resources/stores/kv/${STORE_ID}/keys?limit=1`,
      {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json',
          Accept: 'application/json',
          'Fastly-Key': FASTLY_API_TOKEN,
        },
      },
    );
    console.log(res);
    if (!res.ok) {
      STORE_ID = null;
    }
  }
  if (!STORE_ID) {
    console.log(`Creating new KV store ${KV_STORE_NAME}`);
    process.env.STORE_ID = JSON.parse(
      await zx`fastly kv-store create --quiet --name=${KV_STORE_NAME} --json --token $FASTLY_API_TOKEN`,
    ).StoreID;
  } else {
    console.log(`Using existing KV store ${KV_STORE_NAME}`);
    process.env.STORE_ID = STORE_ID;
  }
  try {
    await zx`fastly resource-link create --service-name ${serviceName} --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`;
  } catch (e) {
    if (!e.message.includes('Duplicate record')) throw e;
  }
}

async function setupSecretStore() {
  let stores = await (async function () {
    try {
      return JSON.parse(
        await zx`fastly secret-store list --quiet --json --token $FASTLY_API_TOKEN`,
      );
    } catch {
      return [];
    }
  })();
  const STORE_ID = existingStoreId(stores, SECRET_STORE_NAME);
  if (!STORE_ID) {
    console.log(`Creating new secret store ${SECRET_STORE_NAME}`);
    process.env.STORE_ID = JSON.parse(
      await zx`fastly secret-store create --quiet --name=${SECRET_STORE_NAME} --json --token $FASTLY_API_TOKEN`,
    ).id;
  } else {
    console.log(`Using existing secret store ${SECRET_STORE_NAME}`);
    process.env.STORE_ID = STORE_ID;
  }
  await zx`echo -n 'This is also some secret data' | fastly secret-store-entry create --recreate-allow --name first --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`;
  let key =
    'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa';
  await zx`echo -n 'This is some secret data' | fastly secret-store-entry create --recreate-allow --name ${key} --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`;
  try {
    await zx`fastly resource-link create --service-name ${serviceName} --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`;
  } catch (e) {
    if (!e.message.includes('Duplicate record')) throw e;
  }
}

await setupConfigStores();
await setupKVStore();
await setupSecretStore();

await zx`fastly service-version activate --service-name ${serviceName} --version latest --token $FASTLY_API_TOKEN`;

console.log(
  `Set up has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
);
