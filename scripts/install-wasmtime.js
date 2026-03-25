#!/usr/bin/env node
import { mkdir, chmod, writeFile } from 'node:fs/promises';
import { createWriteStream } from 'node:fs';
import { pipeline } from 'node:stream/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { get } from 'node:https';

const __dirname = dirname(fileURLToPath(import.meta.url));
const binDir = join(__dirname, '..', 'bin');

const WASMTIME_VERSION = 'v43.0.0';

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
  return new Promise((resolve, reject) => {
    get(url, (response) => {
      if (response.statusCode === 302 || response.statusCode === 301) {
        downloadFile(response.headers.location, dest).then(resolve).catch(reject);
        return;
      }
      if (response.statusCode !== 200) {
        reject(new Error(`Failed to download: ${response.statusCode}`));
        return;
      }
      const fileStream = createWriteStream(dest);
      pipeline(response, fileStream)
        .then(resolve)
        .catch(reject);
    }).on('error', reject);
  });
}

async function extractTarXz(archivePath, destDir) {
  const { spawn } = await import('node:child_process');
  return new Promise((resolve, reject) => {
    const tar = spawn('tar', ['xf', archivePath, '-C', destDir, '--strip-components=1']);
    tar.on('close', (code) => {
      if (code === 0) resolve();
      else reject(new Error(`tar extraction failed with code ${code}`));
    });
    tar.on('error', reject);
  });
}

async function extractZip(archivePath, destDir) {
  const { spawn } = await import('node:child_process');
  return new Promise((resolve, reject) => {
    const unzip = spawn('unzip', ['-o', archivePath, '-d', destDir]);
    unzip.on('close', (code) => {
      if (code === 0) resolve();
      else reject(new Error(`unzip failed with code ${code}`));
    });
    unzip.on('error', reject);
  });
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
