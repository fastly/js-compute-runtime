declare module "fastly:env" {
  /**
   * Class to handle environment variables for the C@E service.
   *
   * For additional references, see the [Fastly Developer Hub for C@E Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
   */
  class Env {
    constructor();

    /**
     * Function to get the environment variable value, for the provided environment variable name.
     *
     * @param name The name of the environment variable
     * @returns the value of the environemnt variable
     */
    get(name: string): string;
  }
}
