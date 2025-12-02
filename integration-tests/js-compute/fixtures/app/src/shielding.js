import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { Shield } from 'fastly:shielding';

routes.set('/shielding/encrypted', async (event) => {
    // If you're running these tests from Sydney, this will fail
    let shield = new Shield('wsi-australia-au');
    if (shield.runningOn()) {
        return await fetch('https://http-me.glitch.me/anything', { backend: 'httpme' });
    }
    let resp = await fetch(event.request, { backend: shield.encryptedBackend() });
    assert(resp.headers.get('x-cache').includes(','), true, 'headers[x-cache].includes(",")');
});

routes.set('/shielding/invalid-shield', () => {
    assertThrows(new Shield('i-am-not-a-real-shield'));
});