/// <reference types="@fastly/js-compute" />

import { Region, Format } from 'fastly:image-optimizer';

addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));

async function handleRequest(event) {
  return await fetch('https://http-me.glitch.me/image-jpeg', {
    imageOptimizerOptions: {
      region: Region.UsEast,
      format: Format.PNG
    },
    backend: 'httpme'
  });
}
