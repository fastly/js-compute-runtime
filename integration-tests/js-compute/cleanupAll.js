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

function existingList(stores, prefix) {
  return stores.filter(
    (store) => store.name?.startsWith(prefix) || store.Name?.startsWith(prefix),
  );
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

async function removeConfigStores(links, services) {
  console.log('Removing config stores...');
  const stores = JSON.parse(
    await zx`fastly config-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );

  let dictionaries = existingList(stores, DICTIONARY_PREFIX);
  for (const dictionary of dictionaries) {
    console.log(`\tDeleting dictionary ${dictionary.name}`);
    try {
      await zx`fastly config-store delete --store-id=${dictionary.id}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
  console.log('All dictionaries removed');

  let configStores = existingList(stores, CONFIG_STORE_PREFIX);
  for (const store of configStores) {
    console.log(`\tDeleting config store ${store.name}`);
    try {
      await zx`fastly config-store delete --store-id=${store.id}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
  console.log('All config stores removed');
}

async function removeKVStores() {
  console.log('Removing KV stores...');
  const stores = JSON.parse(
    await zx`fastly kv-store list --quiet --json --token $FASTLY_API_TOKEN`,
  ).Data;

  let kvStores = existingList(stores, KV_STORE_PREFIX);
  for (const store of kvStores) {
    console.log(`\tDeleting KV store ${store.Name}`);
    try {
      await zx`fastly kv-store delete --store-id=${store.StoreID} --quiet --all -y --token $FASTLY_API_TOKEN`;
    } catch {}
  }
  console.log('All KV stores removed');
}

async function removeSecretStores(services) {
  console.log('Removing secret stores...');
  const stores = JSON.parse(
    await zx`fastly secret-store list --quiet --json --token $FASTLY_API_TOKEN`,
  );
  if (!stores) {
    return;
  }
  const secretStores = existingList(stores, SECRET_STORE_PREFIX);
  for (const store of secretStores) {
    console.log(`\tDeleting secret store ${store.name}`);
    try {
      await zx`fastly secret-store delete --store-id=${store.id}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
  console.log('All secret stores removed');
}

async function removeAcls(services) {
  console.log('Removing ACLs...');
  const acls = existingList(
    JSON.parse(
      await zx`fastly compute acl list-acls --quiet --json --token $FASTLY_API_TOKEN`,
    ).data,
    ACL_PREFIX,
  );

  for (const acl of acls) {
    console.log(`\tDeleting acl ${acl.name}`);
    try {
      await zx`fastly compute acl delete --acl-id=${acl.id}  --token $FASTLY_API_TOKEN`;
    } catch {}
  }
  console.log('All ACLs removed');
}

async function getServices() {
  console.log('Getting services...');
  let services = JSON.parse(
    await zx`fastly service list --token $FASTLY_API_TOKEN --json`,
  );
  services = services.filter(({ Name }) => Name.startsWith(SERVICE_PREFIX));
  console.log('Services to delete:');
  for (const service of services) {
    console.log('\t', service.Name);
  }
  return services;
}

async function removeServices(services) {
  console.log('Removing services...');
  for (const service of services) {
    console.log(`\tDeleting service ${service.Name}`);
    await zx`fastly service delete --force --service-id=${service.ServiceID}`;
  }
  console.log('ALl services removed');
}

async function removeLinks(services) {
  console.log('Removing links...');
  for (const service of services) {
    const links = JSON.parse(
      await zx`fastly resource-link list --service-id=${service.ServiceID} --quiet --json --version latest --token $FASTLY_API_TOKEN`,
    );
    for (const link of links) {
      console.log(
        `\tDeleting link between service ${service.Name} and resource ${link.name}`,
      );
      await zx`fastly resource-link delete --version latest --autoclone --id=${link.id} --service-id=${link.service_id}  --token $FASTLY_API_TOKEN`;
      await zx`fastly service-version activate --version latest --service-id=${link.service_id} --token $FASTLY_API_TOKEN`;
    }
  }
  console.log('All links removed');
}

const services = await getServices();
await removeLinks(services);
await removeConfigStores(services);
await removeKVStores();
await removeSecretStores(services);
await removeAcls(services);
await removeServices(services);

console.log(
  `Cleanup has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
);
