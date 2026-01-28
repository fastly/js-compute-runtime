#!/usr/bin/env node

import { readFileSync, writeFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));

const versionsPath = join(__dirname, 'versions.json');
const versions = JSON.parse(readFileSync(versionsPath, 'utf-8'));

// Find latest version for each major version (versions.json is already sorted newest first)
const latestByMajor = {};
for (const version of versions) {
  const major = version.split('.')[0];
  if (!latestByMajor[major]) {
    latestByMajor[major] = version;
  }
}

// Latest major redirects to /docs/ (no version in path)
const latestMajor = Math.max(...Object.keys(latestByMajor).map(Number));
latestByMajor[latestMajor] = null;

const outputPath = join(__dirname, 'app', 'version-redirects.json');
writeFileSync(outputPath, JSON.stringify({ latestByMajor }, null, 2));
