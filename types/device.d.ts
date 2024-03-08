declare module "fastly:device" {
  class Device {
    static lookup(useragent: string): Device | null;

    get name(): string | null;
    get brand(): string | null;
    get model(): string | null;
    get hardwareType(): string | null;
    get isDesktop(): boolean | null;
    get isGameConsole(): boolean | null;
    get isMediaPlayer(): boolean | null;
    get isMobile(): boolean | null;
    get isSmartTV(): boolean | null;
    get isTablet(): boolean | null;
    get isTouchscreen(): boolean | null;
  }
}
