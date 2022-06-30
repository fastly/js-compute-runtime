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
MSG_DEF(JSMSG_BUILTIN_CTOR_NO_NEW,                             1, JSEXN_TYPEERR, "calling a builtin {0} constructor without new is forbidden")
MSG_DEF(JSMSG_INCOMPATIBLE_INSTANCE,                           2, JSEXN_TYPEERR, "Method {0} called on receiver that's not an instance of {1}")
MSG_DEF(JSMSG_INVALID_BUFFER_ARG,                              2, JSEXN_TYPEERR, "{0} must be of type ArrayBuffer or ArrayBufferView but got \"{1}\"")
MSG_DEF(JSMSG_INVALID_COMPRESSION_FORMAT,                      1, JSEXN_TYPEERR, "'format' has to be \"deflate\", \"deflate-raw\", or \"gzip\", but got \"{0}\"")
MSG_DEF(JSMSG_DECOMPRESSING_ERROR,                             0, JSEXN_TYPEERR, "DecompressionStream transform: error decompressing chunk")
MSG_DEF(JSMSG_OBJECT_STORE_NAME_EMPTY,                         0, JSEXN_TYPEERR, "ObjectStore constructor: name can not be an empty string")
MSG_DEF(JSMSG_OBJECT_STORE_NAME_TOO_LONG,                      0, JSEXN_TYPEERR, "ObjectStore constructor: name can not be more than 255 characters")
MSG_DEF(JSMSG_OBJECT_STORE_NAME_NO_CONTROL_CHARACTERS,         0, JSEXN_TYPEERR, "ObjectStore constructor: name can not contain control characters (\\u0000-\\u001F)")
MSG_DEF(JSMSG_OBJECT_STORE_DOES_NOT_EXIST,                     1, JSEXN_TYPEERR, "ObjectStore constructor: No ObjectStore named '{0}' exists")
MSG_DEF(JSMSG_OBJECT_STORE_KEY_EMPTY,                          0, JSEXN_TYPEERR, "ObjectStore key can not be an empty string")
MSG_DEF(JSMSG_OBJECT_STORE_KEY_TOO_LONG,                       0, JSEXN_TYPEERR, "ObjectStore key can not be more than 1024 characters")
MSG_DEF(JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER,              1, JSEXN_TYPEERR, "ObjectStore key can not contain {0} character")
MSG_DEF(JSMSG_OBJECT_STORE_KEY_ACME,                           0, JSEXN_TYPEERR, "ObjectStore key can not start with .well-known/acme-challenge/")
MSG_DEF(JSMSG_OBJECT_STORE_KEY_RELATIVE,                       0, JSEXN_TYPEERR, "ObjectStore key can not be '.' or '..'")
MSG_DEF(JSMSG_OBJECT_STORE_PUT_CONTENT_STREAM,                 0, JSEXN_TYPEERR, "Content-provided streams are not yet supported for streaming into ObjectStore")
MSG_DEF(JSMSG_OBJECT_STORE_PUT_OVER_30_MB,                     0, JSEXN_TYPEERR, "ObjectStore value can not be more than 30 Megabytes in size")
MSG_DEF(JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED,             0, JSEXN_TYPEERR, "Can't use a ReadableStream that's locked or has ever been read from or canceled")
//clang-format on