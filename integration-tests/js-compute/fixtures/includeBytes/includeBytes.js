const message = fastly.includeBytes("message.txt");

addEventListener("fetch", (event) => {
  event.respondWith(new Response(message));
});
