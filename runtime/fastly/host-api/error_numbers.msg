/*
 * Our own version of spidermonkey/js/friend/ErrorNumbers.msg
 * where we can add our own custom error messages for use within the runtime
 */

/*
 * This is our JavaScript error message file.
 *
 * The format for each JS error message is:
 *
 * MSG_DEF(<SYMBOLIC_NAME>, <ARGUMENT_COUNT>, <EXCEPTION_NAME>,
 *         <FORMAT_STRING>)
 *
 * where ;
 * <SYMBOLIC_NAME> is a legal C identifer that will be used in the
 * JS engine source.
 *
 * <ARGUMENT_COUNT> is an integer literal specifying the total number of
 * replaceable arguments in the following format string.
 *
 * <EXCEPTION_NAME> is an enum JSExnType value, defined in js/ErrorReport.h.
 *
 * <FORMAT_STRING> is a string literal, optionally containing sequences
 * {X} where X  is an integer representing the argument number that will
 * be replaced with a string value when the error is reported.
 *
 * e.g.
 *
 * MSG_DEF(JSMSG_NOT_A_SUBSPECIES, 2, JSEXN_TYPEERROR,
 *         "{0} is not a member of the {1} family")
 *
 * can be used:
 *
 * JS_ReportErrorNumberASCII(JSMSG_NOT_A_SUBSPECIES, "Rhino", "Monkey");
 *
 * to report:
 *
 * "TypeError: Rhino is not a member of the Monkey family"
 */

// clang-format off
MSG_DEF(JSMSG_NOT_AN_ERROR,                                    0, JSEXN_ERR, "<Error #0 is reserved>")
MSG_DEF(JSMSG_ACL_NAME_NOT_STRING,                             0, JSEXN_TYPEERR, "Acl open: name must be a string")
MSG_DEF(JSMSG_ACL_NAME_TOO_LONG,                               0, JSEXN_TYPEERR, "Acl open: name can not be more than 254 characters")
MSG_DEF(JSMSG_ACL_NAME_EMPTY,                                  0, JSEXN_TYPEERR, "Acl open: name can not be an empty string")
MSG_DEF(JSMSG_ACL_NOT_FOUND,                                   1, JSEXN_TYPEERR, "Acl open: \"{0}\" acl not found")
MSG_DEF(JSMSG_CONFIG_STORE_DOES_NOT_EXIST,                     1, JSEXN_TYPEERR, "ConfigStore constructor: No ConfigStore named '{0}' exists")
MSG_DEF(JSMSG_CONFIG_STORE_KEY_EMPTY,                          0, JSEXN_TYPEERR, "ConfigStore key can not be an empty string")
MSG_DEF(JSMSG_CONFIG_STORE_KEY_TOO_LONG,                       0, JSEXN_TYPEERR, "ConfigStore key can not be more than 255 characters")
MSG_DEF(JSMSG_CONFIG_STORE_NAME_CONTAINS_INVALID_CHARACTER,    0, JSEXN_TYPEERR, "ConfigStore constructor: name can contain only ascii alphanumeric characters, underscores, and ascii whitespace")
MSG_DEF(JSMSG_CONFIG_STORE_NAME_EMPTY,                         0, JSEXN_TYPEERR, "ConfigStore constructor: name can not be an empty string")
MSG_DEF(JSMSG_CONFIG_STORE_NAME_START_WITH_ASCII_ALPHA,        0, JSEXN_TYPEERR, "ConfigStore constructor: name must start with an ascii alpabetical character")
MSG_DEF(JSMSG_CONFIG_STORE_NAME_TOO_LONG,                      0, JSEXN_TYPEERR, "ConfigStore constructor: name can not be more than 255 characters")
MSG_DEF(JSMSG_DICTIONARY_DOES_NOT_EXIST,                       1, JSEXN_TYPEERR, "Dictionary constructor: No Dictionary named '{0}' exists")
MSG_DEF(JSMSG_DICTIONARY_KEY_EMPTY,                            0, JSEXN_TYPEERR, "Dictionary key can not be an empty string")
MSG_DEF(JSMSG_DICTIONARY_KEY_TOO_LONG,                         0, JSEXN_TYPEERR, "Dictionary key can not be more than 255 characters")
MSG_DEF(JSMSG_DICTIONARY_NAME_CONTAINS_INVALID_CHARACTER,      0, JSEXN_TYPEERR, "Dictionary constructor: name can contain only ascii alphanumeric characters, underscores, and ascii whitespace")
MSG_DEF(JSMSG_DICTIONARY_NAME_EMPTY,                           0, JSEXN_TYPEERR, "Dictionary constructor: name can not be an empty string")
MSG_DEF(JSMSG_DICTIONARY_NAME_START_WITH_ASCII_ALPHA,          0, JSEXN_TYPEERR, "Dictionary constructor: name must start with an ascii alpabetical character")
MSG_DEF(JSMSG_DICTIONARY_NAME_TOO_LONG,                        0, JSEXN_TYPEERR, "Dictionary constructor: name can not be more than 255 characters")
MSG_DEF(JSMSG_KV_STORE_NAME_EMPTY,                             0, JSEXN_TYPEERR, "KVStore constructor: name can not be an empty string")
MSG_DEF(JSMSG_KV_STORE_NAME_TOO_LONG,                          0, JSEXN_TYPEERR, "KVStore constructor: name can not be more than 255 characters")
MSG_DEF(JSMSG_KV_STORE_NAME_NO_CONTROL_CHARACTERS,             0, JSEXN_TYPEERR, "KVStore constructor: name can not contain control characters (\\u0000-\\u001F)")
MSG_DEF(JSMSG_KV_STORE_DOES_NOT_EXIST,                         1, JSEXN_TYPEERR, "KVStore constructor: No KVStore named '{0}' exists")
MSG_DEF(JSMSG_KV_STORE_KEY_EMPTY,                              0, JSEXN_TYPEERR, "KVStore key can not be an empty string")
MSG_DEF(JSMSG_KV_STORE_KEY_TOO_LONG,                           0, JSEXN_TYPEERR, "KVStore key can not be more than 1024 characters")
MSG_DEF(JSMSG_KV_STORE_KEY_INVALID_CHARACTER,                  1, JSEXN_TYPEERR, "KVStore key can not contain {0} character")
MSG_DEF(JSMSG_KV_STORE_KEY_ACME,                               0, JSEXN_TYPEERR, "KVStore key can not start with .well-known/acme-challenge/")
MSG_DEF(JSMSG_KV_STORE_KEY_RELATIVE,                           0, JSEXN_TYPEERR, "KVStore key can not be '.' or '..'")
MSG_DEF(JSMSG_KV_STORE_PUT_CONTENT_STREAM,                     0, JSEXN_TYPEERR, "Content-provided streams are not yet supported for streaming into KVStore")
MSG_DEF(JSMSG_KV_STORE_PUT_OVER_30_MB,                         0, JSEXN_TYPEERR, "KVStore value can not be more than 30 Megabytes in size")
MSG_DEF(JSMSG_KV_STORE_LIST_ERROR,                             1, JSEXN_TYPEERR, "KVStore list: {0}")
MSG_DEF(JSMSG_KV_STORE_DELETE_ERROR,                           1, JSEXN_TYPEERR, "KVStore delete: {0}")
MSG_DEF(JSMSG_KV_STORE_LOOKUP_ERROR,                           1, JSEXN_TYPEERR, "KVStore lookup: {0}")
MSG_DEF(JSMSG_KV_STORE_INSERT_ERROR,                           1, JSEXN_TYPEERR, "KVStore insert: {0}")
MSG_DEF(JSMSG_SECRET_STORE_DOES_NOT_EXIST,                     1, JSEXN_TYPEERR, "SecretStore constructor: No SecretStore named '{0}' exists")
MSG_DEF(JSMSG_SECRET_STORE_KEY_EMPTY,                          0, JSEXN_TYPEERR, "SecretStore key can not be an empty string")
MSG_DEF(JSMSG_SECRET_STORE_KEY_TOO_LONG,                       0, JSEXN_TYPEERR, "SecretStore key can not be more than 255 characters")
MSG_DEF(JSMSG_SECRET_STORE_KEY_CONTAINS_INVALID_CHARACTER,     0, JSEXN_TYPEERR, "SecretStore key can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace")
MSG_DEF(JSMSG_SECRET_STORE_NAME_CONTAINS_INVALID_CHARACTER,    0, JSEXN_TYPEERR, "SecretStore constructor: name can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace")
MSG_DEF(JSMSG_SECRET_STORE_NAME_EMPTY,                         0, JSEXN_TYPEERR, "SecretStore constructor: name can not be an empty string")
MSG_DEF(JSMSG_SECRET_STORE_NAME_START_WITH_ASCII_ALPHA,        0, JSEXN_TYPEERR, "SecretStore constructor: name must start with an ascii alpabetical character")
MSG_DEF(JSMSG_SECRET_STORE_NAME_TOO_LONG,                      0, JSEXN_TYPEERR, "SecretStore constructor: name can not be more than 255 characters")
MSG_DEF(JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED,             0, JSEXN_TYPEERR, "Can't use a ReadableStream that's locked or has ever been read from or canceled")
MSG_DEF(JSMSG_DYNAMIC_BACKENDS_UNSUPPORTED_EXPLICIT,           2, JSEXN_ERR, "fetch(): No backend provided fetching '{1}'. Since dynamic backends are not enabled for this Fastly service, `fetch()` requires an explicit backend parameter. See https://js-compute-reference-docs.edgecompute.app/docs/globals/fetch for more info. Alternatively, contact Fastly support to enable dynamic backends.")
MSG_DEF(JSMSG_DYNAMIC_BACKENDS_UNSUPPORTED_IMPLICIT,           1, JSEXN_ERR, "Backend constructor: Unable to create a dynamic backend for '{0}' - dynamic backends are unsupported on this service. Either explicitly configure backend services or contact Fastly support to enable dynamic backends.")
MSG_DEF(JSMSG_BACKEND_FROMNAME_BACKEND_DOES_NOT_EXIST,         1, JSEXN_ERR, "Backend.fromName: backend named '{0}' does not exist")
MSG_DEF(JSMSG_BACKEND_IS_HEALTHY_BACKEND_DOES_NOT_EXIST,       1, JSEXN_ERR, "Backend.health: backend named '{0}' does not exist")
MSG_DEF(JSMSG_BACKEND_PARAMETER_NOT_OBJECT,                    0, JSEXN_TYPEERR, "Backend constructor: configuration parameter must be an Object")
MSG_DEF(JSMSG_BACKEND_NAME_NOT_SET,                            0, JSEXN_TYPEERR, "Backend constructor: name can not be null or undefined")
MSG_DEF(JSMSG_BACKEND_NAME_TOO_LONG,                           0, JSEXN_TYPEERR, "Backend constructor: name can not be more than 254 characters")
MSG_DEF(JSMSG_BACKEND_NAME_EMPTY,                              0, JSEXN_TYPEERR, "Backend constructor: name can not be an empty string")
MSG_DEF(JSMSG_BACKEND_TARGET_NOT_SET,                          0, JSEXN_TYPEERR, "Backend constructor: target can not be null or undefined")
MSG_DEF(JSMSG_BACKEND_TARGET_EMPTY,                            0, JSEXN_TYPEERR, "Backend constructor: target can not be an empty string")
MSG_DEF(JSMSG_BACKEND_TARGET_INVALID,                          0, JSEXN_TYPEERR, "Backend constructor: target does not contain a valid IPv4, IPv6, or hostname address")
MSG_DEF(JSMSG_BACKEND_CIPHERS_EMPTY,                           0, JSEXN_TYPEERR, "Backend constructor: ciphers can not be an empty string")
MSG_DEF(JSMSG_BACKEND_CIPHERS_NOT_AVALIABLE,                   0, JSEXN_TYPEERR, "Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration")
MSG_DEF(JSMSG_BACKEND_HOST_OVERRIDE_EMPTY,                     0, JSEXN_TYPEERR, "Backend constructor: hostOverride can not be an empty string")
MSG_DEF(JSMSG_BACKEND_CERTIFICATE_HOSTNAME_EMPTY,              0, JSEXN_TYPEERR, "Backend constructor: certificateHostname can not be an empty string")
MSG_DEF(JSMSG_BACKEND_SNI_HOSTNAME_EMPTY,                      0, JSEXN_TYPEERR, "Backend constructor: sniHostname can not be an empty string")
MSG_DEF(JSMSG_BACKEND_CA_CERTIFICATE_EMPTY,                    0, JSEXN_TYPEERR, "Backend constructor: caCertificate can not be an empty string")
MSG_DEF(JSMSG_BACKEND_TLS_MIN_INVALID,                         0, JSEXN_RANGEERR, "Backend constructor: tlsMinVersion must be either 1, 1.1, 1.2, or 1.3")
MSG_DEF(JSMSG_BACKEND_TLS_MAX_INVALID,                         0, JSEXN_RANGEERR, "Backend constructor: tlsMaxVersion must be either 1, 1.1, 1.2, or 1.3")
MSG_DEF(JSMSG_BACKEND_TLS_MIN_GREATER_THAN_TLS_MAX,            0, JSEXN_RANGEERR, "Backend constructor: tlsMinVersion must be less than or equal to tlsMaxVersion")
MSG_DEF(JSMSG_BACKEND_PORT_INVALID,                            0, JSEXN_RANGEERR, "Backend constructor: port must be more than 0 and less than 2^16 (65,536)")
MSG_DEF(JSMSG_BACKEND_CLIENT_CERTIFICATE_NOT_OBJECT,           0, JSEXN_TYPEERR, "Backend constructor: clientCertificate must be an object containing 'certificate' and 'key' properties")
MSG_DEF(JSMSG_BACKEND_CLIENT_CERTIFICATE_NO_CERTIFICATE,       0, JSEXN_TYPEERR, "Backend constructor: clientCertificate 'certificate' must be a certificate string")
MSG_DEF(JSMSG_BACKEND_CLIENT_CERTIFICATE_CERTIFICATE_EMPTY,    0, JSEXN_TYPEERR, "Backend constructor: clientCertificate 'certificate' can not be an empty string")
MSG_DEF(JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID,          0, JSEXN_TYPEERR, "Backend constructor: clientCertificate 'key' must be a SecretStoreEntry instance")
MSG_DEF(JSMSG_BACKEND_TCP_KEEPALIVE_NOT_OBJECT_OR_BOOL,        0, JSEXN_TYPEERR, "Backend constructor: tcpKeepalive must be a bool or object containing optional 'timeSecs', 'intervalSecs' and 'probes' properties")
MSG_DEF(JSMSG_BACKEND_TCP_KEEPALIVE_INVALID_PROBES,            0, JSEXN_TYPEERR, "Backend constructor: tcpKeepalive 'probes' must be an integer number greater than zero")
MSG_DEF(JSMSG_CACHE_OVERRIDE_MODE_INVALID,                     1, JSEXN_TYPEERR, "CacheOverride constructor: 'mode' has to be \"none\", \"pass\", or \"override\", but got \"{0}\"")
MSG_DEF(JSMSG_RESPONSE_VALUE_NOT_UINT8ARRAY,                   0, JSEXN_TYPEERR, "Can't convert value to Uint8Array while consuming Body")
MSG_DEF(JSMSG_RESPONSE_BODY_DISTURBED_OR_LOCKED,               0, JSEXN_TYPEERR, "Response body object should not be disturbed or locked")
MSG_DEF(JSMSG_RESPONSE_CONSTRUCTOR_INVALID_STATUS,             1, JSEXN_RANGEERR, "Response constructor: Invalid response status code. The status provided ({0}) is outside the range [200, 599].")
MSG_DEF(JSMSG_RESPONSE_CONSTRUCTOR_INVALID_STATUS_TEXT,        0, JSEXN_TYPEERR, "Response constructor: Invalid response status text. The statusText provided contains invalid characters.")
MSG_DEF(JSMSG_RESPONSE_CONSTRUCTOR_BODY_WITH_NULL_BODY_STATUS, 0, JSEXN_TYPEERR, "Response constructor: Response body is given with a null body status.")
MSG_DEF(JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST,                  1, JSEXN_TYPEERR, "Requested backend named '{0}' does not exist")
MSG_DEF(JSMSG_RESPONSE_REDIRECT_INVALID_URI,                   0, JSEXN_TYPEERR, "Response.redirect: url parameter is not a valid URL.")
MSG_DEF(JSMSG_RESPONSE_REDIRECT_INVALID_STATUS,                0, JSEXN_RANGEERR, "Response.redirect: Invalid redirect status code.")
MSG_DEF(JSMSG_RESPONSE_NULL_BODY_STATUS_WITH_BODY,             0, JSEXN_TYPEERR, "Response with null body status cannot have body")
MSG_DEF(JSMSG_RESPONSE_JSON_INVALID_VALUE,                     0, JSEXN_TYPEERR, "Redirect.json: The data is not JSON serializable")
MSG_DEF(JSMSG_TIMEOUT_NEGATIVE,                                2, JSEXN_RANGEERR, "{0}: {1} can not be a negative number")
MSG_DEF(JSMSG_TIMEOUT_TOO_BIG,                                 3, JSEXN_RANGEERR, "{0}: {1} is above the maximum of {2}")
MSG_DEF(JSMSG_TIMEOUT_NAN,                                     2, JSEXN_RANGEERR, "{0}: {1} is not a valid number")
MSG_DEF(JSMSG_INVALID_BUFFER,                                  1, JSEXN_TYPEERR, "{0}: bytes must be an ArrayBuffer or ArrayBufferView object")
MSG_DEF(JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM,                 0, JSEXN_TYPEERR, "Content-provided streams are not yet supported for streaming into SimpleCache")
MSG_DEF(JSMSG_BODY_APPEND_CONTENT_STREAM,                      0, JSEXN_TYPEERR, "Content-provided streams are not yet supported for appending onto a FastlyBody")
MSG_DEF(JSMSG_BODY_PREPEND_CONTENT_STREAM,                     0, JSEXN_TYPEERR, "Content-provided streams are not yet supported for prepending onto a FastlyBody")
//clang-format on