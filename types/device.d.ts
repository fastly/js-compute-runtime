declare module 'fastly:device' {
  /**
   * Provides device detection based on User-Agent strings.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @version 3.10.0
   */
  class Device {
    /**
     * Look up the data associated with a particular User-Agent string.
     * If there is data associated with the User-Agent, a `Device` instance is returned.
     * Otherwise, `null` is returned.
     *
     * @param useragent The User-Agent string to look up.
     * @throws `TypeError` if the provided User-Agent string is empty.
     */
    static lookup(useragent: string): Device | null;

    /**
     * The name of the device, or `null` if no name is known.
     */
    get name(): string | null;
    /**
     * The brand of the device, which may be different from the manufacturer.
     * `null` if no brand is known.
     */
    get brand(): string | null;
    /**
     * The model of the device, or `null` if no model is known.
     */
    get model(): string | null;
    /**
     * A string representation of the device's primary platform hardware,
     * or `null` if not known.
     *
     * The most commonly used device types are also identified via boolean
     * properties. Because a device may have multiple device types and this
     * property only has the primary type, we recommend using the boolean
     * properties for logic and this string representation for logging.
     */
    get hardwareType(): string | null;
    /**
     * Whether the device is a desktop web browser, or `null` if not known.
     */
    get isDesktop(): boolean | null;
    /**
     * Whether the device is a video game console, or `null` if not known.
     */
    get isGameConsole(): boolean | null;
    /**
     * Whether the device is a media player (like Blu-ray players, iPod
     * devices, and smart speakers such as Amazon Echo), or `null` if not known.
     */
    get isMediaPlayer(): boolean | null;
    /**
     * Whether the device is a mobile phone, or `null` if not known.
     */
    get isMobile(): boolean | null;
    /**
     * Whether the device is a smart TV, or `null` if not known.
     */
    get isSmartTV(): boolean | null;
    /**
     * Whether the device is a tablet (like an iPad), or `null` if not known.
     */
    get isTablet(): boolean | null;
    /**
     * Whether the device's screen is touch sensitive, or `null` if not known.
     */
    get isTouchscreen(): boolean | null;
    /**
     * Whether the device is a bot, or `null` if not known.
     *
     * @version 3.39.0
     */
    get isBot(): boolean | null;

    /**
     * Returns a JSON representation of the Device object.
     *
     * To get a JSON string, you can use `JSON.stringify(device)` directly;
     * it will call `toJSON()` automatically.
     */
    toJSON(): Object;
  }
}
