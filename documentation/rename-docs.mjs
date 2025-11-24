/**
 * The docs site relies on path names like "fastly:backend", which
 * are not compatible in Windows.
 * 
 * This script handles renaming the docs site folders to be prefixed with fastly: or not
 */

import { readdirSync, renameSync } from 'node:fs';

const subsystems = [
  'acl',
  'backend',
  'cache-override',
  'cache',
  'compute',
  'config-store',
  'device',
  'dictionary',
  'edge-rate-limiter',
  'env',
  'experimental',
  'fanout',
  'websocket',
  'geolocation',
  'kv-store',
  'logger',
  'object-store',
  'secret-store',
  'html-rewriter',
  'image-optimizer'
];

const files = readdirSync('docs');
const direction = process.argv[2] || (files.some(f => f.startsWith('fastly:')) ? 'remove-prefix' : 'add-prefix');

for (const file of files) {
  if (direction === 'remove-prefix' && file.startsWith('fastly:')) {
    if (!subsystems.includes(file.slice(7)))
      console.error(`missing subsystem - ${file.slice(7)}`);
    renameSync(`docs/${file}`, `docs/${file.slice(7)}`);
  }
  else if (subsystems.includes(file)) {
    renameSync(`docs/${file}`, `docs/fastly:${file}`);
  }
}

const versions = readdirSync('versioned_docs');
for (const version of versions) {
  const files = readdirSync(`versioned_docs/${version}`);
  for (const file of files) {
    if (direction === 'remove-prefix' && file.startsWith('fastly:')) {
      if (!subsystems.includes(file.slice(7)))
        console.error(`missing subsystem - ${file.slice(7)}`);
      renameSync(`versioned_docs/${version}/${file}`, `versioned_docs/${version}/${file.slice(7)}`);
    }
    else if (subsystems.includes(file)) {
      renameSync(`versioned_docs/${version}/${file}`, `versioned_docs/${version}/fastly:${file}`);
    }
  }
}

if (direction === 'remove-prefix')
  console.log('Renamed docs to remove prefix (for committing to git)');
else
  console.log('Renamed docs for building with fastly: prefix');
