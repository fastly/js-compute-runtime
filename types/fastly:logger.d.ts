declare module "fastly:logger" {
  class Logger {
    /**
      * Creates a new Logger instance for the given [named log endpoint](https://developer.fastly.com/learning/integrations/logging).
      *
      * **Note**: Can only be used when processing requests, not during build-time initialization.
      */
    constructor(name: string);
    /**
     * Send the given message, converted to a string, to this Logger instance's endpoint
     */
    log(message: any): void;
  }

}
