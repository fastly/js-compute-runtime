declare module "fastly:env" {
  /**
   * Function to get the value for the provided environment variable name.
   *
   * For a list of available environment variables, see the [Fastly Developer Hub for C@E Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
   *
   * @param name The name of the environment variable
   * @returns The value of the environment variable
   */
  export function env(name: string): string;
}
