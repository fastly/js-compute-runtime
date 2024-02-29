declare module "fastly:edge-rate-limiter" {

  /**
   * A rate counter that can be used with a EdgeRateLimiter or standalone for counting and rate calculations
   */
  export class RateCounter {
    /**
     * Open a RateCounter instance with the given name
     * @param name The name of the rate-counter
     */
    constructor(name: string);
    /**
     * Increment the `entry` by `delta`
     * @param entry The entry to increment
     * @param delta The amount to increment the `entry` by
     */
    increment(entry: string, delta: number): void;
    /**
     * Lookup the current rate for entry for a given window
     * @param entry The entry to lookup
     * @param window The window to lookup alongside the entry, has to be either 1 second, 10 seconds, or 60 seconds
     */
    lookupRate(entry: string, window: [1, 10, 60]): number;
    /**
     * Lookup the current count for entry for a given duration
     * @param entry The entry to lookup
     * @param duration The duration to lookup alongside the entry, has to be either, 10, 20, 30, 40, 50, or 60 seconds.
     */
    lookupCount(entry: string, duration: [10, 20, 30, 40, 50, 60]): number;
  }

}
