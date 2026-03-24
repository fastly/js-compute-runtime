declare module 'fastly:edge-rate-limiter' {
  /**
   * Provides [Edge Rate Limiting](https://docs.fastly.com/products/edge-rate-limiting) by
   * combining a {@link RateCounter} and a {@link PenaltyBox}.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @version 3.9.0
   */
  export class EdgeRateLimiter {
    /**
     * Open an EdgeRateLimiter with the given rate counter and penalty box.
     *
     * @param rateCounter The RateCounter instance to associate with this EdgeRateLimiter.
     * @param penaltyBox The PenaltyBox instance to associate with this EdgeRateLimiter.
     * @throws `TypeError` If `rateCounter` is not an instance of RateCounter.
     * or if `penaltyBox` is not an instance of PenaltyBox.
     */
    constructor(rateCounter: RateCounter, penaltyBox: PenaltyBox);
    /**
     * Increment an entry in the rate counter and check if the entry has exceeded
     * some average number of requests per second (RPS) over the given window.
     * If the entry is over the RPS limit for the window, add it to the penalty
     * box for the given `timeToLive`.
     *
     * @param entry The name of the entry to increment and check.
     * @param delta The amount to increment the entry by.
     * @param window The time period in seconds to check across.
     * @param limit The requests-per-second limit.
     * @param timeToLive How long in minutes (1–60) the entry should be added
     *   into the penalty box. Value is truncated to the nearest minute.
     * @returns `true` if the entry has exceeded the average RPS for the window,
     *   otherwise `false`.
     * @throws `TypeError` If `delta` is not a non-negative finite number,
     * if `window` is not 1, 10, or 60, if `limit` is not a non-negative finite number,
     * or if `timeToLive` is not a number between 1 and 60.
     */
    checkRate(
      entry: string,
      delta: number,
      window: 1 | 10 | 60,
      limit: number,
      timeToLive: number,
    ): boolean;
  }

  /**
   * A penalty box that can be used with {@link EdgeRateLimiter} or standalone
   * for adding and checking if an entry is in the dataset.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @version 3.9.0
   */
  export class PenaltyBox {
    /**
     * Open a penalty box identified by the given name.
     *
     * @param name The name of the penalty box to open.
     */
    constructor(name: string);
    /**
     * Add an entry to the penalty box for the duration of the given `timeToLive`.
     *
     * @param entry The entry to add.
     * @param timeToLive How long in minutes (1–60) the entry should be added
     *   into the penalty box. Value is truncated to the nearest minute.
     * @throws `TypeError` if `timeToLive` is not a number between 1 and 60.
     */
    add(entry: string, timeToLive: number): void;
    /**
     * Check if an entry is in the penalty box.
     *
     * @param entry The entry to look up.
     * @returns `true` if the entry is in the penalty box, otherwise `false`.
     */
    has(entry: string): boolean;
  }

  /**
   * A rate counter that can be used with {@link EdgeRateLimiter} or standalone
   * for counting and rate calculations.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @version 3.9.0
   */
  export class RateCounter {
    /**
     * Open a RateCounter instance with the given name.
     *
     * @param name The name of the rate counter.
     */
    constructor(name: string);
    /**
     * Increment the given entry by `delta`.
     *
     * @param entry The entry to increment.
     * @param delta The amount to increment the entry by.
     * @throws `TypeError` if `delta` is not a non-negative finite number.
     */
    increment(entry: string, delta: number): void;
    /**
     * Look up the current rate for an entry over a given window.
     *
     * @param entry The entry to look up.
     * @param window The time window in seconds to look up alongside the entry.
     * @returns The rate for the given entry and window.
     * @throws `TypeError` if `window` is not 1, 10, or 60.
     */
    lookupRate(entry: string, window: 1 | 10 | 60): number;
    /**
     * Look up the current count for an entry over a given duration.
     *
     * @param entry The entry to look up.
     * @param duration The duration in seconds to look up alongside the entry.
     * @returns The count for the given entry and duration.
     * @throws `TypeError` if `duration` is not 10, 20, 30, 40, 50, or 60.
     */
    lookupCount(entry: string, duration: 10 | 20 | 30 | 40 | 50 | 60): number;
  }
}
