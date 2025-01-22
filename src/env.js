// env.js
function parseEnvPair(pair) {
  const trimmedPair = pair.trim();

  // If no '=', treat as a variable to inherit
  if (!trimmedPair.includes('=')) {
    const value = process.env[trimmedPair];
    if (value === undefined) {
      throw new Error(`Environment variable ${trimmedPair} is not defined`);
    }
    console.warn(
      `Writing ${trimmedPair} environment variable into the runtime from the current process environment`,
    );
    return [trimmedPair, value];
  }

  const matches = trimmedPair.match(/^([^=]+)=(.*)$/);
  if (!matches) {
    throw new Error(
      `Invalid environment variable format: ${trimmedPair}\nMust be in format KEY=VALUE or an existing environment variable name`,
    );
  }

  const key = matches[1].trim();
  const value = matches[2];

  if (!key) {
    throw new Error(
      `Invalid environment variable format: ${trimmedPair}\nMust be in format KEY=VALUE or an existing environment variable name`,
    );
  }

  return [key, value];
}

function parseEnvString(envString) {
  const result = {};

  // Split on unescaped commas and filter out empty strings
  const pairs = envString
    .split(/(?<!\\),/) // Split on commas that aren't preceded by backslash
    .map((s) => s.replace(/\\,/g, ',')) // Replace escaped commas with regular commas
    .filter(Boolean);

  // Parse each pair into the result object
  for (const pair of pairs) {
    const [key, value] = parseEnvPair(pair);
    result[key] = value;
  }

  return result;
}

export class EnvParser {
  constructor() {
    this.env = {};
  }

  /**
   * Parse environment variables string, which can be either KEY=VALUE pairs
   * or names of environment variables to inherit
   * @param {string} value - The environment variable string to parse
   */
  parse(value) {
    if (!value) {
      throw new Error(
        'Invalid environment variable format: \nMust be in format KEY=VALUE or an existing environment variable name',
      );
    }
    const newEnv = parseEnvString(value);
    this.env = { ...this.env, ...newEnv };
  }

  /**
   * Get the parsed environment variables
   * @returns {Object} The environment variables object
   */
  getEnv() {
    return this.env;
  }
}
