#!/usr/bin/env node

// Clean up any stale resources and services left behind by the integration tests

import { $ as zx } from 'zx';
import { getPrefixes } from './env.js';

const {
  SERVICE_PREFIX,
  ACL_PREFIX,
  DICTIONARY_PREFIX,
  CONFIG_STORE_PREFIX,
  KV_STORE_PREFIX,
  SECRET_STORE_PREFIX,
} = getPrefixes();

function existingListIds(stores, prefix) {
  return stores
    .filter(
      (store) =>
        store.name?.startsWith(prefix) || store.Name?.startsWith(prefix),
    )
    .map((existing) => existing.id || existing.StoreID);
}

const startTime = Date.now();

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

async function removeConfigStores(links, serviceIds) {
  const stores = JSON.parse(
    await zx`fastly config-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );

  let STORE_IDS = existingListIds(stores, DICTIONARY_PREFIX);
  for (const STORE_ID of STORE_IDS) {
    await zx`fastly config-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
  }

  STORE_IDS = existingListIds(stores, CONFIG_STORE_PREFIX);
  for (const STORE_ID of STORE_IDS) {
    try {
      await zx`fastly config-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
}

async function removeKVStores() {
  const stores = JSON.parse(
    await zx`fastly kv-store list --quiet --json --token $FASTLY_API_TOKEN`,
  ).Data;

  let STORE_IDS = existingListIds(stores, KV_STORE_PREFIX);
  for (const STORE_ID of STORE_IDS) {
    await zx`fastly kv-store delete --store-id=${STORE_ID} --quiet --all -y --token $FASTLY_API_TOKEN`;
  }
}

async function removeSecretStores(serviceIds) {
  const stores = JSON.parse(
    await zx`fastly secret-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );
  if (!stores) {
    return;
  }
  const STORE_IDS = existingListIds(stores, SECRET_STORE_PREFIX);
  for (const STORE_ID of STORE_IDS) {
    try {
      await zx`fastly secret-store delete --store-id=${STORE_ID}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
}

async function removeAcls(serviceIds) {
  const ACL_IDS = existingListIds(
    JSON.parse(
      await zx`fastly compute acl list-acls --quiet --json --token $FASTLY_API_TOKEN`,
    ).data,
    ACL_PREFIX,
  );

  for (const ACL_ID of ACL_IDS) {
    await zx`fastly compute acl delete --acl-id=${ACL_ID}  --token $FASTLY_API_TOKEN`;
  }
}

async function getServiceIds() {
  let services = JSON.parse(
    await zx`fastly service list --token $FASTLY_API_TOKEN --json`,
  );
  return services
    .filter(({ Name }) => Name.startsWith(SERVICE_PREFIX))
    .map((service) => service.ServiceID);
}

async function removeServices(serviceIds) {
  for (const serviceId of serviceIds) {
    await zx`fastly service delete --force --service-id=${serviceId}`;
  }
}

async function removeLinks(serviceIds) {
  for (const serviceId of serviceIds) {
    const links = JSON.parse(
      await zx`fastly resource-link list --service-id=${serviceId} --quiet --json --version latest --token $FASTLY_API_TOKEN`,
    );
    for (const link of links) {
      await zx`fastly resource-link delete --version latest --autoclone --id=${link.id} --service-id=${link.service_id}  --token $FASTLY_API_TOKEN`;
      await zx`fastly service-version activate --version latest --service-id=${link.service_id} --token $FASTLY_API_TOKEN`;
    }
  }
}

const serviceIds = await getServiceIds();
await removeLinks(serviceIds);
await Promise.all([
  removeConfigStores(serviceIds),
  removeKVStores(),
  removeSecretStores(serviceIds),
  removeAcls(serviceIds),
]);
await removeServices(serviceIds);

console.log(
  `Tear down has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
);
