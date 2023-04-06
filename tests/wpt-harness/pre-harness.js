globalThis.GLOBAL = {
  isWindow: function() { return false; },
  isWorker: function() { return true; },
  isShadowRealm: function() { return false; },
};

function ReadableByteStreamController () {}

globalThis.Window=self;
let originalAEL = addEventListener;
addEventListener = function addEventListener_wpt(type, handler) {
  if (type == "fetch") {
    originalAEL(type, handler);
  } else {
    console.log(`Ignoring handler for event ${type}`);
  }
}

class Blob {}
class FormData {}
;
