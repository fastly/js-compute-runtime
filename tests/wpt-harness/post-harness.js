fastly.enableDebugLogging(true);
fastly.defaultBackend = "wpt";

let completionPromise = new Promise((resolve, reject) => {
    add_completion_callback(function(tests, harness_status, asserts) {
      resolve({tests, harness_status, asserts});
    });
});

setup({ explicit_done: true });

async function handleRequest(event) {
  let url = new URL(event.request.url);
  let input = `http://web-platform.test:8000${url.pathname}${url.search}`;
  fastly.baseURL = new URL(input);
  globalThis.location = fastly.baseURL;
  try {
    let response = await fetch(input);
    let testSource = await response.text();
    testSource += "\n//# sourceURL=" + url.pathname;

    let scripts = [];

    for (let [_, path] of testSource.matchAll(/META: *script=(.+)/g)) {
      let metaSource = await loadMetaScript(path, input);
      scripts.push(metaSource);
    }

    scripts.push(testSource);
    evalAllScripts(scripts);
    done();

    let {tests, harness_status, asserts} = await completionPromise;

    return new Response(JSON.stringify(tests, null, 2), { headers: { "content-encoding" : "application/json" } });
  } catch (e) {
    console.log(`error: ${e}, stack:\n${e.stack}`);
    return new Response(`{
      "error": {
        "message": ${JSON.stringify(e.message)},
        "stack": ${JSON.stringify(e.stack)}
      }
    }`, { status: 500 });
  }
};

function evalAllScripts(wpt_test_scripts) {
  for (wpt_test_script of wpt_test_scripts) {
    eval(wpt_test_script);
  }
}

async function loadMetaScript(path, base) {
  let response = await fetch(path);
  let metaSource = await response.text();
  // Somewhat annoyingly, the WPT harness includes META scripts as <script src=[path]> tags,
  // which don't create their own scope for `const` and `let bindings, as `eval` does.
  // That means that running them in `eval` doesn't make any `const` and `let` bound values
  // available to code outside of the current `eval`, which breaks various WPT files.
  //
  // Short of introducing a host call for evaluating code in the same way a `<script>` tag does
  // the only way to get around that is to replace all these bindings with `var`. Which is
  // an ugly hack, but works reasonably well.
  let lines = metaSource.split("\n");
  lines = lines.map(line => {
    if (line.indexOf("const ") == 0) {
      return "var " + line.substr(6);
    }
    if (line.indexOf("let ") == 0) {
      return "var " + line.substr(4);
    }
    return line;
  });
  metaSource = lines.join("\n");
  metaSource += "\n//# sourceURL=" + path;
  return metaSource;
}

addEventListener("fetch", event => event.respondWith(handleRequest(event)));

{
  let timeout_id = 0;
  let cleared_ids = new Set();

  function setTimeout(func, delay, ...args) {
    return run_timeout(func, delay, args, false);
  }

  function setInterval(func, delay, ...args) {
    return run_timeout(func, delay, args, true);
  }

  function run_timeout(func, delay, args, repeat) {
    let id = timeout_id++;
    let iterations = 100;
    function wait() {
      if (cleared_ids.has(id)) {
        return;
      }
      if (iterations--) {
        return Promise.resolve().then(wait);
      }
      try {
        func(...args);
      } catch (e) {
        console.log(`Error caught running ${repeat ? "setInterval" : "setTimout"} callback: ${e}`);
      }

      if (repeat) {
        iterations = 100;
        wait();
      }
    }
    wait();
    return id;
  }

  function clearTimeout(id) {
    cleared_ids.add(id);
  }

  function clearInterval(id) {
    cleared_ids.add(id);
  }
}

self.GLOBAL = {
  isWindow: () => false,
  isWorker: () => true,
};
