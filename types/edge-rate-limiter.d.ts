declare module "fastly:edge-rate-limiter" {

  /**
   * A penalty-box that can be used with the EdgeRateLimiter or standalone for adding and checking if some entry is in the dataset.
   */
  export class PenaltyBox {
    /**
     * Open a penalty-box identified by the given name
     * @param name The name of the penalty-box to open
     */
    constructor(name: string);
    /**
     * Add entry to the penalty-box for the duration of the given timeToLive.
     *
     * Valid `timeToLive` is 1 minute to 60 minutes and `timeToLive` value is truncated to the nearest minute.
     * @param entry The entry to add
     * @param timeToLive In minutes, how long the entry should be added into the penalty-box
     */
    add(entry: string, timeToLive: number): void;
    /**
     * Check if entry is in the penalty-box.
     */
    has(entry: string): boolean;
  }

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
    lookupRate(entry: string, window: 1 | 10 | 60): number;
    /**
     * Lookup the current count for entry for a given duration
     * @param entry The entry to lookup
     * @param duration The duration to lookup alongside the entry, has to be either, 10, 20, 30, 40, 50, or 60 seconds.
     */
    lookupCount(entry: string, duration: 10 | 20 | 30 | 40 | 50 | 60): number;
  }

}
