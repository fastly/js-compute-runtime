/* eslint-env serviceworker */
/* global fastly */
function strictEqual (a, b, description) {
  if (a !== b) {
    if (description)
      console.log('-- ' + description + ' --');
    console.log('ACTUAL: ', a);
    console.log('EXPECTED: ', b);
    throw new Error(`Assertion failure`);
  }
}

addEventListener("fetch", async (event) => {
  const stream = new ReadableStream({
    type: 'bytes',
    autoAllocateChunkSize: 1024,
    async pull (controller) {
      const view = controller.byobRequest.view;
      view[0] = 104;
      view[1] = 101;
      view[2] = 121;
      view[3] = 51;
      controller.byobRequest.respond(4);
      controller.close();
    }
  });
  event.respondWith(new Response(stream));
});
