#include "xqd_world.h"


__attribute__((weak, export_name("cabi_realloc")))
void *cabi_realloc(void *ptr, size_t orig_size, size_t org_align, size_t new_size) {
  void *ret = realloc(ptr, new_size);
  if (!ret) abort();
  return ret;
}

// Component Adapters

fastly_error_t fastly_abi_init(uint64_t abi_version) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_abi_init((int64_t) (abi_version), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[36];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_uap_parse((int32_t) (*user_agent).ptr, (int32_t) (*user_agent).len, ptr);
  fastly_result_user_agent_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_user_agent_t) {
        (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) },
        (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 12))), (size_t)(*((int32_t*) (ptr + 16))) },
        (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 20))), (size_t)(*((int32_t*) (ptr + 24))) },
        (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 28))), (size_t)(*((int32_t*) (ptr + 32))) },
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_body_new(fastly_body_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_body_new(ptr);
  fastly_result_body_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_body_append((int32_t) (dest), (int32_t) (src), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_body_read((int32_t) (h), (int32_t) (chunk_size), ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf, fastly_body_write_end_t end, uint32_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_body_write((int32_t) (h), (int32_t) (*buf).ptr, (int32_t) (*buf).len, (int32_t) end, ptr);
  fastly_result_u32_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_body_close(fastly_body_handle_t h) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_body_close((int32_t) (h), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_log_endpoint_get((int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_log_endpoint_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_log_write((int32_t) (h), (int32_t) (*msg).ptr, (int32_t) (*msg).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_body_downstream_get(fastly_request_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_body_downstream_get(ptr);
  fastly_result_request_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_request_t) {
        (uint32_t) (*((int32_t*) (ptr + 4))),
        (uint32_t) (*((int32_t*) (ptr + 8))),
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_cache_override_set(fastly_request_handle_t h, fastly_http_cache_override_tag_t tag, fastly_option_u32_t *ttl, fastly_option_u32_t *stale_while_revalidate, fastly_option_string_t *sk) {
  int32_t option;
  int32_t option1;
  if ((*ttl).is_some) {
    const uint32_t *payload0 = &(*ttl).val;
    option = 1;
    option1 = (int32_t) (*payload0);
  } else {
    option = 0;
    option1 = 0;
  }
  int32_t option4;
  int32_t option5;
  if ((*stale_while_revalidate).is_some) {
    const uint32_t *payload3 = &(*stale_while_revalidate).val;
    option4 = 1;
    option5 = (int32_t) (*payload3);
  } else {
    option4 = 0;
    option5 = 0;
  }
  int32_t option8;
  int32_t option9;
  int32_t option10;
  if ((*sk).is_some) {
    const xqd_world_string_t *payload7 = &(*sk).val;
    option8 = 1;
    option9 = (int32_t) (*payload7).ptr;
    option10 = (int32_t) (*payload7).len;
  } else {
    option8 = 0;
    option9 = 0;
    option10 = 0;
  }
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_cache_override_set((int32_t) (h), tag, option, option1, option4, option5, option8, option9, option10, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_client_ip_addr(ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_client_h2_fingerprint(ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_cipher_openssl_name(ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_protocol(ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_client_hello(ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_client_certificate(ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_client_cert_verify_result(void) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_client_cert_verify_result(ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_downstream_tls_ja3_md5(ptr);
  fastly_result_list_u8_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_u8_t) { (uint8_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_new(fastly_request_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_new(ptr);
  fastly_result_request_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_list_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_names_get((int32_t) (h), ptr);
  fastly_result_list_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_string_t) { (xqd_world_string_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_value_get(fastly_request_handle_t h, xqd_world_string_t *name, fastly_option_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_value_get((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_option_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      fastly_option_string_t option;
      switch ((int32_t) (*((uint8_t*) (ptr + 4)))) {
        case 0: {
          option.is_some = false;
          break;
        }
        case 1: {
          option.is_some = true;
          option.val = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 8))), (size_t)(*((int32_t*) (ptr + 12))) };
          break;
        }
      }
      
      result.val.ok = option;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_values_get(fastly_request_handle_t h, xqd_world_string_t *name, fastly_option_list_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_values_get((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_option_list_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      fastly_option_list_string_t option;
      switch ((int32_t) (*((uint8_t*) (ptr + 4)))) {
        case 0: {
          option.is_some = false;
          break;
        }
        case 1: {
          option.is_some = true;
          option.val = (fastly_list_string_t) { (xqd_world_string_t*)(*((int32_t*) (ptr + 8))), (size_t)(*((int32_t*) (ptr + 12))) };
          break;
        }
      }
      
      result.val.ok = option;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_values_set(fastly_request_handle_t h, xqd_world_string_t *name, fastly_list_string_t *values) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_values_set((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*values).ptr, (int32_t) (*values).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name, xqd_world_string_t *value) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_insert((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*value).ptr, (int32_t) (*value).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name, xqd_world_string_t *value) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_append((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*value).ptr, (int32_t) (*value).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_header_remove((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_method_get((int32_t) (h), ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_method_set((int32_t) (h), (int32_t) (*method).ptr, (int32_t) (*method).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_uri_get((int32_t) (h), ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_uri_set((int32_t) (h), (int32_t) (*uri).ptr, (int32_t) (*uri).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_version_get((int32_t) (h), ptr);
  fastly_result_http_version_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_version_set(fastly_request_handle_t h, fastly_http_version_t version) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_version_set((int32_t) (h), (int32_t) version, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b, xqd_world_string_t *backend, fastly_response_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_send((int32_t) (h), (int32_t) (b), (int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_response_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_response_t) {
        (uint32_t) (*((int32_t*) (ptr + 4))),
        (uint32_t) (*((int32_t*) (ptr + 8))),
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b, xqd_world_string_t *backend, fastly_pending_request_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_send_async((int32_t) (h), (int32_t) (b), (int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_pending_request_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b, xqd_world_string_t *backend, fastly_pending_request_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_send_async_streaming((int32_t) (h), (int32_t) (b), (int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_pending_request_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h, fastly_option_response_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_pending_req_poll((int32_t) (h), ptr);
  fastly_result_option_response_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      fastly_option_response_t option;
      switch ((int32_t) (*((uint8_t*) (ptr + 4)))) {
        case 0: {
          option.is_some = false;
          break;
        }
        case 1: {
          option.is_some = true;
          option.val = (fastly_response_t) {
            (uint32_t) (*((int32_t*) (ptr + 8))),
            (uint32_t) (*((int32_t*) (ptr + 12))),
          };
          break;
        }
      }
      
      result.val.ok = option;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_pending_req_wait((int32_t) (h), ptr);
  fastly_result_response_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_response_t) {
        (uint32_t) (*((int32_t*) (ptr + 4))),
        (uint32_t) (*((int32_t*) (ptr + 8))),
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h, fastly_tuple2_u32_response_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_pending_req_select((int32_t) (*h).ptr, (int32_t) (*h).len, ptr);
  fastly_result_tuple2_u32_response_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_tuple2_u32_response_t) {
        (uint32_t) (*((int32_t*) (ptr + 4))),
        (fastly_response_t) {
          (uint32_t) (*((int32_t*) (ptr + 8))),
          (uint32_t) (*((int32_t*) (ptr + 12))),
        },
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_key_is_valid(bool *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_key_is_valid(ptr);
  fastly_result_bool_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_close(fastly_request_handle_t h) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_close((int32_t) (h), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h, fastly_content_encodings_t encodings) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_auto_decompress_response_set((int32_t) (h), (int32_t) encodings, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_upgrade_websocket(xqd_world_string_t *backend) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_upgrade_websocket((int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_redirect_to_websocket_proxy((int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_redirect_to_grip_proxy((int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h, fastly_framing_headers_mode_t mode) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_req_framing_headers_mode_set((int32_t) (h), (int32_t) mode, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix, xqd_world_string_t *target, fastly_dynamic_backend_config_t *config) {
  __attribute__((aligned(4)))
  uint8_t ret_area[108];
  int32_t ptr = (int32_t) &ret_area;
  *((int32_t*)(ptr + 4)) = (int32_t) (*prefix).len;
  *((int32_t*)(ptr + 0)) = (int32_t) (*prefix).ptr;
  *((int32_t*)(ptr + 12)) = (int32_t) (*target).len;
  *((int32_t*)(ptr + 8)) = (int32_t) (*target).ptr;
  if (((*config).host_override).is_some) {
    const xqd_world_string_t *payload0 = &((*config).host_override).val;
    *((int8_t*)(ptr + 16)) = 1;
    *((int32_t*)(ptr + 24)) = (int32_t) (*payload0).len;
    *((int32_t*)(ptr + 20)) = (int32_t) (*payload0).ptr;
  } else {
    *((int8_t*)(ptr + 16)) = 0;
  }
  if (((*config).connect_timeout).is_some) {
    const uint32_t *payload2 = &((*config).connect_timeout).val;
    *((int8_t*)(ptr + 28)) = 1;
    *((int32_t*)(ptr + 32)) = (int32_t) (*payload2);
  } else {
    *((int8_t*)(ptr + 28)) = 0;
  }
  if (((*config).first_byte_timeout).is_some) {
    const uint32_t *payload4 = &((*config).first_byte_timeout).val;
    *((int8_t*)(ptr + 36)) = 1;
    *((int32_t*)(ptr + 40)) = (int32_t) (*payload4);
  } else {
    *((int8_t*)(ptr + 36)) = 0;
  }
  if (((*config).between_bytes_timeout).is_some) {
    const uint32_t *payload6 = &((*config).between_bytes_timeout).val;
    *((int8_t*)(ptr + 44)) = 1;
    *((int32_t*)(ptr + 48)) = (int32_t) (*payload6);
  } else {
    *((int8_t*)(ptr + 44)) = 0;
  }
  if (((*config).use_ssl).is_some) {
    const bool *payload8 = &((*config).use_ssl).val;
    *((int8_t*)(ptr + 52)) = 1;
    *((int8_t*)(ptr + 53)) = *payload8;
  } else {
    *((int8_t*)(ptr + 52)) = 0;
  }
  if (((*config).ssl_min_version).is_some) {
    const fastly_tls_version_t *payload10 = &((*config).ssl_min_version).val;
    *((int8_t*)(ptr + 54)) = 1;
    *((int8_t*)(ptr + 55)) = (int32_t) *payload10;
  } else {
    *((int8_t*)(ptr + 54)) = 0;
  }
  if (((*config).ssl_max_version).is_some) {
    const fastly_tls_version_t *payload12 = &((*config).ssl_max_version).val;
    *((int8_t*)(ptr + 56)) = 1;
    *((int8_t*)(ptr + 57)) = (int32_t) *payload12;
  } else {
    *((int8_t*)(ptr + 56)) = 0;
  }
  if (((*config).cert_hostname).is_some) {
    const xqd_world_string_t *payload14 = &((*config).cert_hostname).val;
    *((int8_t*)(ptr + 60)) = 1;
    *((int32_t*)(ptr + 68)) = (int32_t) (*payload14).len;
    *((int32_t*)(ptr + 64)) = (int32_t) (*payload14).ptr;
  } else {
    *((int8_t*)(ptr + 60)) = 0;
  }
  if (((*config).ca_cert).is_some) {
    const xqd_world_string_t *payload16 = &((*config).ca_cert).val;
    *((int8_t*)(ptr + 72)) = 1;
    *((int32_t*)(ptr + 80)) = (int32_t) (*payload16).len;
    *((int32_t*)(ptr + 76)) = (int32_t) (*payload16).ptr;
  } else {
    *((int8_t*)(ptr + 72)) = 0;
  }
  if (((*config).ciphers).is_some) {
    const xqd_world_string_t *payload18 = &((*config).ciphers).val;
    *((int8_t*)(ptr + 84)) = 1;
    *((int32_t*)(ptr + 92)) = (int32_t) (*payload18).len;
    *((int32_t*)(ptr + 88)) = (int32_t) (*payload18).ptr;
  } else {
    *((int8_t*)(ptr + 84)) = 0;
  }
  if (((*config).sni_hostname).is_some) {
    const xqd_world_string_t *payload20 = &((*config).sni_hostname).val;
    *((int8_t*)(ptr + 96)) = 1;
    *((int32_t*)(ptr + 104)) = (int32_t) (*payload20).len;
    *((int32_t*)(ptr + 100)) = (int32_t) (*payload20).ptr;
  } else {
    *((int8_t*)(ptr + 96)) = 0;
  }
  {
    __attribute__((aligned(1)))
    uint8_t ret_area[2];
    int32_t ptr21 = (int32_t) &ret_area;
    __wasm_import_fastly_http_req_register_dynamic_backend(ptr, ptr21);
    fastly_result_void_error_t result;
    switch ((int32_t) (*((uint8_t*) (ptr21 + 0)))) {
      case 0: {
        result.is_err = false;
        break;
      }
      case 1: {
        result.is_err = true;
        result.val.err = (int32_t) (*((uint8_t*) (ptr21 + 1)));
        break;
      }
    }
    return result.is_err ? result.val.err : -1;
  }
}

fastly_error_t fastly_http_resp_new(fastly_response_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_new(ptr);
  fastly_result_response_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_list_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_names_get((int32_t) (h), ptr);
  fastly_result_list_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_list_string_t) { (xqd_world_string_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_value_get(fastly_response_handle_t h, xqd_world_string_t *name, fastly_option_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_value_get((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_option_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      fastly_option_string_t option;
      switch ((int32_t) (*((uint8_t*) (ptr + 4)))) {
        case 0: {
          option.is_some = false;
          break;
        }
        case 1: {
          option.is_some = true;
          option.val = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 8))), (size_t)(*((int32_t*) (ptr + 12))) };
          break;
        }
      }
      
      result.val.ok = option;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_values_get(fastly_response_handle_t h, xqd_world_string_t *name, fastly_option_list_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[16];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_values_get((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_option_list_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      fastly_option_list_string_t option;
      switch ((int32_t) (*((uint8_t*) (ptr + 4)))) {
        case 0: {
          option.is_some = false;
          break;
        }
        case 1: {
          option.is_some = true;
          option.val = (fastly_list_string_t) { (xqd_world_string_t*)(*((int32_t*) (ptr + 8))), (size_t)(*((int32_t*) (ptr + 12))) };
          break;
        }
      }
      
      result.val.ok = option;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_values_set(fastly_response_handle_t h, xqd_world_string_t *name, fastly_list_string_t *values) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_values_set((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*values).ptr, (int32_t) (*values).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name, xqd_world_string_t *value) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_insert((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*value).ptr, (int32_t) (*value).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name, xqd_world_string_t *value) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_append((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*value).ptr, (int32_t) (*value).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_header_remove((int32_t) (h), (int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_version_get((int32_t) (h), ptr);
  fastly_result_http_version_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_version_set(fastly_response_handle_t h, fastly_http_version_t version) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_version_set((int32_t) (h), (int32_t) version, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b, bool streaming) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_send_downstream((int32_t) (h), (int32_t) (b), streaming, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret) {
  __attribute__((aligned(2)))
  uint8_t ret_area[4];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_status_get((int32_t) (h), ptr);
  fastly_result_http_status_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint16_t) ((int32_t) (*((uint16_t*) (ptr + 2))));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 2)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_status_set((int32_t) (h), (int32_t) (status), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_close(fastly_response_handle_t h) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_close((int32_t) (h), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h, fastly_framing_headers_mode_t mode) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_http_resp_framing_headers_mode_set((int32_t) (h), (int32_t) mode, ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_dictionary_open((int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_dictionary_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key, xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_dictionary_get((int32_t) (h), (int32_t) (*key).ptr, (int32_t) (*key).len, ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_geo_lookup((int32_t) (*addr_octets).ptr, (int32_t) (*addr_octets).len, ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_kv_open(xqd_world_string_t *name, fastly_kv_store_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_kv_open((int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_kv_store_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_kv_lookup(fastly_kv_store_handle_t store, fastly_list_u8_t *key, fastly_body_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_kv_lookup((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, ptr);
  fastly_result_body_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_kv_insert(fastly_kv_store_handle_t store, fastly_list_u8_t *key, fastly_body_handle_t body_handle, uint32_t max_age, bool *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_kv_insert((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, (int32_t) (body_handle), (int32_t) (max_age), ptr);
  fastly_result_bool_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_object_store_open(xqd_world_string_t *name, fastly_object_store_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_object_store_open((int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_object_store_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_object_store_lookup(fastly_object_store_handle_t store, xqd_world_string_t *key, fastly_body_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_object_store_lookup((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, ptr);
  fastly_result_body_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store, xqd_world_string_t *key, fastly_fd_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_object_store_lookup_as_fd((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, ptr);
  fastly_result_fd_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_object_store_insert(fastly_object_store_handle_t store, xqd_world_string_t *key, fastly_body_handle_t body_handle) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_object_store_insert((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, (int32_t) (body_handle), ptr);
  fastly_result_void_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_secret_store_open(xqd_world_string_t *name, fastly_secret_store_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_secret_store_open((int32_t) (*name).ptr, (int32_t) (*name).len, ptr);
  fastly_result_secret_store_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_secret_store_get(fastly_secret_store_handle_t store, xqd_world_string_t *key, fastly_secret_handle_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_secret_store_get((int32_t) (store), (int32_t) (*key).ptr, (int32_t) (*key).len, ptr);
  fastly_result_secret_handle_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_secret_store_plaintext(fastly_secret_handle_t secret, xqd_world_string_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_secret_store_plaintext((int32_t) (secret), ptr);
  fastly_result_string_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_backend_is_healthy(xqd_world_string_t *backend, fastly_backend_health_t *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_backend_is_healthy((int32_t) (*backend).ptr, (int32_t) (*backend).len, ptr);
  fastly_result_backend_health_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms, uint32_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[8];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_async_io_select((int32_t) (*hs).ptr, (int32_t) (*hs).len, (int32_t) (timeout_ms), ptr);
  fastly_result_u32_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (uint32_t) (*((int32_t*) (ptr + 4)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret) {
  __attribute__((aligned(1)))
  uint8_t ret_area[2];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_async_io_is_ready((int32_t) (handle), ptr);
  fastly_result_bool_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 1)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

fastly_error_t fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge, fastly_purge_result_t *ret) {
  __attribute__((aligned(4)))
  uint8_t ret_area[12];
  int32_t ptr = (int32_t) &ret_area;
  __wasm_import_fastly_purge_surrogate_key((int32_t) (*surrogate_key).ptr, (int32_t) (*surrogate_key).len, soft_purge, ptr);
  fastly_result_purge_result_error_t result;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      result.is_err = false;
      result.val.ok = (fastly_purge_result_t) {
        (xqd_world_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) },
      };
      break;
    }
    case 1: {
      result.is_err = true;
      result.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }
  *ret = result.val.ok;
  return result.is_err ? result.val.err : -1;
}

extern void __component_type_object_force_link_xqd_world(void);
void __component_type_object_force_link_xqd_world_public_use_in_this_compilation_unit(void) {
  __component_type_object_force_link_xqd_world();
}
