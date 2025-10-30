/* eslint-env serviceworker */

import { routes } from './routes.js';
import { Region, Auto } from 'fastly:image-optimizer';
import {
    assert,
    assertThrows,
    assertRejects,
    strictEqual,
} from './assertions.js';

routes.set('/image-optimizer/test', async () => {
    const response = await fetch('https://http-me.glitch.me/image-jpeg', {
        imageOptimizerOptions: {
            region: Region.UsEast,
            auto: Auto.AVIF
        }
    });

});
