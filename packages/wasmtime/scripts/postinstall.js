#!/usr/bin/env node
import { mkdir, chmod, readFile } from 'node:fs/promises';
import { createReadStream, createWriteStream } from 'node:fs';
import { Readable } from 'node:stream';
import { pipeline } from 'node:stream/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { extract } from 'tar';
import lzma from 'lzma-native';
import AdmZip from 'adm-zip';

const __dirname = dirname(fileURLToPath(import.meta.url));
const binDir = join(__dirname, '..', 'bin');

const packageJson = JSON.parse(
  await readFile(join(__dirname, '..', 'package.json'), 'utf-8')
);
const WASMTIME_VERSION = `v${packageJson.version}`;

function getPlatformInfo() {
  const platform = process.platform;
  const arch = process.arch;

  if (platform === 'darwin' && arch === 'x64') {
    return { file: `wasmtime-${WASMTIME_VERSION}-x86_64-macos.tar.xz`, binary: 'wasmtime' };
  }
  if (platform === 'darwin' && arch === 'arm64') {
    return { file: `wasmtime-${WASMTIME_VERSION}-aarch64-macos.tar.xz`, binary: 'wasmtime' };
  }
  if (platform === 'linux' && arch === 'x64') {
    return { file: `wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz`, binary: 'wasmtime' };
  }
  if (platform === 'linux' && arch === 'arm64') {
    return { file: `wasmtime-${WASMTIME_VERSION}-aarch64-linux.tar.xz`, binary: 'wasmtime' };
  }
  if (platform === 'win32' && arch === 'x64') {
    return { file: `wasmtime-${WASMTIME_VERSION}-x86_64-windows.zip`, binary: 'wasmtime.exe' };
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
    createReadStream(archivePath),
    lzma.createDecompressor(),
    extract({
      cwd: destDir,
      strip: 1,
    })
  );
}

async function extractZip(archivePath, destDir) {
  new AdmZip(archivePath).extractAllTo(destDir, true);
}

async function installWasmtime() {
  try {
    const { file, binary } = getPlatformInfo();
    const url = `https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/${file}`;
    
    console.log(`Downloading wasmtime ${WASMTIME_VERSION} for ${process.platform}-${process.arch}...`);
    
    await mkdir(binDir, { recursive: true });
    
    const archivePath = join(binDir, file);
    await downloadFile(url, archivePath);
    
    console.log('Extracting...');
    if (file.endsWith('.tar.xz')) {
      await extractTarXz(archivePath, binDir);
    } else if (file.endsWith('.zip')) {
      await extractZip(archivePath, binDir);
    }
    
    const binaryPath = join(binDir, binary);
    await chmod(binaryPath, 0o755);
    
    console.log('✓ wasmtime installed successfully');
  } catch (error) {
    console.error('Failed to install wasmtime:', error.message);
    console.error('You may need to install wasmtime manually: https://wasmtime.dev/');
    process.exit(1);
  }
}

installWasmtime();
