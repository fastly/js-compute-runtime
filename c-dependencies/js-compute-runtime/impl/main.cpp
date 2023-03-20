#include "host_interface/c-at-e.h"

int main(int argc, const char *argv[]) {
  fastly_request_t req;
  if (c_at_e_req_body_downstream_get(&req.f0, &req.f1) != 0) {
    abort();
    return 1;
  }

  js_compute_runtime_serve(static_cast<js_compute_runtime_request_t *>(static_cast<void *>(&req)));

  // Note: we deliberately skip shutdown, because it takes quite a while,
  // and serves no purpose for us.
  // TODO: investigate also skipping the destructors deliberately run in
  // wizer.h. GLOBAL = nullptr; CONTEXT = nullptr; JS_DestroyContext(cx);
  // JS_ShutDown();
  return 0;
}
