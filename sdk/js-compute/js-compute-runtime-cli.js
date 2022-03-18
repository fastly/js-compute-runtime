#!/usr/bin/env node

const { spawnSync } = require("child_process");
const fs = require("fs");
const path = require("path");
const os = require("os");

function getPlatform() {
  const osMap = {
    Windows_NT: {
      name: "windows",
      archs: {
        x64: "x86_64",
      },
    },
    Linux: {
      name: "linux",
      archs: {
        x64: "x86_64",
      },
    },
    Darwin: {
      name: "macos",
      archs: {
        arm64: "x86_64",
        x64: "x86_64",
      },
    },
  };
  const type = os.type();
  const arch = os.arch();
  const osDef = osMap[type];
  if (!osDef || !osDef.archs[arch]) {
    throw new Error(`Unsupported platform: ${type} ${arch}`);
  }
  return {
    type: osDef.name,
    arch: osDef.archs[arch],
  };
}

let platform = getPlatform();
let binaryPath = path.join(
  path.dirname(fs.realpathSync(__filename)),
  "bin",
  `${platform.type}-${platform.arch}`,
  "js-compute-runtime"
);
if (platform.type === "windows") {
  binaryPath += ".exe";
}

const result = spawnSync(binaryPath, process.argv.slice(2), {
  stdio: "inherit",
});
process.exitCode = result.status;
console.log(" ");
