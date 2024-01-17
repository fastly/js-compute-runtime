#include "host_interface/fastly.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"

int main() { return 0; }

bool exports_fastly_compute_at_edge_reactor_serve(fastly_compute_at_edge_reactor_request_t *req) {
  host_api::Request request{host_api::HttpReq{req->f0}, host_api::HttpBody{req->f1}};
  return reactor_main(request);
}
