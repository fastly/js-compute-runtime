#include "host_interface/fastly.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"

int main(int argc, const char *argv[]) {
  host_api::Request req;

  if (fastly::req_body_downstream_get(&req.req.handle, &req.body.handle) != 0) {
    abort();
    return 1;
  }

  reactor_main(req);

  // Note: we deliberately skip shutdown, because it takes quite a while,
  // and serves no purpose for us.
  // TODO: investigate also skipping the destructors deliberately run in
  // wizer.h. GLOBAL = nullptr; CONTEXT = nullptr; JS_DestroyContext(cx);
  // JS_ShutDown();
  return 0;
}
