function random3Decimals() {
  return String(Math.random()).slice(0, 5);
}

let a = random3Decimals();
let b = random3Decimals();

addEventListener("fetch", event => {
  if (a === b) {
    return event.respondWith(new Response('The first 4 digits were repeated in sequential calls to Math.random() during wizening\n\n' + JSON.stringify({ a, b }, undefined, 4), { status: 500 }));
  }

  let c = random3Decimals();
  let d = random3Decimals();
  if (c === d) {
    return event.respondWith(new Response('The first 4 digits were repeated in sequential calls to Math.random() during request handling\n\n' + JSON.stringify({ c, d }, undefined, 4), { status: 500 }));
  }


  return event.respondWith(new Response(JSON.stringify({ initialisedResults: [a, b], results: [c, d] }, undefined, 4)));
});
