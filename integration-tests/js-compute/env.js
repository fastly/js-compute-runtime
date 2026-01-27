export const GLOBAL_PREFIX = 'js_integration_test_';

function applyGlobalPrefix(map) {
  for (const key in map) {
    map[key] = GLOBAL_PREFIX + map[key];
  }
  return map;
}

export function getEnv(serviceName) {
  return applyGlobalPrefix({
    DICTIONARY_NAME: `aZ2__2${serviceName ? '__' + serviceName.replace(/-/g, '_') : ''}`,
    CONFIG_STORE_NAME: `config${serviceName ? '__' + serviceName.replace(/-/g, '_') : ''}`,
    KV_STORE_NAME: `kv-store${serviceName ? '--' + serviceName : ''}`,
    SECRET_STORE_NAME: `secret-store${serviceName ? '--' + serviceName : ''}`,
    ACL_NAME: `acl${serviceName ? '__' + serviceName.replace(/-/g, '_') : ''}`,
  });
}

export function getPrefixes() {
  return applyGlobalPrefix({
    SERVICE_PREFIX: `app-`,
    DICTIONARY_PREFIX: `aZ2__2`,
    CONFIG_STORE_PREFIX: `config`,
    KV_STORE_PREFIX: `kv-store`,
    SECRET_STORE_PREFIX: `secret-store`,
    ACL_PREFIX: `acl`,
  });
}
