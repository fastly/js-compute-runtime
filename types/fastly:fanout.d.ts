declare module "fastly:fanout" {
  /**
   *
   * @param request The request to pass through Fanout.
   *
   * The name of the backend that Fanout should send the request to.
   * The name has to be between 1 and 254 characters inclusive.
   * @param backend The name of the environment variable
   *
   * @throws {TypeError} Throws a TypeError if the backend is not valid. I.E. The backend is null, undefined, an empty string or a string with more than 254 characters.
   */
  function createFanoutHandoff(request: Request, backend: string): Response;
}
