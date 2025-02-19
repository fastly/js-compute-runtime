#!/usr/bin/env node

import { $ as zx } from 'zx';
import { argv } from 'node:process';
import { getEnv } from './env.js';

const serviceId = argv[2];
const serviceName = argv[3];

const { DICTIONARY_NAME, CONFIG_STORE_NAME, KV_STORE_NAME, SECRET_STORE_NAME } =
  getEnv(serviceName);

function existingListId(stores, existingName) {
  const existing = stores.find(({ name }) => name === existingName);
  return existing?.id || existing?.StoreID;
}

const startTime = Date.now();

zx.verbose = false;
if (process.env.FASTLY_API_TOKEN === undefined) {
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
}
const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;
zx.verbose = true;

async function removeConfigStores() {
  const stores = JSON.parse(
    await zx`fastly config-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );
  const links = JSON.parse(
    await zx`fastly resource-link list --quiet --json --version latest --token $FASTLY_API_TOKEN`,
  );

  let STORE_ID = existingListId(stores, DICTIONARY_NAME);
  if (STORE_ID) {
    const LINK_ID = links.find(
      ({ resource_id }) => resource_id == STORE_ID,
    )?.id;
    if (LINK_ID) {
      try {
        await zx`fastly resource-link delete --version latest --autoclone --id=${LINK_ID}  --token $FASTLY_API_TOKEN`;
        await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`;
      } catch {}
    }
    try {
      await zx`fastly config-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }

  STORE_ID = existingListId(stores, CONFIG_STORE_NAME);
  if (STORE_ID) {
    const LINK_ID = links.find(
      ({ resource_id }) => resource_id == STORE_ID,
    )?.id;
    if (LINK_ID) {
      await zx`fastly resource-link delete --version latest --autoclone --id=${LINK_ID}  --token $FASTLY_API_TOKEN`;
      await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`;
    }
    await zx`fastly config-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
  }
}

async function removeKVStore() {
  let stores = (
    await fetch('https://api.fastly.com/resources/stores/object', {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
        Accept: 'application/json',
        'Fastly-Key': FASTLY_API_TOKEN,
      },
    }).then((res) => res.json())
  ).Data;

  let STORE_ID = existingListId(stores, KV_STORE_NAME);
  if (STORE_ID) {
    await fetch(`https://api.fastly.com/resources/stores/object/${STORE_ID}`, {
      method: 'DELETE',
      headers: {
        'Fastly-Key': FASTLY_API_TOKEN,
      },
    });
  }
}

async function removeSecretStore() {
  const stores = JSON.parse(
    await zx`fastly secret-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );
  const links = JSON.parse(
    await zx`fastly resource-link list --quiet --json --version latest --token $FASTLY_API_TOKEN`,
  );

  const STORE_ID = existingListId(stores, SECRET_STORE_NAME);
  if (STORE_ID) {
    const LINK_ID = links.find(
      ({ resource_id }) => resource_id == STORE_ID,
    )?.id;
    if (LINK_ID) {
      await zx`fastly resource-link delete --version latest --autoclone --id=${LINK_ID}  --token $FASTLY_API_TOKEN`;
      await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`;
    }
    await zx`fastly secret-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
  }
}

async function removeAcl() {
  const links = JSON.parse(
    await zx`fastly resource-link list --quiet --json --version latest --token $FASTLY_API_TOKEN`,
  );
  const LINK_ID = links.find(({ resource_id }) => resource_id == STORE_ID)?.id;
  if (LINK_ID) {
    await zx`fastly resource-link delete --version latest --autoclone --id=${LINK_ID}  --token $FASTLY_API_TOKEN`;
    await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`;
  }

  const ACL_ID = existingListId(
    JSON.parse(
      await zx`fastly compute acl acl-list --quiet --json --token $FASTLY_API_TOKEN`,
    ).data,
    ACL_NAME,
  );
  if (ACL_ID) {
    await zx`fastly compute acl delete --acl-id=${ACL_ID}  --token $FASTLY_API_TOKEN`;
  }
}

await removeConfigStores();
await removeKVStore();
await removeSecretStore();
await removeAcl();

console.log(
  `Tear down has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
);
