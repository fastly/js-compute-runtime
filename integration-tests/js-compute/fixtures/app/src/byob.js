import { routes } from './routes';
routes.set('/byob', () => {
  // eslint-disable-next-line no-undef
  const stream = new ReadableStream({
    type: 'bytes',
    autoAllocateChunkSize: 1024,
    async pull(controller) {
      const view = controller.byobRequest.view;
      view[0] = 104;
      view[1] = 101;
      view[2] = 121;
      view[3] = 51;
      controller.byobRequest.respond(4);
      controller.close();
    },
  });
  return new Response(stream);
});
