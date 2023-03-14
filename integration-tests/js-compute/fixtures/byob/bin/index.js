/* eslint-env serviceworker */
/* global fastly */
function strictEqual (a, b) {
  if (a !== b) {
    console.log('LHS: ', a);
    console.log('RHS: ', b);
    throw new Error(`Assertion failure`);
  }
}

addEventListener("fetch", async (event) => {
  let cnt = 0;
  const stream = new ReadableStream({
    // type: 'bytes',
    pull (controller) {
      cnt++;
      const view = controller.byobRequest?.view;
      if (view) {
        view[0] = 104;
        view[1] = 101;
        view[2] = 121;
        controller.byobRequest.respond(3);
      } else {
        controller.enqueue(new Uint8Array([104, 101, 106]));
      }
      if (cnt === 3)
        return controller.close();
    }
  });
  event.respondWith(new Response(stream));

  {
    let cnt = 0;
    const stream = new ReadableStream({
      type: 'bytes',
      autoAllocateChunkSize: 1024,
      start (controller) {
      },
      pull (controller) {
        cnt++;
        const view = controller.byobRequest?.view;
        if (cnt < 3) {
          view[0] = 1;
          view[1] = 2;
          view[2] = 3;
          controller.byobRequest.respond(3);
        } else if (cnt == 3) {
          controller.enqueue(new Uint8Array([1,2,4]));
        }
        if (cnt === 3)
          return controller.close();
      }
    });
  
    const byobReader = stream.getReader();
    let buf;
    let result;
    let offset = 0;
    while (true) {
      if (buf && cnt < 3) {
        const output = new Uint8Array(buf);
        strictEqual(output[0], 1);
        strictEqual(output[1], 2);
        strictEqual(output[2], 3);
        strictEqual(output[3], 0);
        strictEqual(output.byteLength, 1024);
      }
      result = await byobReader.read();
      if (result.done)
        break;
      offset += result.value.byteLength;
      buf = result.value.buffer;
    }

    strictEqual(cnt, 3);
    const output = new Uint8Array(buf);
    strictEqual(output[0], 1);
    strictEqual(output[1], 2);
    strictEqual(output[2], 4);
    strictEqual(output[3], undefined);
    strictEqual(output.byteLength, 3);
    console.log('Passed 1');
  }

  {
    let cnt = 0;
    const stream = new ReadableStream({
      type: 'bytes',
      start (controller) {
      },
      pull (controller) {
        cnt++;
        const view = controller.byobRequest?.view;
        if (view) {
          view[0] = 1;
          view[1] = 2;
          view[2] = 3;
          controller.byobRequest.respond(3);
        }
        if (cnt === 3)
          return controller.close();
      }
    });
  
    const byobReader = stream.getReader({ mode: 'byob' });
    let buf = new ArrayBuffer(10);
    let result;
    let offset = 0;
    do {
      result = await byobReader.read(new Uint8Array(buf, offset));
      offset += result.value.byteLength;
      buf = result.value.buffer;
    } while(!result.done);
  
    const out = new Uint8Array(buf);
    if (out[0] === 1 && out[1] === 2 && out[2] === 3 && out[3] === 1 && out[4] === 2 && out[5] === 3 && out[6] === 1 && out[7] === 2 && out[8] === 3) {
        console.log('Passed 2');
    }
  }
});
