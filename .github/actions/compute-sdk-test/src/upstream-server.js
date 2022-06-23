import http from 'http';

// Upstream Server - A local node HTTP Server that was can assign as a backend
// To verify Upstream requests sent by a wasm module
class UpstreamServer {

  constructor() {
    this.setExpectedNumberOfRequests(0);
    this.server = null;
  }

  setExpectedNumberOfRequests(expectedNumberOfRequests) {
    this.expectedNumberOfRequests = expectedNumberOfRequests;
    this.localUpstreamRequestNumber = 0;
    this.skippedViceroyCheck = false;
    this.upstreamServerResolve;
    this.upstreamServerReject;
    this.upstreamServerPromise = new Promise((resolve, reject) => {
      this.upstreamServerResolve = resolve;
      this.upstreamServerReject = reject;
    });
  }

  async waitForExpectedNumberOfRequests(timeout) {
    if (this.localUpstreamRequestNumber >= this.expectedNumberOfRequests) {
      return;
    }

    if (!timeout) {
      timeout = 1000 * 15;
    }
    console.log(`Waiting for ${timeout / 1000} seconds any other pending upstream requests...`);

    let rejectTimeout = setTimeout(() => {
      this.upstreamServerReject();
    }, timeout);

    await this.upstreamServerPromise;

    clearTimeout(rejectTimeout);

    console.log('Upstream requests are finished!');
  }

  listen(port, asyncRequestCallback) {
    this.server = http.createServer(async (req, res) => {

      console.log('Got an upstream request:');
      const { rawHeaders, httpVersion, method, socket, url } = req;
      console.log(
        JSON.stringify({
          timestamp: Date.now(),
          rawHeaders,
          httpVersion,
          method,
          url
        }, null, 2)
      );

      // Skip the first request, since it is viceroy checking if the server is up:
      if (!this.skippedViceroyCheck && 
        this.localUpstreamRequestNumber === 0 && 
        req.method === "GET" &&
        req.url === "/") {
        this.skippedViceroyCheck = true;
        res.write('Ok!');
        res.end();
        return;
      }

      // Run our request callback
      await asyncRequestCallback(this.localUpstreamRequestNumber, req, res);

      // Increment our request number, and check if we handled all the expected upstream requests
      this.localUpstreamRequestNumber++;
      if (this.expectedNumberOfRequests === 0) {
        this.upstreamServerResolve();
      } else if (this.expectedNumberOfRequests === this.localUpstreamRequestNumber) {
        this.upstreamServerResolve();
      }

      if (!res.headersSent) {
        res.write('Ok!');
        res.end();
      }
    });
    this.server.listen(port);
  }

  async close() {
    if (this.server) {
      await this.waitForExpectedNumberOfRequests();
      this.server.close();
    }
  }
};

export default UpstreamServer;
