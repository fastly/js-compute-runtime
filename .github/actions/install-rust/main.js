const child_process = require('child_process');
const toolchain = process.env.INPUT_TOOLCHAIN;
const fs = require('fs');

function set_env(name, val) {
  fs.appendFileSync(process.env['GITHUB_ENV'], `${name}=${val}\n`)
}

child_process.execFileSync('rustup', ['show']);

// Deny warnings on CI to keep our code warning-free as it lands in-tree. Don't
// do this on nightly though since there's a fair amount of warning churn there.
if (!toolchain.startsWith('nightly')) {
  set_env("RUSTFLAGS", "-D warnings");
}

// Save disk space by avoiding incremental compilation, and also we don't use
// any caching so incremental wouldn't help anyway.
set_env("CARGO_INCREMENTAL", "0");

// Turn down debuginfo from 2 to 1 to help save disk space
set_env("CARGO_PROFILE_DEV_DEBUG", "1");
set_env("CARGO_PROFILE_TEST_DEBUG", "1");
