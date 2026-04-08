import { cosmiconfig } from 'cosmiconfig';

const additiveOptionsMap = {
  env: '--env',
};

const strictOptionsMap = {
  enableAOT: '--enable-aot',
  aotCache: '--aot-cache',
  enableHttpCache: '--enable-http-cache',
  enableExperimentalHighResolutionTimeMethods:
    '--enable-experimental-high-resolution-time-methods',
  enableExperimentalTopLevelAwait: '--enable-experimental-top-level-await',
  enableStackTraces: '--enable-stack-traces',
  excludeSources: '--exclude-sources',
  debugIntermediateFiles: '--debug-intermediate-files',
  debugBuild: '--debug-build',
  engineWasm: '--engine-wasm',
  wevalBin: '--weval-bin',
};

export async function readConfigFileAndCliArguments(cliArgs: string[]) {
  const explorer = cosmiconfig('fastlycompute');
  const result = await explorer.search();
  if (!result?.config) {
    return cliArgs;
  }

  const config = result.config as Record<
    string,
    string | boolean | object | (string | boolean | object)[]
  >;
  const synthesizedArgs: string[] = [];

  // --- Loop 1: Additive Options (Array-Normalized) ---
  for (const [configKey, flag] of Object.entries(additiveOptionsMap)) {
    const val = config[configKey];
    if (val === undefined || val === null) continue;

    // Wrap in an array if it isn't one already
    const items = (Array.isArray(val) ? val : [val]) as (
      | string
      | boolean
      | object
    )[];

    for (const item of items) {
      if (typeof item === 'object' && item !== null) {
        // Handle: { "FOO": "bar" } -> --env FOO=bar
        for (const [k, v] of Object.entries(item)) {
          synthesizedArgs.push(flag, `${k}=${v}`);
        }
      } else {
        // Handle: "A,B" -> --env A,B
        synthesizedArgs.push(flag, String(item));
      }
    }
  }

  // --- Loop 2: Strict Options (Override Check) ---
  for (const [configKey, flag] of Object.entries(strictOptionsMap)) {
    const val = config[configKey];
    if (val === undefined || val === null) continue;

    const isOverridden = cliArgs.some(
      (arg) => arg === flag || arg.startsWith(`${flag}=`),
    );

    if (!isOverridden) {
      if (typeof val === 'boolean' && val) {
        synthesizedArgs.push(flag);
      } else if (typeof val === 'string' || typeof val === 'number') {
        synthesizedArgs.push(flag, String(val));
      }
    }
  }

  return [...synthesizedArgs, ...cliArgs];
}
