/* eslint-env serviceworker */
import { assertThrows } from './assertions.js';
import { routes } from './routes.js';

{
  routes.set('/request-upstream', async (event) => {
    /**
     * @type {Request} request
     **/
    const request = event.request;

    assertThrows(() => {
      request.headers.set('should-be', 'immutable');
    }, TypeError);

    const headers = {};
    for (const [name, value] of request.headers.entries()) {
      if (!headers[name]) {
        headers[name] = [];
      }
      headers[name].push(value);
    }
    delete headers['user-agent'];
    return new Response(JSON.stringify(headers), { headers: request.headers });
  });
  routes.set('/request-init', async () => {
    const requestHeaders = new Headers();
    requestHeaders.append('cats', 'aki');
    requestHeaders.append('cats', 'yuki');
    requestHeaders.append('dogs', 'buster');
    requestHeaders.append('numbers', '1');
    requestHeaders.append('numbers', '2');
    requestHeaders.append('numbers', '3');
    requestHeaders.append('numbers', '4');
    requestHeaders.append('numbers', '5');
    requestHeaders.append('numbers', '6');
    requestHeaders.append('numbers', '7');
    requestHeaders.append('numbers', '8');
    requestHeaders.append('numbers', '9');
    let request = new Request('https://www.exmaple.com', {
      headers: requestHeaders,
    });

    const headers = {};
    for (const [name, value] of request.headers.entries()) {
      if (!headers[name]) {
        headers[name] = [];
      }
      headers[name].push(value);
    }
    delete headers['user-agent'];
    return new Response(JSON.stringify(headers), { headers: request.headers });
  });
  routes.set('/request-direct', async () => {
    let request = new Request('https://www.exmaple.com');

    request.headers.append('cats', 'aki');
    request.headers.append('cats', 'yuki');
    request.headers.append('dogs', 'buster');
    request.headers.append('numbers', '1');
    request.headers.append('numbers', '2');
    request.headers.append('numbers', '3');
    request.headers.append('numbers', '4');
    request.headers.append('numbers', '5');
    request.headers.append('numbers', '6');
    request.headers.append('numbers', '7');
    request.headers.append('numbers', '8');
    request.headers.append('numbers', '9');
    const headers = {};
    for (const [name, value] of request.headers.entries()) {
      if (!headers[name]) {
        headers[name] = [];
      }
      headers[name].push(value);
    }
    delete headers['user-agent'];
    return new Response(JSON.stringify(headers), { headers: request.headers });
  });
}
