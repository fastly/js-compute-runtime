addEventListener("fetch", (event) => {
  let config = new ConfigStore("testconfig");
  let twitterValue = config.get("twitter");
  if (twitterValue) {
    event.respondWith(new Response(twitterValue));
  } else {
    event.respondWith(new Response("twitter key does not exist", {
      status: 500,
    }));
  }
});
