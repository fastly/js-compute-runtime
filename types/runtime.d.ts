declare module "fastly:runtime" {
  /**
   * Get the current elapsed vCPU time in milliseconds for the current request handler
   */
  export function vCpuTime(): number;
}
