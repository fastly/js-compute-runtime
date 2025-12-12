import { routes } from './routes.js';

function upperCase() {
  const decoder = new TextDecoder();
  const encoder = new TextEncoder();
  return new TransformStream({
    transform(chunk, controller) {
      controller.enqueue(encoder.encode(decoder.decode(chunk).toUpperCase()));
    },
  });
}

routes.set('/transform-stream/identity', () => {
  return fetch('https://http-me.fastly.dev/test?body=hello').then(
    (response) => {
      return new Response(response.body.pipeThrough(new TransformStream()));
    },
  );
});

routes.set('/transform-stream/uppercase', () => {
  return fetch('https://http-me.fastly.dev/test?body=hello').then(
    (response) => {
      return new Response(response.body.pipeThrough(upperCase()));
    },
  );
});

routes.set('/transform-stream/parallel-uppercase', () => {
  return fetch('https://http-me.fastly.dev/test?body=hello').then(
    (response) => {
      return new Response(response.body.pipeThrough(upperCase()));
    },
  );
});

// This is not a test, but the nested stream we loop back to in testing
routes.set(
  '/transform-stream/multi-stream-forwarding/nested',
  async (event) => {
    let encoder = new TextEncoder();
    let body = new TransformStream({
      start(controller) {},
      transform(chunk, controller) {
        controller.enqueue(encoder.encode(chunk));
      },
      flush(controller) {},
    });
    let writer = body.writable.getWriter();
    event.respondWith(new Response(body.readable));
    let word = new URL(event.request.url).searchParams.get('word');
    console.log(`streaming word: ${word}`);
    for (let letter of word) {
      console.log(`Writing letter ${letter}`);
      await writer.write(letter);
    }
    if (word.endsWith('.')) {
      await writer.write('\n');
    }
    await writer.close();

    // tell the route handler not to call respondWith as we already did
    return false;
  },
);

routes.set('/transform-stream/multi-stream-forwarding', async (event) => {
  let fullBody = 'This sentence will be streamed in chunks.';
  let responses = [];
  for (let word of fullBody.split(' ').join('+ ').split(' ')) {
    responses.push(
      (await fetch(`${event.request.url}/nested?word=${word}`)).body,
    );
  }
  return new Response(concatStreams(responses));
});

function concatStreams(streams) {
  let { readable, writable } = new TransformStream();
  async function iter() {
    for (let stream of streams) {
      try {
        await stream.pipeTo(writable, { preventClose: true });
      } catch (e) {
        console.error(`error during pipeline execution: ${e}`);
      }
    }
    console.log('closing writable');
    await writable.close();
  }
  iter();
  return readable;
}
