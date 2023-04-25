globalThis.GLOBAL = {
  isWindow: function() { return false; },
  isWorker: function() { return true; },
  isShadowRealm: function() { return false; },
};

globalThis.Window=self;
let originalAEL = addEventListener;
addEventListener = function addEventListener_wpt(type, handler) {
  if (type == "fetch") {
    originalAEL(type, handler);
  } else {
    console.log(`Ignoring handler for event ${type}`);
  }
}

globalThis.Blob = class Blob{};
globalThis.FormData = class FormData{};
globalThis.SharedArrayBuffer = class SharedArrayBuffer{};
globalThis.MessageChannel = class MessageChannel{};
;
