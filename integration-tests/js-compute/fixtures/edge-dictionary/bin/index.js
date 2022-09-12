addEventListener("fetch", (event) => {
  let asDictionary = new Dictionary("edge_dictionary");
  let twitterValue = asDictionary.get("twitter");
  if (twitterValue) {
    event.respondWith(new Response(twitterValue));
  } else {
    event.respondWith(new Response("twitter key does not exist", {
      status: 500,
    }));
  }
});
