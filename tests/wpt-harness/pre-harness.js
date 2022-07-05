let originalAEL = addEventListener;
addEventListener = function addEventListener_wpt(type, handler) {
  if (type == "fetch") {
    originalAEL(type, handler);
  } else {
    console.log(`Ignoring handler for event ${type}`);
  }
}

class FormData {}
;
