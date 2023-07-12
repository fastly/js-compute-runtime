#include "host_interface/fastly.h"

int main(int argc, const char *argv[]) {
  fastly_compute_at_edge_fastly_request_t req;
  if (fastly::req_body_downstream_get(&req.f0, &req.f1) != 0) {
    abort();
    return 1;
  }

  compute_at_edge_serve(static_cast<compute_at_edge_request_t *>(static_cast<void *>(&req)));

  // Note: we deliberately skip shutdown, because it takes quite a while,
  // and serves no purpose for us.
  // TODO: investigate also skipping the destructors deliberately run in
  // wizer.h. GLOBAL = nullptr; CONTEXT = nullptr; JS_DestroyContext(cx);
  // JS_ShutDown();
  return 0;
}
