#!/usr/bin/env node
import {
  mkdir,
  chmod,
  mkdtemp,
  readFile,
  readdir,
  rename,
  rm,
} from 'node:fs/promises';
import { createReadStream, createWriteStream, existsSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { Readable } from 'node:stream';
import { pipeline } from 'node:stream/promises';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { extract } from 'tar-fs';
import xz from 'xz-decompress';
import AdmZip from 'adm-zip';

const __dirname = dirname(fileURLToPath(import.meta.url));
const binDir = join(__dirname, '..', 'vendor', 'wasmtime', 'bin');

function getPlatformInfo(platform, arch) {
  if (platform === 'darwin' && arch === 'x64') {
    return { archive: 'x86_64-macos.tar.xz', binary: 'wasmtime' };
  }
  if (platform === 'darwin' && arch === 'arm64') {
    return { archive: 'aarch64-macos.tar.xz', binary: 'wasmtime' };
  }
  if (platform === 'linux' && arch === 'x64') {
    return { archive: 'x86_64-linux.tar.xz', binary: 'wasmtime' };
  }
  if (platform === 'linux' && arch === 'arm64') {
    return { archive: 'aarch64-linux.tar.xz', binary: 'wasmtime' };
  }
  if (platform === 'win32' && arch === 'x64') {
    return { archive: 'x86_64-windows.zip', binary: 'wasmtime.exe' };
  }

  throw new Error(`Unsupported platform: ${platform}-${arch}`);
}

async function downloadFile(url, dest) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Failed to download: ${response.status}`);
  }
  const fileStream = createWriteStream(dest);
  await pipeline(Readable.fromWeb(response.body), fileStream);
}

async function extractTarXz(archivePath, destDir) {
  await pipeline(
    Readable.fromWeb(
      new xz.XzReadableStream(Readable.toWeb(createReadStream(archivePath))),
    ),
    extract(destDir, {
      strip: 1,
    }),
  );
}

async function extractZipWithStrip(archivePath, destDir) {
  const tempDir = join(destDir, '.extract-temp');

  const zip = new AdmZip(archivePath);
  zip.extractAllTo(tempDir);

  const entries = await readdir(tempDir);
  if (entries.length !== 1) {
    throw new Error('Expected single root directory in zip file');
  }

  const rootDir = join(tempDir, entries[0]);
  const contents = await readdir(rootDir);

  for (const item of contents) {
    await rename(join(rootDir, item), join(destDir, item));
  }

  await rm(tempDir, { recursive: true });
}

export async function installWasmtime({
  destination = binDir,
  platform = process.platform,
  arch = process.arch,
} = {}) {
  const { archive, binary } = getPlatformInfo(platform, arch);
  const binaryPath = join(destination, binary);

  if (existsSync(binaryPath)) {
    if (platform !== 'win32') {
      await chmod(binaryPath, 0o755);
    }
    return;
  }

  const { wasmtime } = JSON.parse(
    await readFile(join(__dirname, '..', 'package.json'), 'utf8'),
  );
  const version = `v${wasmtime.version}`;
  const expectedHash = wasmtime.sha256[archive];
  if (!expectedHash) {
    throw new Error(`No Wasmtime checksum configured for ${archive}`);
  }
  const file = `wasmtime-${version}-${archive}`;
  const url = `https://github.com/bytecodealliance/wasmtime/releases/download/${version}/${file}`;

  console.log(`Downloading wasmtime ${version} for ${platform}-${arch}...`);

  await mkdir(destination, { recursive: true });
  const tempDir = await mkdtemp(join(destination, '.wasmtime-install-'));
  try {
    const archivePath = join(tempDir, file);
    await downloadFile(url, archivePath);
    const hash = createHash('sha256');
    for await (const chunk of createReadStream(archivePath)) {
      hash.update(chunk);
    }
    if (hash.digest('hex') !== expectedHash) {
      throw new Error(`SHA-256 checksum mismatch for ${file}`);
    }

    console.log('Extracting...');
    if (file.endsWith('.tar.xz')) {
      await extractTarXz(archivePath, tempDir);
    } else {
      await extractZipWithStrip(archivePath, tempDir);
    }

    const extractedBinary = join(tempDir, binary);
    if (platform !== 'win32') {
      await chmod(extractedBinary, 0o755);
    }
    await rename(extractedBinary, binaryPath);

    console.log('✓ wasmtime installed successfully');
  } finally {
    await rm(tempDir, { recursive: true, force: true });
  }
}

if (
  process.argv[1] &&
  resolve(process.argv[1]) === fileURLToPath(import.meta.url)
) {
  if (process.env.FASTLY_SKIP_WASMTIME_INSTALL === '1') {
    console.log(
      'Skipping Wasmtime installation (FASTLY_SKIP_WASMTIME_INSTALL=1).',
    );
  } else {
    installWasmtime().catch((error) => {
      console.error('Failed to install wasmtime:', error.message);
      console.error(
        'See the Wasmtime installation instructions in the @fastly/js-compute README.',
      );
      process.exitCode = 1;
    });
  }
}
