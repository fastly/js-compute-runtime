import test from "brittle";
import { getBinPath } from "get-bin-path";
import { prepareEnvironment } from "@jakechampion/cli-testing-library";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
const __dirname = dirname(fileURLToPath(import.meta.url));

const packageJson = readFileSync(join(__dirname, "../../package.json"), {
  encoding: "utf-8",
});
const version = JSON.parse(packageJson).version;

const cli = await getBinPath({ name: "js-compute" });

test("--version should return version number on stdout and zero exit code", async function (t) {
  const { execute, cleanup } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });
  const { code, stdout, stderr } = await execute(
    process.execPath,
    `${cli} --version`,
  );

  t.is(code, 0);
  t.alike(stdout, [`js-compute-runtime-cli.js ${version}`]);
  t.alike(stderr, []);
});

test("-V should return version number on stdout and zero exit code", async function (t) {
  const { execute, cleanup } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });
  const { code, stdout, stderr } = await execute(process.execPath, `${cli} -V`);

  t.is(code, 0);
  t.alike(stdout, [`js-compute-runtime-cli.js ${version}`]);
  t.alike(stderr, []);
});
