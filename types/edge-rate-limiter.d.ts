declare module "fastly:edge-rate-limiter" {

  export class EdgeRateLimiter {
    /**
     * Open a EdgeRateLimiter with the given ratecounter and penaltybox.
     */
    constructor(ratecounter: RateCounter, penaltybox: PenaltyBox);
    /**
     * Increment an entry in the rate counter and check if the client has exceeded some average number of requests per second (RPS) over the given window.
     * If the client is over the rps limit for the window, add to the penaltybox for the given timeToLive.
     * Valid timeToLive span is 1m to 1h and timeToLive value is truncated to the nearest minute.
     */
    checkRate(entry: string, delta: number, window: [1, 10, 60], limit: number, timeToLive: number): boolean;
  }

  /**
   * A penalty-box that can be used with the EdgeRateLimiter or stand alone for adding and checking if some entry is in the data set.
   */
  export class PenaltyBox {
    /**
     * Open a penalty-box identified by the given name
     * @param name The name of the penalty-box to open
     */
    constructor(name: string);
    /**
     * Add entry to the penalty-box for the duration of the given timeToLive.
     * Valid timeToLive span is 1m to 1h and timeToLive value is truncated to the nearest minute.
     * @param entry The entry to add
     * @param timeToLive How long the entry should be added into the penalty-box
     */
    add(entry: string, timeToLive: number): void;
    /**
     * Check if entry is in the penaltybox.
     */
    has(entry: string): boolean;
  }

  /**
   * A rate counter that can be used with a EdgeRateLimiter or stand alone for counting and rate calculations
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
     * @param window The window to lookup alongside the entry
     */
    lookupRate(entry: string, window: [1, 10, 60]): number;
    /**
     * Lookup the current count for entry for a given duration
     * @param entry The entry to lookup
     * @param duration The duration to lookup alongside the entry
     */
    lookupCount(entry: string, duration: [10, 20, 30, 40, 50, 60]): number;
  }

}
