/* eslint-env serviceworker */
import { routes } from "./routes.js";
// React
var React = {};
{
  /**
   * @license React
   * react.production.min.js
   *
   * Copyright (c) Facebook, Inc. and its affiliates.
   *
   * This source code is licensed under the MIT license found in the
   * LICENSE file in the root directory of this source tree.
   */
  let exports = React;
  let l = Symbol.for("react.element"),
    n = Symbol.for("react.portal"),
    p = Symbol.for("react.fragment"),
    q = Symbol.for("react.strict_mode"),
    r = Symbol.for("react.profiler"),
    t = Symbol.for("react.provider"),
    u = Symbol.for("react.context"),
    v = Symbol.for("react.forward_ref"),
    w = Symbol.for("react.suspense"),
    x = Symbol.for("react.memo"),
    y = Symbol.for("react.lazy"),
    z = Symbol.iterator;
  function A(a) {
    if (null === a || "object" !== typeof a) return null;
    a = (z && a[z]) || a["@@iterator"];
    return "function" === typeof a ? a : null;
  }
  let B = {
      isMounted: function () {
        return !1;
      },
      enqueueForceUpdate: function () {},
      enqueueReplaceState: function () {},
      enqueueSetState: function () {},
    },
    C = Object.assign,
    D = {};
  function E(a, b, e) {
    this.props = a;
    this.context = b;
    this.refs = D;
    this.updater = e || B;
  }
  E.prototype.isReactComponent = {};
  E.prototype.setState = function (a, b) {
    if ("object" !== typeof a && "function" !== typeof a && null != a)
      throw Error(
        "setState(...): takes an object of state variables to update or a function which returns an object of state variables.",
      );
    this.updater.enqueueSetState(this, a, b, "setState");
  };
  E.prototype.forceUpdate = function (a) {
    this.updater.enqueueForceUpdate(this, a, "forceUpdate");
  };
  function F() {}
  F.prototype = E.prototype;
  function G(a, b, e) {
    this.props = a;
    this.context = b;
    this.refs = D;
    this.updater = e || B;
  }
  var H = (G.prototype = new F());
  H.constructor = G;
  C(H, E.prototype);
  H.isPureReactComponent = !0;
  var I = Array.isArray,
    J = Object.prototype.hasOwnProperty,
    K = { current: null },
    L = { key: !0, ref: !0, __self: !0, __source: !0 };
  function M(a, b, e) {
    var d,
      c = {},
      k = null,
      h = null;
    if (null != b)
      for (d in (void 0 !== b.ref && (h = b.ref),
      void 0 !== b.key && (k = "" + b.key),
      b))
        J.call(b, d) && !L.hasOwnProperty(d) && (c[d] = b[d]);
    var g = arguments.length - 2;
    if (1 === g) c.children = e;
    else if (1 < g) {
      for (var f = Array(g), m = 0; m < g; m++) f[m] = arguments[m + 2];
      c.children = f;
    }
    if (a && a.defaultProps)
      for (d in ((g = a.defaultProps), g)) void 0 === c[d] && (c[d] = g[d]);
    return {
      $$typeof: l,
      type: a,
      key: k,
      ref: h,
      props: c,
      _owner: K.current,
    };
  }
  function N(a, b) {
    return {
      $$typeof: l,
      type: a.type,
      key: b,
      ref: a.ref,
      props: a.props,
      _owner: a._owner,
    };
  }
  function O(a) {
    return "object" === typeof a && null !== a && a.$$typeof === l;
  }
  function escape(a) {
    var b = { "=": "=0", ":": "=2" };
    return (
      "$" +
      a.replace(/[=:]/g, function (a) {
        return b[a];
      })
    );
  }
  var P = /\/+/g;
  function Q(a, b) {
    return "object" === typeof a && null !== a && null != a.key
      ? escape("" + a.key)
      : b.toString(36);
  }
  function R(a, b, e, d, c) {
    var k = typeof a;
    if ("undefined" === k || "boolean" === k) a = null;
    var h = !1;
    if (null === a) h = !0;
    else
      switch (k) {
        case "string":
        case "number":
          h = !0;
          break;
        case "object":
          switch (a.$$typeof) {
            case l:
            case n:
              h = !0;
          }
      }
    if (h)
      return (
        (h = a),
        (c = c(h)),
        (a = "" === d ? "." + Q(h, 0) : d),
        I(c)
          ? ((e = ""),
            null != a && (e = a.replace(P, "$&/") + "/"),
            R(c, b, e, "", function (a) {
              return a;
            }))
          : null != c &&
            (O(c) &&
              (c = N(
                c,
                e +
                  (!c.key || (h && h.key === c.key)
                    ? ""
                    : ("" + c.key).replace(P, "$&/") + "/") +
                  a,
              )),
            b.push(c)),
        1
      );
    h = 0;
    d = "" === d ? "." : d + ":";
    if (I(a))
      for (var g = 0; g < a.length; g++) {
        k = a[g];
        var f = d + Q(k, g);
        h += R(k, b, e, f, c);
      }
    else if (((f = A(a)), "function" === typeof f))
      for (a = f.call(a), g = 0; !(k = a.next()).done; )
        (k = k.value), (f = d + Q(k, g++)), (h += R(k, b, e, f, c));
    else if ("object" === k)
      throw (
        ((b = String(a)),
        Error(
          "Objects are not valid as a React child (found: " +
            ("[object Object]" === b
              ? "object with keys {" + Object.keys(a).join(", ") + "}"
              : b) +
            "). If you meant to render a collection of children, use an array instead.",
        ))
      );
    return h;
  }
  function S(a, b, e) {
    if (null == a) return a;
    var d = [],
      c = 0;
    R(a, d, "", "", function (a) {
      return b.call(e, a, c++);
    });
    return d;
  }
  function T(a) {
    if (-1 === a._status) {
      var b = a._result;
      b = b();
      b.then(
        function (b) {
          if (0 === a._status || -1 === a._status)
            (a._status = 1), (a._result = b);
        },
        function (b) {
          if (0 === a._status || -1 === a._status)
            (a._status = 2), (a._result = b);
        },
      );
      -1 === a._status && ((a._status = 0), (a._result = b));
    }
    if (1 === a._status) return a._result.default;
    throw a._result;
  }
  var U = { current: null },
    V = { transition: null },
    W = {
      ReactCurrentDispatcher: U,
      ReactCurrentBatchConfig: V,
      ReactCurrentOwner: K,
    };
  exports.Children = {
    map: S,
    forEach: function (a, b, e) {
      S(
        a,
        function () {
          b.apply(this, arguments);
        },
        e,
      );
    },
    count: function (a) {
      var b = 0;
      S(a, function () {
        b++;
      });
      return b;
    },
    toArray: function (a) {
      return (
        S(a, function (a) {
          return a;
        }) || []
      );
    },
    only: function (a) {
      if (!O(a))
        throw Error(
          "React.Children.only expected to receive a single React element child.",
        );
      return a;
    },
  };
  exports.Component = E;
  exports.Fragment = p;
  exports.Profiler = r;
  exports.PureComponent = G;
  exports.StrictMode = q;
  exports.Suspense = w;
  exports.__SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED = W;
  exports.cloneElement = function (a, b, e) {
    if (null === a || void 0 === a)
      throw Error(
        "React.cloneElement(...): The argument must be a React element, but you passed " +
          a +
          ".",
      );
    var d = C({}, a.props),
      c = a.key,
      k = a.ref,
      h = a._owner;
    if (null != b) {
      void 0 !== b.ref && ((k = b.ref), (h = K.current));
      void 0 !== b.key && (c = "" + b.key);
      if (a.type && a.type.defaultProps) var g = a.type.defaultProps;
      for (f in b)
        J.call(b, f) &&
          !L.hasOwnProperty(f) &&
          (d[f] = void 0 === b[f] && void 0 !== g ? g[f] : b[f]);
    }
    var f = arguments.length - 2;
    if (1 === f) d.children = e;
    else if (1 < f) {
      g = Array(f);
      for (var m = 0; m < f; m++) g[m] = arguments[m + 2];
      d.children = g;
    }
    return { $$typeof: l, type: a.type, key: c, ref: k, props: d, _owner: h };
  };
  exports.createContext = function (a) {
    a = {
      $$typeof: u,
      _currentValue: a,
      _currentValue2: a,
      _threadCount: 0,
      Provider: null,
      Consumer: null,
      _defaultValue: null,
      _globalName: null,
    };
    a.Provider = { $$typeof: t, _context: a };
    return (a.Consumer = a);
  };
  exports.createElement = M;
  exports.createFactory = function (a) {
    var b = M.bind(null, a);
    b.type = a;
    return b;
  };
  exports.createRef = function () {
    return { current: null };
  };
  exports.forwardRef = function (a) {
    return { $$typeof: v, render: a };
  };
  exports.isValidElement = O;
  exports.lazy = function (a) {
    return { $$typeof: y, _payload: { _status: -1, _result: a }, _init: T };
  };
  exports.memo = function (a, b) {
    return { $$typeof: x, type: a, compare: void 0 === b ? null : b };
  };
  exports.startTransition = function (a) {
    var b = V.transition;
    V.transition = {};
    try {
      a();
    } finally {
      V.transition = b;
    }
  };
  exports.unstable_act = function () {
    throw Error("act(...) is not supported in production builds of React.");
  };
  exports.useCallback = function (a, b) {
    return U.current.useCallback(a, b);
  };
  exports.useContext = function (a) {
    return U.current.useContext(a);
  };
  exports.useDebugValue = function () {};
  exports.useDeferredValue = function (a) {
    return U.current.useDeferredValue(a);
  };
  exports.useEffect = function (a, b) {
    return U.current.useEffect(a, b);
  };
  exports.useId = function () {
    return U.current.useId();
  };
  exports.useImperativeHandle = function (a, b, e) {
    return U.current.useImperativeHandle(a, b, e);
  };
  exports.useInsertionEffect = function (a, b) {
    return U.current.useInsertionEffect(a, b);
  };
  exports.useLayoutEffect = function (a, b) {
    return U.current.useLayoutEffect(a, b);
  };
  exports.useMemo = function (a, b) {
    return U.current.useMemo(a, b);
  };
  exports.useReducer = function (a, b, e) {
    return U.current.useReducer(a, b, e);
  };
  exports.useRef = function (a) {
    return U.current.useRef(a);
  };
  exports.useState = function (a) {
    return U.current.useState(a);
  };
  exports.useSyncExternalStore = function (a, b, e) {
    return U.current.useSyncExternalStore(a, b, e);
  };
  exports.useTransition = function () {
    return U.current.useTransition();
  };
  exports.version = "18.2.0";
}

// react/jsx-runtime
let jsx, jsxs;
{
  /**
   * @license React
   * react-jsx-runtime.production.min.js
   *
   * Copyright (c) Facebook, Inc. and its affiliates.
   *
   * This source code is licensed under the MIT license found in the
   * LICENSE file in the root directory of this source tree.
   */
  let k = Symbol.for("react.element"),
    m = Object.prototype.hasOwnProperty,
    n =
      React.__SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED
        .ReactCurrentOwner,
    p = { key: !0, ref: !0, __self: !0, __source: !0 };
  function q(c, a, g) {
    var b,
      d = {},
      e = null,
      h = null;
    void 0 !== g && (e = "" + g);
    void 0 !== a.key && (e = "" + a.key);
    void 0 !== a.ref && (h = a.ref);
    for (b in a) m.call(a, b) && !p.hasOwnProperty(b) && (d[b] = a[b]);
    if (c && c.defaultProps)
      for (b in ((a = c.defaultProps), a)) void 0 === d[b] && (d[b] = a[b]);
    return {
      $$typeof: k,
      type: c,
      key: e,
      ref: h,
      props: d,
      _owner: n.current,
    };
  }
  jsx = jsxs = q;
}

// react-dom/server
let renderToReadableStream;
{
  /**
   * @license React
   * react-dom-server.browser.production.min.js
   *
   * Copyright (c) Facebook, Inc. and its affiliates.
   *
   * This source code is licensed under the MIT license found in the
   * LICENSE file in the root directory of this source tree.
   */
  var aa = React;
  function k(a) {
    for (
      var b = "https://reactjs.org/docs/error-decoder.html?invariant=" + a,
        c = 1;
      c < arguments.length;
      c++
    )
      b += "&args[]=" + encodeURIComponent(arguments[c]);
    return (
      "Minified React error #" +
      a +
      "; visit " +
      b +
      " for the full message or use the non-minified dev environment for full errors and additional helpful warnings."
    );
  }
  var l = null,
    n = 0;
  function p(a, b) {
    if (0 !== b.length)
      if (512 < b.length)
        0 < n &&
          (a.enqueue(new Uint8Array(l.buffer, 0, n)),
          (l = new Uint8Array(512)),
          (n = 0)),
          a.enqueue(b);
      else {
        var c = l.length - n;
        c < b.length &&
          (0 === c
            ? a.enqueue(l)
            : (l.set(b.subarray(0, c), n), a.enqueue(l), (b = b.subarray(c))),
          (l = new Uint8Array(512)),
          (n = 0));
        l.set(b, n);
        n += b.length;
      }
  }
  function t(a, b) {
    p(a, b);
    return !0;
  }
  function ba(a) {
    l &&
      0 < n &&
      (a.enqueue(new Uint8Array(l.buffer, 0, n)), (l = null), (n = 0));
  }
  var ca = new TextEncoder();
  function u(a) {
    return ca.encode(a);
  }
  function w(a) {
    return ca.encode(a);
  }
  function da(a, b) {
    "function" === typeof a.error ? a.error(b) : a.close();
  }
  var x = Object.prototype.hasOwnProperty,
    ea =
      /^[:A-Z_a-z\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF\u0370-\u037D\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD][:A-Z_a-z\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF\u0370-\u037D\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD\-.0-9\u00B7\u0300-\u036F\u203F-\u2040]*$/,
    fa = {},
    ha = {};
  function ia(a) {
    if (x.call(ha, a)) return !0;
    if (x.call(fa, a)) return !1;
    if (ea.test(a)) return (ha[a] = !0);
    fa[a] = !0;
    return !1;
  }
  function y(a, b, c, d, f, e, g) {
    this.acceptsBooleans = 2 === b || 3 === b || 4 === b;
    this.attributeName = d;
    this.attributeNamespace = f;
    this.mustUseProperty = c;
    this.propertyName = a;
    this.type = b;
    this.sanitizeURL = e;
    this.removeEmptyString = g;
  }
  var z = {};
  "children dangerouslySetInnerHTML defaultValue defaultChecked innerHTML suppressContentEditableWarning suppressHydrationWarning style"
    .split(" ")
    .forEach(function (a) {
      z[a] = new y(a, 0, !1, a, null, !1, !1);
    });
  [
    ["acceptCharset", "accept-charset"],
    ["className", "class"],
    ["htmlFor", "for"],
    ["httpEquiv", "http-equiv"],
  ].forEach(function (a) {
    var b = a[0];
    z[b] = new y(b, 1, !1, a[1], null, !1, !1);
  });
  ["contentEditable", "draggable", "spellCheck", "value"].forEach(function (a) {
    z[a] = new y(a, 2, !1, a.toLowerCase(), null, !1, !1);
  });
  [
    "autoReverse",
    "externalResourcesRequired",
    "focusable",
    "preserveAlpha",
  ].forEach(function (a) {
    z[a] = new y(a, 2, !1, a, null, !1, !1);
  });
  "allowFullScreen async autoFocus autoPlay controls default defer disabled disablePictureInPicture disableRemotePlayback formNoValidate hidden loop noModule noValidate open playsInline readOnly required reversed scoped seamless itemScope"
    .split(" ")
    .forEach(function (a) {
      z[a] = new y(a, 3, !1, a.toLowerCase(), null, !1, !1);
    });
  ["checked", "multiple", "muted", "selected"].forEach(function (a) {
    z[a] = new y(a, 3, !0, a, null, !1, !1);
  });
  ["capture", "download"].forEach(function (a) {
    z[a] = new y(a, 4, !1, a, null, !1, !1);
  });
  ["cols", "rows", "size", "span"].forEach(function (a) {
    z[a] = new y(a, 6, !1, a, null, !1, !1);
  });
  ["rowSpan", "start"].forEach(function (a) {
    z[a] = new y(a, 5, !1, a.toLowerCase(), null, !1, !1);
  });
  var ja = /[\-:]([a-z])/g;
  function ka(a) {
    return a[1].toUpperCase();
  }
  "accent-height alignment-baseline arabic-form baseline-shift cap-height clip-path clip-rule color-interpolation color-interpolation-filters color-profile color-rendering dominant-baseline enable-background fill-opacity fill-rule flood-color flood-opacity font-family font-size font-size-adjust font-stretch font-style font-variant font-weight glyph-name glyph-orientation-horizontal glyph-orientation-vertical horiz-adv-x horiz-origin-x image-rendering letter-spacing lighting-color marker-end marker-mid marker-start overline-position overline-thickness paint-order panose-1 pointer-events rendering-intent shape-rendering stop-color stop-opacity strikethrough-position strikethrough-thickness stroke-dasharray stroke-dashoffset stroke-linecap stroke-linejoin stroke-miterlimit stroke-opacity stroke-width text-anchor text-decoration text-rendering underline-position underline-thickness unicode-bidi unicode-range units-per-em v-alphabetic v-hanging v-ideographic v-mathematical vector-effect vert-adv-y vert-origin-x vert-origin-y word-spacing writing-mode xmlns:xlink x-height"
    .split(" ")
    .forEach(function (a) {
      var b = a.replace(ja, ka);
      z[b] = new y(b, 1, !1, a, null, !1, !1);
    });
  "xlink:actuate xlink:arcrole xlink:role xlink:show xlink:title xlink:type"
    .split(" ")
    .forEach(function (a) {
      var b = a.replace(ja, ka);
      z[b] = new y(b, 1, !1, a, "http://www.w3.org/1999/xlink", !1, !1);
    });
  ["xml:base", "xml:lang", "xml:space"].forEach(function (a) {
    var b = a.replace(ja, ka);
    z[b] = new y(b, 1, !1, a, "http://www.w3.org/XML/1998/namespace", !1, !1);
  });
  ["tabIndex", "crossOrigin"].forEach(function (a) {
    z[a] = new y(a, 1, !1, a.toLowerCase(), null, !1, !1);
  });
  z.xlinkHref = new y(
    "xlinkHref",
    1,
    !1,
    "xlink:href",
    "http://www.w3.org/1999/xlink",
    !0,
    !1,
  );
  ["src", "href", "action", "formAction"].forEach(function (a) {
    z[a] = new y(a, 1, !1, a.toLowerCase(), null, !0, !0);
  });
  var B = {
      animationIterationCount: !0,
      aspectRatio: !0,
      borderImageOutset: !0,
      borderImageSlice: !0,
      borderImageWidth: !0,
      boxFlex: !0,
      boxFlexGroup: !0,
      boxOrdinalGroup: !0,
      columnCount: !0,
      columns: !0,
      flex: !0,
      flexGrow: !0,
      flexPositive: !0,
      flexShrink: !0,
      flexNegative: !0,
      flexOrder: !0,
      gridArea: !0,
      gridRow: !0,
      gridRowEnd: !0,
      gridRowSpan: !0,
      gridRowStart: !0,
      gridColumn: !0,
      gridColumnEnd: !0,
      gridColumnSpan: !0,
      gridColumnStart: !0,
      fontWeight: !0,
      lineClamp: !0,
      lineHeight: !0,
      opacity: !0,
      order: !0,
      orphans: !0,
      tabSize: !0,
      widows: !0,
      zIndex: !0,
      zoom: !0,
      fillOpacity: !0,
      floodOpacity: !0,
      stopOpacity: !0,
      strokeDasharray: !0,
      strokeDashoffset: !0,
      strokeMiterlimit: !0,
      strokeOpacity: !0,
      strokeWidth: !0,
    },
    la = ["Webkit", "ms", "Moz", "O"];
  Object.keys(B).forEach(function (a) {
    la.forEach(function (b) {
      b = b + a.charAt(0).toUpperCase() + a.substring(1);
      B[b] = B[a];
    });
  });
  var oa = /["'&<>]/;
  function C(a) {
    if ("boolean" === typeof a || "number" === typeof a) return "" + a;
    a = "" + a;
    var b = oa.exec(a);
    if (b) {
      var c = "",
        d,
        f = 0;
      for (d = b.index; d < a.length; d++) {
        switch (a.charCodeAt(d)) {
          case 34:
            b = "&quot;";
            break;
          case 38:
            b = "&amp;";
            break;
          case 39:
            b = "&#x27;";
            break;
          case 60:
            b = "&lt;";
            break;
          case 62:
            b = "&gt;";
            break;
          default:
            continue;
        }
        f !== d && (c += a.substring(f, d));
        f = d + 1;
        c += b;
      }
      a = f !== d ? c + a.substring(f, d) : c;
    }
    return a;
  }
  var pa = /([A-Z])/g,
    qa = /^ms-/,
    ra = Array.isArray,
    sa = w("<script>"),
    ta = w("\x3c/script>"),
    ua = w('<script src="'),
    va = w('<script type="module" src="'),
    wa = w('" async="">\x3c/script>'),
    xa = /(<\/|<)(s)(cript)/gi;
  function ya(a, b, c, d) {
    return "" + b + ("s" === c ? "\\u0073" : "\\u0053") + d;
  }
  function za(a, b, c, d, f) {
    a = void 0 === a ? "" : a;
    b = void 0 === b ? sa : w('<script nonce="' + C(b) + '">');
    var e = [];
    void 0 !== c && e.push(b, u(("" + c).replace(xa, ya)), ta);
    if (void 0 !== d) for (c = 0; c < d.length; c++) e.push(ua, u(C(d[c])), wa);
    if (void 0 !== f) for (d = 0; d < f.length; d++) e.push(va, u(C(f[d])), wa);
    return {
      bootstrapChunks: e,
      startInlineScript: b,
      placeholderPrefix: w(a + "P:"),
      segmentPrefix: w(a + "S:"),
      boundaryPrefix: a + "B:",
      idPrefix: a,
      nextSuspenseID: 0,
      sentCompleteSegmentFunction: !1,
      sentCompleteBoundaryFunction: !1,
      sentClientRenderFunction: !1,
    };
  }
  function D(a, b) {
    return { insertionMode: a, selectedValue: b };
  }
  function Aa(a) {
    return D(
      "http://www.w3.org/2000/svg" === a
        ? 2
        : "http://www.w3.org/1998/Math/MathML" === a
          ? 3
          : 0,
      null,
    );
  }
  function Ba(a, b, c) {
    switch (b) {
      case "select":
        return D(1, null != c.value ? c.value : c.defaultValue);
      case "svg":
        return D(2, null);
      case "math":
        return D(3, null);
      case "foreignObject":
        return D(1, null);
      case "table":
        return D(4, null);
      case "thead":
      case "tbody":
      case "tfoot":
        return D(5, null);
      case "colgroup":
        return D(7, null);
      case "tr":
        return D(6, null);
    }
    return 4 <= a.insertionMode || 0 === a.insertionMode ? D(1, null) : a;
  }
  var Ca = w("\x3c!-- --\x3e");
  function Da(a, b, c, d) {
    if ("" === b) return d;
    d && a.push(Ca);
    a.push(u(C(b)));
    return !0;
  }
  var Ea = new Map(),
    Fa = w(' style="'),
    Ga = w(":"),
    Ha = w(";");
  function Ia(a, b, c) {
    if ("object" !== typeof c) throw Error(k(62));
    b = !0;
    for (var d in c)
      if (x.call(c, d)) {
        var f = c[d];
        if (null != f && "boolean" !== typeof f && "" !== f) {
          if (0 === d.indexOf("--")) {
            var e = u(C(d));
            f = u(C(("" + f).trim()));
          } else {
            e = d;
            var g = Ea.get(e);
            void 0 !== g
              ? (e = g)
              : ((g = w(
                  C(e.replace(pa, "-$1").toLowerCase().replace(qa, "-ms-")),
                )),
                Ea.set(e, g),
                (e = g));
            f =
              "number" === typeof f
                ? 0 === f || x.call(B, d)
                  ? u("" + f)
                  : u(f + "px")
                : u(C(("" + f).trim()));
          }
          b ? ((b = !1), a.push(Fa, e, Ga, f)) : a.push(Ha, e, Ga, f);
        }
      }
    b || a.push(E);
  }
  var H = w(" "),
    I = w('="'),
    E = w('"'),
    Ja = w('=""');
  function J(a, b, c, d) {
    switch (c) {
      case "style":
        Ia(a, b, d);
        return;
      case "defaultValue":
      case "defaultChecked":
      case "innerHTML":
      case "suppressContentEditableWarning":
      case "suppressHydrationWarning":
        return;
    }
    if (
      !(2 < c.length) ||
      ("o" !== c[0] && "O" !== c[0]) ||
      ("n" !== c[1] && "N" !== c[1])
    )
      if (((b = z.hasOwnProperty(c) ? z[c] : null), null !== b)) {
        switch (typeof d) {
          case "function":
          case "symbol":
            return;
          case "boolean":
            if (!b.acceptsBooleans) return;
        }
        c = u(b.attributeName);
        switch (b.type) {
          case 3:
            d && a.push(H, c, Ja);
            break;
          case 4:
            !0 === d
              ? a.push(H, c, Ja)
              : !1 !== d && a.push(H, c, I, u(C(d)), E);
            break;
          case 5:
            isNaN(d) || a.push(H, c, I, u(C(d)), E);
            break;
          case 6:
            !isNaN(d) && 1 <= d && a.push(H, c, I, u(C(d)), E);
            break;
          default:
            b.sanitizeURL && (d = "" + d), a.push(H, c, I, u(C(d)), E);
        }
      } else if (ia(c)) {
        switch (typeof d) {
          case "function":
          case "symbol":
            return;
          case "boolean":
            if (
              ((b = c.toLowerCase().slice(0, 5)),
              "data-" !== b && "aria-" !== b)
            )
              return;
        }
        a.push(H, u(c), I, u(C(d)), E);
      }
  }
  var K = w(">"),
    Ka = w("/>");
  function L(a, b, c) {
    if (null != b) {
      if (null != c) throw Error(k(60));
      if ("object" !== typeof b || !("__html" in b)) throw Error(k(61));
      b = b.__html;
      null !== b && void 0 !== b && a.push(u("" + b));
    }
  }
  function La(a) {
    var b = "";
    aa.Children.forEach(a, function (a) {
      null != a && (b += a);
    });
    return b;
  }
  var Ma = w(' selected=""');
  function Na(a, b, c, d) {
    a.push(M(c));
    var f = (c = null),
      e;
    for (e in b)
      if (x.call(b, e)) {
        var g = b[e];
        if (null != g)
          switch (e) {
            case "children":
              c = g;
              break;
            case "dangerouslySetInnerHTML":
              f = g;
              break;
            default:
              J(a, d, e, g);
          }
      }
    a.push(K);
    L(a, f, c);
    return "string" === typeof c ? (a.push(u(C(c))), null) : c;
  }
  var Oa = w("\n"),
    Pa = /^[a-zA-Z][a-zA-Z:_\.\-\d]*$/,
    Qa = new Map();
  function M(a) {
    var b = Qa.get(a);
    if (void 0 === b) {
      if (!Pa.test(a)) throw Error(k(65, a));
      b = w("<" + a);
      Qa.set(a, b);
    }
    return b;
  }
  var Ra = w("<!DOCTYPE html>");
  function Sa(a, b, c, d, f) {
    switch (b) {
      case "select":
        a.push(M("select"));
        var e = null,
          g = null;
        for (r in c)
          if (x.call(c, r)) {
            var h = c[r];
            if (null != h)
              switch (r) {
                case "children":
                  e = h;
                  break;
                case "dangerouslySetInnerHTML":
                  g = h;
                  break;
                case "defaultValue":
                case "value":
                  break;
                default:
                  J(a, d, r, h);
              }
          }
        a.push(K);
        L(a, g, e);
        return e;
      case "option":
        g = f.selectedValue;
        a.push(M("option"));
        var m = (h = null),
          q = null;
        var r = null;
        for (e in c)
          if (x.call(c, e)) {
            var v = c[e];
            if (null != v)
              switch (e) {
                case "children":
                  h = v;
                  break;
                case "selected":
                  q = v;
                  break;
                case "dangerouslySetInnerHTML":
                  r = v;
                  break;
                case "value":
                  m = v;
                default:
                  J(a, d, e, v);
              }
          }
        if (null != g)
          if (((c = null !== m ? "" + m : La(h)), ra(g)))
            for (d = 0; d < g.length; d++) {
              if ("" + g[d] === c) {
                a.push(Ma);
                break;
              }
            }
          else "" + g === c && a.push(Ma);
        else q && a.push(Ma);
        a.push(K);
        L(a, r, h);
        return h;
      case "textarea":
        a.push(M("textarea"));
        r = g = e = null;
        for (h in c)
          if (x.call(c, h) && ((m = c[h]), null != m))
            switch (h) {
              case "children":
                r = m;
                break;
              case "value":
                e = m;
                break;
              case "defaultValue":
                g = m;
                break;
              case "dangerouslySetInnerHTML":
                throw Error(k(91));
              default:
                J(a, d, h, m);
            }
        null === e && null !== g && (e = g);
        a.push(K);
        if (null != r) {
          if (null != e) throw Error(k(92));
          if (ra(r) && 1 < r.length) throw Error(k(93));
          e = "" + r;
        }
        "string" === typeof e && "\n" === e[0] && a.push(Oa);
        null !== e && a.push(u(C("" + e)));
        return null;
      case "input":
        a.push(M("input"));
        m = r = h = e = null;
        for (g in c)
          if (x.call(c, g) && ((q = c[g]), null != q))
            switch (g) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(k(399, "input"));
              case "defaultChecked":
                m = q;
                break;
              case "defaultValue":
                h = q;
                break;
              case "checked":
                r = q;
                break;
              case "value":
                e = q;
                break;
              default:
                J(a, d, g, q);
            }
        null !== r
          ? J(a, d, "checked", r)
          : null !== m && J(a, d, "checked", m);
        null !== e ? J(a, d, "value", e) : null !== h && J(a, d, "value", h);
        a.push(Ka);
        return null;
      case "menuitem":
        a.push(M("menuitem"));
        for (var A in c)
          if (x.call(c, A) && ((e = c[A]), null != e))
            switch (A) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(k(400));
              default:
                J(a, d, A, e);
            }
        a.push(K);
        return null;
      case "title":
        a.push(M("title"));
        e = null;
        for (v in c)
          if (x.call(c, v) && ((g = c[v]), null != g))
            switch (v) {
              case "children":
                e = g;
                break;
              case "dangerouslySetInnerHTML":
                throw Error(k(434));
              default:
                J(a, d, v, g);
            }
        a.push(K);
        return e;
      case "listing":
      case "pre":
        a.push(M(b));
        g = e = null;
        for (m in c)
          if (x.call(c, m) && ((h = c[m]), null != h))
            switch (m) {
              case "children":
                e = h;
                break;
              case "dangerouslySetInnerHTML":
                g = h;
                break;
              default:
                J(a, d, m, h);
            }
        a.push(K);
        if (null != g) {
          if (null != e) throw Error(k(60));
          if ("object" !== typeof g || !("__html" in g)) throw Error(k(61));
          c = g.__html;
          null !== c &&
            void 0 !== c &&
            ("string" === typeof c && 0 < c.length && "\n" === c[0]
              ? a.push(Oa, u(c))
              : a.push(u("" + c)));
        }
        "string" === typeof e && "\n" === e[0] && a.push(Oa);
        return e;
      case "area":
      case "base":
      case "br":
      case "col":
      case "embed":
      case "hr":
      case "img":
      case "keygen":
      case "link":
      case "meta":
      case "param":
      case "source":
      case "track":
      case "wbr":
        a.push(M(b));
        for (var F in c)
          if (x.call(c, F) && ((e = c[F]), null != e))
            switch (F) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(k(399, b));
              default:
                J(a, d, F, e);
            }
        a.push(Ka);
        return null;
      case "annotation-xml":
      case "color-profile":
      case "font-face":
      case "font-face-src":
      case "font-face-uri":
      case "font-face-format":
      case "font-face-name":
      case "missing-glyph":
        return Na(a, c, b, d);
      case "html":
        return 0 === f.insertionMode && a.push(Ra), Na(a, c, b, d);
      default:
        if (-1 === b.indexOf("-") && "string" !== typeof c.is)
          return Na(a, c, b, d);
        a.push(M(b));
        g = e = null;
        for (q in c)
          if (x.call(c, q) && ((h = c[q]), null != h))
            switch (q) {
              case "children":
                e = h;
                break;
              case "dangerouslySetInnerHTML":
                g = h;
                break;
              case "style":
                Ia(a, d, h);
                break;
              case "suppressContentEditableWarning":
              case "suppressHydrationWarning":
                break;
              default:
                ia(q) &&
                  "function" !== typeof h &&
                  "symbol" !== typeof h &&
                  a.push(H, u(q), I, u(C(h)), E);
            }
        a.push(K);
        L(a, g, e);
        return e;
    }
  }
  var Ta = w("</"),
    Ua = w(">"),
    Va = w('<template id="'),
    Wa = w('"></template>'),
    Xa = w("\x3c!--$--\x3e"),
    Ya = w('\x3c!--$?--\x3e<template id="'),
    Za = w('"></template>'),
    $a = w("\x3c!--$!--\x3e"),
    ab = w("\x3c!--/$--\x3e"),
    bb = w("<template"),
    cb = w('"'),
    db = w(' data-dgst="');
  w(' data-msg="');
  w(' data-stck="');
  var eb = w("></template>");
  function fb(a, b, c) {
    p(a, Ya);
    if (null === c) throw Error(k(395));
    p(a, c);
    return t(a, Za);
  }
  var gb = w('<div hidden id="'),
    hb = w('">'),
    ib = w("</div>"),
    jb = w('<svg aria-hidden="true" style="display:none" id="'),
    kb = w('">'),
    lb = w("</svg>"),
    mb = w('<math aria-hidden="true" style="display:none" id="'),
    nb = w('">'),
    ob = w("</math>"),
    pb = w('<table hidden id="'),
    qb = w('">'),
    rb = w("</table>"),
    sb = w('<table hidden><tbody id="'),
    tb = w('">'),
    ub = w("</tbody></table>"),
    vb = w('<table hidden><tr id="'),
    wb = w('">'),
    xb = w("</tr></table>"),
    yb = w('<table hidden><colgroup id="'),
    zb = w('">'),
    Ab = w("</colgroup></table>");
  function Bb(a, b, c, d) {
    switch (c.insertionMode) {
      case 0:
      case 1:
        return (
          p(a, gb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, hb)
        );
      case 2:
        return (
          p(a, jb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, kb)
        );
      case 3:
        return (
          p(a, mb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, nb)
        );
      case 4:
        return (
          p(a, pb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, qb)
        );
      case 5:
        return (
          p(a, sb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, tb)
        );
      case 6:
        return (
          p(a, vb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, wb)
        );
      case 7:
        return (
          p(a, yb), p(a, b.segmentPrefix), p(a, u(d.toString(16))), t(a, zb)
        );
      default:
        throw Error(k(397));
    }
  }
  function Cb(a, b) {
    switch (b.insertionMode) {
      case 0:
      case 1:
        return t(a, ib);
      case 2:
        return t(a, lb);
      case 3:
        return t(a, ob);
      case 4:
        return t(a, rb);
      case 5:
        return t(a, ub);
      case 6:
        return t(a, xb);
      case 7:
        return t(a, Ab);
      default:
        throw Error(k(397));
    }
  }
  var Db = w(
      'function $RS(a,b){a=document.getElementById(a);b=document.getElementById(b);for(a.parentNode.removeChild(a);a.firstChild;)b.parentNode.insertBefore(a.firstChild,b);b.parentNode.removeChild(b)};$RS("',
    ),
    Eb = w('$RS("'),
    Gb = w('","'),
    Hb = w('")\x3c/script>'),
    Ib = w(
      'function $RC(a,b){a=document.getElementById(a);b=document.getElementById(b);b.parentNode.removeChild(b);if(a){a=a.previousSibling;var f=a.parentNode,c=a.nextSibling,e=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d)if(0===e)break;else e--;else"$"!==d&&"$?"!==d&&"$!"!==d||e++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;b.firstChild;)f.insertBefore(b.firstChild,c);a.data="$";a._reactRetry&&a._reactRetry()}};$RC("',
    ),
    Jb = w('$RC("'),
    Kb = w('","'),
    Lb = w('")\x3c/script>'),
    Mb = w(
      'function $RX(b,c,d,e){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),b._reactRetry&&b._reactRetry())};$RX("',
    ),
    Nb = w('$RX("'),
    Ob = w('"'),
    Pb = w(")\x3c/script>"),
    Qb = w(","),
    Rb = /[<\u2028\u2029]/g;
  function Sb(a) {
    return JSON.stringify(a).replace(Rb, function (a) {
      switch (a) {
        case "<":
          return "\\u003c";
        case "\u2028":
          return "\\u2028";
        case "\u2029":
          return "\\u2029";
        default:
          throw Error(
            "escapeJSStringsForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React",
          );
      }
    });
  }
  var N = Object.assign,
    Tb = Symbol.for("react.element"),
    Ub = Symbol.for("react.portal"),
    Vb = Symbol.for("react.fragment"),
    Wb = Symbol.for("react.strict_mode"),
    Xb = Symbol.for("react.profiler"),
    Yb = Symbol.for("react.provider"),
    Zb = Symbol.for("react.context"),
    $b = Symbol.for("react.forward_ref"),
    ac = Symbol.for("react.suspense"),
    bc = Symbol.for("react.suspense_list"),
    cc = Symbol.for("react.memo"),
    dc = Symbol.for("react.lazy"),
    ec = Symbol.for("react.scope"),
    fc = Symbol.for("react.debug_trace_mode"),
    gc = Symbol.for("react.legacy_hidden"),
    hc = Symbol.for("react.default_value"),
    ic = Symbol.iterator;
  function jc(a) {
    if (null == a) return null;
    if ("function" === typeof a) return a.displayName || a.name || null;
    if ("string" === typeof a) return a;
    switch (a) {
      case Vb:
        return "Fragment";
      case Ub:
        return "Portal";
      case Xb:
        return "Profiler";
      case Wb:
        return "StrictMode";
      case ac:
        return "Suspense";
      case bc:
        return "SuspenseList";
    }
    if ("object" === typeof a)
      switch (a.$$typeof) {
        case Zb:
          return (a.displayName || "Context") + ".Consumer";
        case Yb:
          return (a._context.displayName || "Context") + ".Provider";
        case $b:
          var b = a.render;
          a = a.displayName;
          a ||
            ((a = b.displayName || b.name || ""),
            (a = "" !== a ? "ForwardRef(" + a + ")" : "ForwardRef"));
          return a;
        case cc:
          return (
            (b = a.displayName || null), null !== b ? b : jc(a.type) || "Memo"
          );
        case dc:
          b = a._payload;
          a = a._init;
          try {
            return jc(a(b));
          } catch (c) {}
      }
    return null;
  }
  var kc = {};
  function lc(a, b) {
    a = a.contextTypes;
    if (!a) return kc;
    var c = {},
      d;
    for (d in a) c[d] = b[d];
    return c;
  }
  var O = null;
  function P(a, b) {
    if (a !== b) {
      a.context._currentValue = a.parentValue;
      a = a.parent;
      var c = b.parent;
      if (null === a) {
        if (null !== c) throw Error(k(401));
      } else {
        if (null === c) throw Error(k(401));
        P(a, c);
      }
      b.context._currentValue = b.value;
    }
  }
  function mc(a) {
    a.context._currentValue = a.parentValue;
    a = a.parent;
    null !== a && mc(a);
  }
  function nc(a) {
    var b = a.parent;
    null !== b && nc(b);
    a.context._currentValue = a.value;
  }
  function oc(a, b) {
    a.context._currentValue = a.parentValue;
    a = a.parent;
    if (null === a) throw Error(k(402));
    a.depth === b.depth ? P(a, b) : oc(a, b);
  }
  function pc(a, b) {
    var c = b.parent;
    if (null === c) throw Error(k(402));
    a.depth === c.depth ? P(a, c) : pc(a, c);
    b.context._currentValue = b.value;
  }
  function Q(a) {
    var b = O;
    b !== a &&
      (null === b
        ? nc(a)
        : null === a
          ? mc(b)
          : b.depth === a.depth
            ? P(b, a)
            : b.depth > a.depth
              ? oc(b, a)
              : pc(b, a),
      (O = a));
  }
  var qc = {
    isMounted: function () {
      return !1;
    },
    enqueueSetState: function (a, b) {
      a = a._reactInternals;
      null !== a.queue && a.queue.push(b);
    },
    enqueueReplaceState: function (a, b) {
      a = a._reactInternals;
      a.replace = !0;
      a.queue = [b];
    },
    enqueueForceUpdate: function () {},
  };
  function rc(a, b, c, d) {
    var f = void 0 !== a.state ? a.state : null;
    a.updater = qc;
    a.props = c;
    a.state = f;
    var e = { queue: [], replace: !1 };
    a._reactInternals = e;
    var g = b.contextType;
    a.context = "object" === typeof g && null !== g ? g._currentValue : d;
    g = b.getDerivedStateFromProps;
    "function" === typeof g &&
      ((g = g(c, f)),
      (f = null === g || void 0 === g ? f : N({}, f, g)),
      (a.state = f));
    if (
      "function" !== typeof b.getDerivedStateFromProps &&
      "function" !== typeof a.getSnapshotBeforeUpdate &&
      ("function" === typeof a.UNSAFE_componentWillMount ||
        "function" === typeof a.componentWillMount)
    )
      if (
        ((b = a.state),
        "function" === typeof a.componentWillMount && a.componentWillMount(),
        "function" === typeof a.UNSAFE_componentWillMount &&
          a.UNSAFE_componentWillMount(),
        b !== a.state && qc.enqueueReplaceState(a, a.state, null),
        null !== e.queue && 0 < e.queue.length)
      )
        if (
          ((b = e.queue),
          (g = e.replace),
          (e.queue = null),
          (e.replace = !1),
          g && 1 === b.length)
        )
          a.state = b[0];
        else {
          e = g ? b[0] : a.state;
          f = !0;
          for (g = g ? 1 : 0; g < b.length; g++) {
            var h = b[g];
            h = "function" === typeof h ? h.call(a, e, c, d) : h;
            null != h && (f ? ((f = !1), (e = N({}, e, h))) : N(e, h));
          }
          a.state = e;
        }
      else e.queue = null;
  }
  var sc = { id: 1, overflow: "" };
  function tc(a, b, c) {
    var d = a.id;
    a = a.overflow;
    var f = 32 - uc(d) - 1;
    d &= ~(1 << f);
    c += 1;
    var e = 32 - uc(b) + f;
    if (30 < e) {
      var g = f - (f % 5);
      e = (d & ((1 << g) - 1)).toString(32);
      d >>= g;
      f -= g;
      return { id: (1 << (32 - uc(b) + f)) | (c << f) | d, overflow: e + a };
    }
    return { id: (1 << e) | (c << f) | d, overflow: a };
  }
  var uc = Math.clz32 ? Math.clz32 : vc,
    wc = Math.log,
    xc = Math.LN2;
  function vc(a) {
    a >>>= 0;
    return 0 === a ? 32 : (31 - ((wc(a) / xc) | 0)) | 0;
  }
  function yc(a, b) {
    return (a === b && (0 !== a || 1 / a === 1 / b)) || (a !== a && b !== b);
  }
  var zc = "function" === typeof Object.is ? Object.is : yc,
    R = null,
    Ac = null,
    Bc = null,
    S = null,
    T = !1,
    Cc = !1,
    U = 0,
    V = null,
    Dc = 0;
  function W() {
    if (null === R) throw Error(k(321));
    return R;
  }
  function Ec() {
    if (0 < Dc) throw Error(k(312));
    return { memoizedState: null, queue: null, next: null };
  }
  function Fc() {
    null === S
      ? null === Bc
        ? ((T = !1), (Bc = S = Ec()))
        : ((T = !0), (S = Bc))
      : null === S.next
        ? ((T = !1), (S = S.next = Ec()))
        : ((T = !0), (S = S.next));
    return S;
  }
  function Gc() {
    Ac = R = null;
    Cc = !1;
    Bc = null;
    Dc = 0;
    S = V = null;
  }
  function Hc(a, b) {
    return "function" === typeof b ? b(a) : b;
  }
  function Ic(a, b, c) {
    R = W();
    S = Fc();
    if (T) {
      var d = S.queue;
      b = d.dispatch;
      if (null !== V && ((c = V.get(d)), void 0 !== c)) {
        V.delete(d);
        d = S.memoizedState;
        do (d = a(d, c.action)), (c = c.next);
        while (null !== c);
        S.memoizedState = d;
        return [d, b];
      }
      return [S.memoizedState, b];
    }
    a =
      a === Hc ? ("function" === typeof b ? b() : b) : void 0 !== c ? c(b) : b;
    S.memoizedState = a;
    a = S.queue = { last: null, dispatch: null };
    a = a.dispatch = Jc.bind(null, R, a);
    return [S.memoizedState, a];
  }
  function Kc(a, b) {
    R = W();
    S = Fc();
    b = void 0 === b ? null : b;
    if (null !== S) {
      var c = S.memoizedState;
      if (null !== c && null !== b) {
        var d = c[1];
        a: if (null === d) d = !1;
        else {
          for (var f = 0; f < d.length && f < b.length; f++)
            if (!zc(b[f], d[f])) {
              d = !1;
              break a;
            }
          d = !0;
        }
        if (d) return c[0];
      }
    }
    a = a();
    S.memoizedState = [a, b];
    return a;
  }
  function Jc(a, b, c) {
    if (25 <= Dc) throw Error(k(301));
    if (a === R)
      if (
        ((Cc = !0),
        (a = { action: c, next: null }),
        null === V && (V = new Map()),
        (c = V.get(b)),
        void 0 === c)
      )
        V.set(b, a);
      else {
        for (b = c; null !== b.next; ) b = b.next;
        b.next = a;
      }
  }
  function Lc() {
    throw Error(k(394));
  }
  function Mc() {}
  var Oc = {
      readContext: function (a) {
        return a._currentValue;
      },
      useContext: function (a) {
        W();
        return a._currentValue;
      },
      useMemo: Kc,
      useReducer: Ic,
      useRef: function (a) {
        R = W();
        S = Fc();
        var b = S.memoizedState;
        return null === b ? ((a = { current: a }), (S.memoizedState = a)) : b;
      },
      useState: function (a) {
        return Ic(Hc, a);
      },
      useInsertionEffect: Mc,
      useLayoutEffect: function () {},
      useCallback: function (a, b) {
        return Kc(function () {
          return a;
        }, b);
      },
      useImperativeHandle: Mc,
      useEffect: Mc,
      useDebugValue: Mc,
      useDeferredValue: function (a) {
        W();
        return a;
      },
      useTransition: function () {
        W();
        return [!1, Lc];
      },
      useId: function () {
        var a = Ac.treeContext;
        var b = a.overflow;
        a = a.id;
        a = (a & ~(1 << (32 - uc(a) - 1))).toString(32) + b;
        var c = Nc;
        if (null === c) throw Error(k(404));
        b = U++;
        a = ":" + c.idPrefix + "R" + a;
        0 < b && (a += "H" + b.toString(32));
        return a + ":";
      },
      useMutableSource: function (a, b) {
        W();
        return b(a._source);
      },
      useSyncExternalStore: function (a, b, c) {
        if (void 0 === c) throw Error(k(407));
        return c();
      },
    },
    Nc = null,
    Pc =
      aa.__SECRET_INTERNALS_DO_NOT_USE_OR_YOU_WILL_BE_FIRED
        .ReactCurrentDispatcher;
  function Qc(a) {
    console.error(a);
    return null;
  }
  function X() {}
  function Rc(a, b, c, d, f, e, g, h, m) {
    var q = [],
      r = new Set();
    b = {
      destination: null,
      responseState: b,
      progressiveChunkSize: void 0 === d ? 12800 : d,
      status: 0,
      fatalError: null,
      nextSegmentId: 0,
      allPendingTasks: 0,
      pendingRootTasks: 0,
      completedRootSegment: null,
      abortableTasks: r,
      pingedTasks: q,
      clientRenderedBoundaries: [],
      completedBoundaries: [],
      partialBoundaries: [],
      onError: void 0 === f ? Qc : f,
      onAllReady: void 0 === e ? X : e,
      onShellReady: void 0 === g ? X : g,
      onShellError: void 0 === h ? X : h,
      onFatalError: void 0 === m ? X : m,
    };
    c = Sc(b, 0, null, c, !1, !1);
    c.parentFlushed = !0;
    a = Tc(b, a, null, c, r, kc, null, sc);
    q.push(a);
    return b;
  }
  function Tc(a, b, c, d, f, e, g, h) {
    a.allPendingTasks++;
    null === c ? a.pendingRootTasks++ : c.pendingTasks++;
    var m = {
      node: b,
      ping: function () {
        var b = a.pingedTasks;
        b.push(m);
        1 === b.length && Uc(a);
      },
      blockedBoundary: c,
      blockedSegment: d,
      abortSet: f,
      legacyContext: e,
      context: g,
      treeContext: h,
    };
    f.add(m);
    return m;
  }
  function Sc(a, b, c, d, f, e) {
    return {
      status: 0,
      id: -1,
      index: b,
      parentFlushed: !1,
      chunks: [],
      children: [],
      formatContext: d,
      boundary: c,
      lastPushedText: f,
      textEmbedded: e,
    };
  }
  function Y(a, b) {
    a = a.onError(b);
    if (null != a && "string" !== typeof a)
      throw Error(
        'onError returned something with a type other than "string". onError should return a string and may return null or undefined but must not return anything else. It received something of type "' +
          typeof a +
          '" instead',
      );
    return a;
  }
  function Vc(a, b) {
    var c = a.onShellError;
    c(b);
    c = a.onFatalError;
    c(b);
    null !== a.destination
      ? ((a.status = 2), da(a.destination, b))
      : ((a.status = 1), (a.fatalError = b));
  }
  function Wc(a, b, c, d, f) {
    R = {};
    Ac = b;
    U = 0;
    for (a = c(d, f); Cc; )
      (Cc = !1), (U = 0), (Dc += 1), (S = null), (a = c(d, f));
    Gc();
    return a;
  }
  function Xc(a, b, c, d) {
    var f = c.render(),
      e = d.childContextTypes;
    if (null !== e && void 0 !== e) {
      var g = b.legacyContext;
      if ("function" !== typeof c.getChildContext) d = g;
      else {
        c = c.getChildContext();
        for (var h in c)
          if (!(h in e)) throw Error(k(108, jc(d) || "Unknown", h));
        d = N({}, g, c);
      }
      b.legacyContext = d;
      Z(a, b, f);
      b.legacyContext = g;
    } else Z(a, b, f);
  }
  function Yc(a, b) {
    if (a && a.defaultProps) {
      b = N({}, b);
      a = a.defaultProps;
      for (var c in a) void 0 === b[c] && (b[c] = a[c]);
      return b;
    }
    return b;
  }
  function Zc(a, b, c, d, f) {
    if ("function" === typeof c)
      if (c.prototype && c.prototype.isReactComponent) {
        f = lc(c, b.legacyContext);
        var e = c.contextType;
        e = new c(d, "object" === typeof e && null !== e ? e._currentValue : f);
        rc(e, c, d, f);
        Xc(a, b, e, c);
      } else {
        e = lc(c, b.legacyContext);
        f = Wc(a, b, c, d, e);
        var g = 0 !== U;
        if (
          "object" === typeof f &&
          null !== f &&
          "function" === typeof f.render &&
          void 0 === f.$$typeof
        )
          rc(f, c, d, e), Xc(a, b, f, c);
        else if (g) {
          d = b.treeContext;
          b.treeContext = tc(d, 1, 0);
          try {
            Z(a, b, f);
          } finally {
            b.treeContext = d;
          }
        } else Z(a, b, f);
      }
    else if ("string" === typeof c) {
      f = b.blockedSegment;
      e = Sa(f.chunks, c, d, a.responseState, f.formatContext);
      f.lastPushedText = !1;
      g = f.formatContext;
      f.formatContext = Ba(g, c, d);
      $c(a, b, e);
      f.formatContext = g;
      switch (c) {
        case "area":
        case "base":
        case "br":
        case "col":
        case "embed":
        case "hr":
        case "img":
        case "input":
        case "keygen":
        case "link":
        case "meta":
        case "param":
        case "source":
        case "track":
        case "wbr":
          break;
        default:
          f.chunks.push(Ta, u(c), Ua);
      }
      f.lastPushedText = !1;
    } else {
      switch (c) {
        case gc:
        case fc:
        case Wb:
        case Xb:
        case Vb:
          Z(a, b, d.children);
          return;
        case bc:
          Z(a, b, d.children);
          return;
        case ec:
          throw Error(k(343));
        case ac:
          a: {
            c = b.blockedBoundary;
            f = b.blockedSegment;
            e = d.fallback;
            d = d.children;
            g = new Set();
            var h = {
                id: null,
                rootSegmentID: -1,
                parentFlushed: !1,
                pendingTasks: 0,
                forceClientRender: !1,
                completedSegments: [],
                byteSize: 0,
                fallbackAbortableTasks: g,
                errorDigest: null,
              },
              m = Sc(a, f.chunks.length, h, f.formatContext, !1, !1);
            f.children.push(m);
            f.lastPushedText = !1;
            var q = Sc(a, 0, null, f.formatContext, !1, !1);
            q.parentFlushed = !0;
            b.blockedBoundary = h;
            b.blockedSegment = q;
            try {
              if (
                ($c(a, b, d),
                q.lastPushedText && q.textEmbedded && q.chunks.push(Ca),
                (q.status = 1),
                ad(h, q),
                0 === h.pendingTasks)
              )
                break a;
            } catch (r) {
              (q.status = 4),
                (h.forceClientRender = !0),
                (h.errorDigest = Y(a, r));
            } finally {
              (b.blockedBoundary = c), (b.blockedSegment = f);
            }
            b = Tc(a, e, c, m, g, b.legacyContext, b.context, b.treeContext);
            a.pingedTasks.push(b);
          }
          return;
      }
      if ("object" === typeof c && null !== c)
        switch (c.$$typeof) {
          case $b:
            d = Wc(a, b, c.render, d, f);
            if (0 !== U) {
              c = b.treeContext;
              b.treeContext = tc(c, 1, 0);
              try {
                Z(a, b, d);
              } finally {
                b.treeContext = c;
              }
            } else Z(a, b, d);
            return;
          case cc:
            c = c.type;
            d = Yc(c, d);
            Zc(a, b, c, d, f);
            return;
          case Yb:
            f = d.children;
            c = c._context;
            d = d.value;
            e = c._currentValue;
            c._currentValue = d;
            g = O;
            O = d = {
              parent: g,
              depth: null === g ? 0 : g.depth + 1,
              context: c,
              parentValue: e,
              value: d,
            };
            b.context = d;
            Z(a, b, f);
            a = O;
            if (null === a) throw Error(k(403));
            d = a.parentValue;
            a.context._currentValue = d === hc ? a.context._defaultValue : d;
            a = O = a.parent;
            b.context = a;
            return;
          case Zb:
            d = d.children;
            d = d(c._currentValue);
            Z(a, b, d);
            return;
          case dc:
            f = c._init;
            c = f(c._payload);
            d = Yc(c, d);
            Zc(a, b, c, d, void 0);
            return;
        }
      throw Error(k(130, null == c ? c : typeof c, ""));
    }
  }
  function Z(a, b, c) {
    b.node = c;
    if ("object" === typeof c && null !== c) {
      switch (c.$$typeof) {
        case Tb:
          Zc(a, b, c.type, c.props, c.ref);
          return;
        case Ub:
          throw Error(k(257));
        case dc:
          var d = c._init;
          c = d(c._payload);
          Z(a, b, c);
          return;
      }
      if (ra(c)) {
        bd(a, b, c);
        return;
      }
      null === c || "object" !== typeof c
        ? (d = null)
        : ((d = (ic && c[ic]) || c["@@iterator"]),
          (d = "function" === typeof d ? d : null));
      if (d && (d = d.call(c))) {
        c = d.next();
        if (!c.done) {
          var f = [];
          do f.push(c.value), (c = d.next());
          while (!c.done);
          bd(a, b, f);
        }
        return;
      }
      a = Object.prototype.toString.call(c);
      throw Error(
        k(
          31,
          "[object Object]" === a
            ? "object with keys {" + Object.keys(c).join(", ") + "}"
            : a,
        ),
      );
    }
    "string" === typeof c
      ? ((d = b.blockedSegment),
        (d.lastPushedText = Da(
          b.blockedSegment.chunks,
          c,
          a.responseState,
          d.lastPushedText,
        )))
      : "number" === typeof c &&
        ((d = b.blockedSegment),
        (d.lastPushedText = Da(
          b.blockedSegment.chunks,
          "" + c,
          a.responseState,
          d.lastPushedText,
        )));
  }
  function bd(a, b, c) {
    for (var d = c.length, f = 0; f < d; f++) {
      var e = b.treeContext;
      b.treeContext = tc(e, d, f);
      try {
        $c(a, b, c[f]);
      } finally {
        b.treeContext = e;
      }
    }
  }
  function $c(a, b, c) {
    var d = b.blockedSegment.formatContext,
      f = b.legacyContext,
      e = b.context;
    try {
      return Z(a, b, c);
    } catch (m) {
      if (
        (Gc(),
        "object" === typeof m && null !== m && "function" === typeof m.then)
      ) {
        c = m;
        var g = b.blockedSegment,
          h = Sc(
            a,
            g.chunks.length,
            null,
            g.formatContext,
            g.lastPushedText,
            !0,
          );
        g.children.push(h);
        g.lastPushedText = !1;
        a = Tc(
          a,
          b.node,
          b.blockedBoundary,
          h,
          b.abortSet,
          b.legacyContext,
          b.context,
          b.treeContext,
        ).ping;
        c.then(a, a);
        b.blockedSegment.formatContext = d;
        b.legacyContext = f;
        b.context = e;
        Q(e);
      } else
        throw (
          ((b.blockedSegment.formatContext = d),
          (b.legacyContext = f),
          (b.context = e),
          Q(e),
          m)
        );
    }
  }
  function cd(a) {
    var b = a.blockedBoundary;
    a = a.blockedSegment;
    a.status = 3;
    dd(this, b, a);
  }
  function ed(a, b, c) {
    var d = a.blockedBoundary;
    a.blockedSegment.status = 3;
    null === d
      ? (b.allPendingTasks--,
        2 !== b.status &&
          ((b.status = 2), null !== b.destination && b.destination.close()))
      : (d.pendingTasks--,
        d.forceClientRender ||
          ((d.forceClientRender = !0),
          (a = void 0 === c ? Error(k(432)) : c),
          (d.errorDigest = b.onError(a)),
          d.parentFlushed && b.clientRenderedBoundaries.push(d)),
        d.fallbackAbortableTasks.forEach(function (a) {
          return ed(a, b, c);
        }),
        d.fallbackAbortableTasks.clear(),
        b.allPendingTasks--,
        0 === b.allPendingTasks && ((d = b.onAllReady), d()));
  }
  function ad(a, b) {
    if (
      0 === b.chunks.length &&
      1 === b.children.length &&
      null === b.children[0].boundary
    ) {
      var c = b.children[0];
      c.id = b.id;
      c.parentFlushed = !0;
      1 === c.status && ad(a, c);
    } else a.completedSegments.push(b);
  }
  function dd(a, b, c) {
    if (null === b) {
      if (c.parentFlushed) {
        if (null !== a.completedRootSegment) throw Error(k(389));
        a.completedRootSegment = c;
      }
      a.pendingRootTasks--;
      0 === a.pendingRootTasks &&
        ((a.onShellError = X), (b = a.onShellReady), b());
    } else
      b.pendingTasks--,
        b.forceClientRender ||
          (0 === b.pendingTasks
            ? (c.parentFlushed && 1 === c.status && ad(b, c),
              b.parentFlushed && a.completedBoundaries.push(b),
              b.fallbackAbortableTasks.forEach(cd, a),
              b.fallbackAbortableTasks.clear())
            : c.parentFlushed &&
              1 === c.status &&
              (ad(b, c),
              1 === b.completedSegments.length &&
                b.parentFlushed &&
                a.partialBoundaries.push(b)));
    a.allPendingTasks--;
    0 === a.allPendingTasks && ((a = a.onAllReady), a());
  }
  function Uc(a) {
    if (2 !== a.status) {
      var b = O,
        c = Pc.current;
      Pc.current = Oc;
      var d = Nc;
      Nc = a.responseState;
      try {
        var f = a.pingedTasks,
          e;
        for (e = 0; e < f.length; e++) {
          var g = f[e];
          var h = a,
            m = g.blockedSegment;
          if (0 === m.status) {
            Q(g.context);
            try {
              Z(h, g, g.node),
                m.lastPushedText && m.textEmbedded && m.chunks.push(Ca),
                g.abortSet.delete(g),
                (m.status = 1),
                dd(h, g.blockedBoundary, m);
            } catch (G) {
              if (
                (Gc(),
                "object" === typeof G &&
                  null !== G &&
                  "function" === typeof G.then)
              ) {
                var q = g.ping;
                G.then(q, q);
              } else {
                g.abortSet.delete(g);
                m.status = 4;
                var r = g.blockedBoundary,
                  v = G,
                  A = Y(h, v);
                null === r
                  ? Vc(h, v)
                  : (r.pendingTasks--,
                    r.forceClientRender ||
                      ((r.forceClientRender = !0),
                      (r.errorDigest = A),
                      r.parentFlushed && h.clientRenderedBoundaries.push(r)));
                h.allPendingTasks--;
                if (0 === h.allPendingTasks) {
                  var F = h.onAllReady;
                  F();
                }
              }
            } finally {
            }
          }
        }
        f.splice(0, e);
        null !== a.destination && fd(a, a.destination);
      } catch (G) {
        Y(a, G), Vc(a, G);
      } finally {
        (Nc = d), (Pc.current = c), c === Oc && Q(b);
      }
    }
  }
  function gd(a, b, c) {
    c.parentFlushed = !0;
    switch (c.status) {
      case 0:
        var d = (c.id = a.nextSegmentId++);
        c.lastPushedText = !1;
        c.textEmbedded = !1;
        a = a.responseState;
        p(b, Va);
        p(b, a.placeholderPrefix);
        a = u(d.toString(16));
        p(b, a);
        return t(b, Wa);
      case 1:
        c.status = 2;
        var f = !0;
        d = c.chunks;
        var e = 0;
        c = c.children;
        for (var g = 0; g < c.length; g++) {
          for (f = c[g]; e < f.index; e++) p(b, d[e]);
          f = hd(a, b, f);
        }
        for (; e < d.length - 1; e++) p(b, d[e]);
        e < d.length && (f = t(b, d[e]));
        return f;
      default:
        throw Error(k(390));
    }
  }
  function hd(a, b, c) {
    var d = c.boundary;
    if (null === d) return gd(a, b, c);
    d.parentFlushed = !0;
    if (d.forceClientRender)
      (d = d.errorDigest),
        t(b, $a),
        p(b, bb),
        d && (p(b, db), p(b, u(C(d))), p(b, cb)),
        t(b, eb),
        gd(a, b, c);
    else if (0 < d.pendingTasks) {
      d.rootSegmentID = a.nextSegmentId++;
      0 < d.completedSegments.length && a.partialBoundaries.push(d);
      var f = a.responseState;
      var e = f.nextSuspenseID++;
      f = w(f.boundaryPrefix + e.toString(16));
      d = d.id = f;
      fb(b, a.responseState, d);
      gd(a, b, c);
    } else if (d.byteSize > a.progressiveChunkSize)
      (d.rootSegmentID = a.nextSegmentId++),
        a.completedBoundaries.push(d),
        fb(b, a.responseState, d.id),
        gd(a, b, c);
    else {
      t(b, Xa);
      c = d.completedSegments;
      if (1 !== c.length) throw Error(k(391));
      hd(a, b, c[0]);
    }
    return t(b, ab);
  }
  function id(a, b, c) {
    Bb(b, a.responseState, c.formatContext, c.id);
    hd(a, b, c);
    return Cb(b, c.formatContext);
  }
  function jd(a, b, c) {
    for (var d = c.completedSegments, f = 0; f < d.length; f++)
      kd(a, b, c, d[f]);
    d.length = 0;
    a = a.responseState;
    d = c.id;
    c = c.rootSegmentID;
    p(b, a.startInlineScript);
    a.sentCompleteBoundaryFunction
      ? p(b, Jb)
      : ((a.sentCompleteBoundaryFunction = !0), p(b, Ib));
    if (null === d) throw Error(k(395));
    c = u(c.toString(16));
    p(b, d);
    p(b, Kb);
    p(b, a.segmentPrefix);
    p(b, c);
    return t(b, Lb);
  }
  function kd(a, b, c, d) {
    if (2 === d.status) return !0;
    var f = d.id;
    if (-1 === f) {
      if (-1 === (d.id = c.rootSegmentID)) throw Error(k(392));
      return id(a, b, d);
    }
    id(a, b, d);
    a = a.responseState;
    p(b, a.startInlineScript);
    a.sentCompleteSegmentFunction
      ? p(b, Eb)
      : ((a.sentCompleteSegmentFunction = !0), p(b, Db));
    p(b, a.segmentPrefix);
    f = u(f.toString(16));
    p(b, f);
    p(b, Gb);
    p(b, a.placeholderPrefix);
    p(b, f);
    return t(b, Hb);
  }
  function fd(a, b) {
    l = new Uint8Array(512);
    n = 0;
    try {
      var c = a.completedRootSegment;
      if (null !== c && 0 === a.pendingRootTasks) {
        hd(a, b, c);
        a.completedRootSegment = null;
        var d = a.responseState.bootstrapChunks;
        for (c = 0; c < d.length - 1; c++) p(b, d[c]);
        c < d.length && t(b, d[c]);
      }
      var f = a.clientRenderedBoundaries,
        e;
      for (e = 0; e < f.length; e++) {
        var g = f[e];
        d = b;
        var h = a.responseState,
          m = g.id,
          q = g.errorDigest,
          r = g.errorMessage,
          v = g.errorComponentStack;
        p(d, h.startInlineScript);
        h.sentClientRenderFunction
          ? p(d, Nb)
          : ((h.sentClientRenderFunction = !0), p(d, Mb));
        if (null === m) throw Error(k(395));
        p(d, m);
        p(d, Ob);
        if (q || r || v) p(d, Qb), p(d, u(Sb(q || "")));
        if (r || v) p(d, Qb), p(d, u(Sb(r || "")));
        v && (p(d, Qb), p(d, u(Sb(v))));
        if (!t(d, Pb)) {
          a.destination = null;
          e++;
          f.splice(0, e);
          return;
        }
      }
      f.splice(0, e);
      var A = a.completedBoundaries;
      for (e = 0; e < A.length; e++)
        if (!jd(a, b, A[e])) {
          a.destination = null;
          e++;
          A.splice(0, e);
          return;
        }
      A.splice(0, e);
      ba(b);
      l = new Uint8Array(512);
      n = 0;
      var F = a.partialBoundaries;
      for (e = 0; e < F.length; e++) {
        var G = F[e];
        a: {
          f = a;
          g = b;
          var ma = G.completedSegments;
          for (h = 0; h < ma.length; h++)
            if (!kd(f, g, G, ma[h])) {
              h++;
              ma.splice(0, h);
              var Fb = !1;
              break a;
            }
          ma.splice(0, h);
          Fb = !0;
        }
        if (!Fb) {
          a.destination = null;
          e++;
          F.splice(0, e);
          return;
        }
      }
      F.splice(0, e);
      var na = a.completedBoundaries;
      for (e = 0; e < na.length; e++)
        if (!jd(a, b, na[e])) {
          a.destination = null;
          e++;
          na.splice(0, e);
          return;
        }
      na.splice(0, e);
    } finally {
      ba(b),
        0 === a.allPendingTasks &&
          0 === a.pingedTasks.length &&
          0 === a.clientRenderedBoundaries.length &&
          0 === a.completedBoundaries.length &&
          b.close();
    }
  }
  function ld(a, b) {
    try {
      var c = a.abortableTasks;
      c.forEach(function (c) {
        return ed(c, a, b);
      });
      c.clear();
      null !== a.destination && fd(a, a.destination);
    } catch (d) {
      Y(a, d), Vc(a, d);
    }
  }
  renderToReadableStream = function (a, b) {
    return new Promise(function (c, d) {
      var f,
        e,
        g = new Promise(function (a, b) {
          e = a;
          f = b;
        }),
        h = Rc(
          a,
          za(
            b ? b.identifierPrefix : void 0,
            b ? b.nonce : void 0,
            b ? b.bootstrapScriptContent : void 0,
            b ? b.bootstrapScripts : void 0,
            b ? b.bootstrapModules : void 0,
          ),
          Aa(b ? b.namespaceURI : void 0),
          b ? b.progressiveChunkSize : void 0,
          b ? b.onError : void 0,
          e,
          function () {
            var a = new ReadableStream(
              {
                type: "bytes",
                pull: function (a) {
                  if (1 === h.status) (h.status = 2), da(a, h.fatalError);
                  else if (2 !== h.status && null === h.destination) {
                    h.destination = a;
                    try {
                      fd(h, a);
                    } catch (A) {
                      Y(h, A), Vc(h, A);
                    }
                  }
                },
                cancel: function () {
                  ld(h);
                },
              },
              { highWaterMark: 0 },
            );
            a.allReady = g;
            c(a);
          },
          function (a) {
            g.catch(function () {});
            d(a);
          },
          f,
        );
      if (b && b.signal) {
        var m = b.signal,
          q = function () {
            ld(h, m.reason);
            m.removeEventListener("abort", q);
          };
        m.addEventListener("abort", q);
      }
      Uc(h);
    });
  };
}

routes.set("/react-byob", async () => {
  const stream = await renderToReadableStream(jsx(App, {}), {
    bootstrapScripts: ["/main.js"],
  });
  return new Response(stream, {
    headers: {
      "content-type": "text/html",
    },
  });

  function App() {
    const qq = typeof jsx !== "undefined" ? jsx : q;
    return qq("html", {
      children: [
        qq("title", {
          children: "My app",
        }),
        qq("body", {
          children: qq("h1", {
            children: "App",
          }),
        }),
      ],
    });
  }
});
