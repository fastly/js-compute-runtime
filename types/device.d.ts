declare module 'fastly:device' {
  class Device {
    /**
     * Look up the data associated with a particular User-Agent string.
     * If there is data associated with the User-Agent, a `Device` instance is returned.
     * Otherwise, `null` is returned.
     * @param useragent The User-Agent string to look up
     */
    static lookup(useragent: string): Device | null;

    /**
     * The name of the device.
     * If no name is known, the value will be `null`.
     */
    get name(): string | null;
    /**
     * The brand of the device, possibly different from the manufacturer of that device.
     * If no brand is known, the value will be `null`.
     */
    get brand(): string | null;
    /**
     * The model of the device.
     * If no model is known, the value will be `null`.
     */
    get model(): string | null;
    /**
     * A string representation of the device's primary platform hardware.
     * The most commonly used device types are also identified via boolean variables.
     * Because a device may have multiple device types and this variable only has the primary type,
     * we recommend using the boolean variables for logic and using this string representation for logging.
     *
     * If primary platform hardware is not known, the value will be `null`.
     */
    get hardwareType(): string | null;
    /**
     * Either a boolean stating if the device is a desktop web browser, or `null` if the this is not known.
     */
    get isDesktop(): boolean | null;
    /**
     * Either a boolean stating if the device is a video game console, or `null` if the this is not known.
     */
    get isGameConsole(): boolean | null;
    /**
     * Either a boolean stating if the device is a media player (like Blu-ray players, iPod devices, and smart speakers such as Amazon Echo), or `null` if the this is not known.
     */
    get isMediaPlayer(): boolean | null;
    /**
     * Either a boolean stating if the device is a mobile phone, or `null` if the this is not known.
     */
    get isMobile(): boolean | null;
    /**
     * Either a boolean stating if the device is a smart TV, or `null` if the this is not known.
     */
    get isSmartTV(): boolean | null;
    /**
     * Either a boolean stating if the device is a tablet (like an iPad), or `null` if the this is not known.
     */
    get isTablet(): boolean | null;
    /**
     * Either a boolean stating if the device's screen is touch sensitive, or `null` if the this is not known.
     */
    get isTouchscreen(): boolean | null;
    /**
     * Either a boolean stating if the device is a bot, or `null` if the this is not known.
     */
    get isBot(): boolean | null;

    /**
     * The `toJSON()` method of the Device interface is a serializer; it returns a JSON representation of the Device object.
     *
     * To get a JSON string, you can use `JSON.stringify(device)` directly; it will call `toJSON()` automatically.
     */
    toJSON(): Object;
  }
}
