import { pass, ok, strictEqual, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { Shield } from 'fastly:shielding';

routes.set('/shielding/encrypted', async (event) => {
    // If you're running these tests from Sydney, this will fail
    let shield = new Shield('wsi-australia-au');
    if (shield.runningOn()) {
        return await fetch('https://http-me.glitch.me/anything', { backend: 'httpme' });
    }
    return await fetch(event.request, { backend: shield.encryptedBackend() });
});

routes.set('/shielding/unencrypted', async (event) => {
    // If you're running these tests from Sydney, this will fail
    let shield = new Shield('wsi-australia-au');
    if (shield.runningOn()) {
        return await fetch('https://http-me.glitch.me/anything', { backend: 'httpme' });
    }
    return await fetch(event.request, { backend: shield.unencryptedBackend() });
});
