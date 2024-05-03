import { pass, assert, assertThrows } from "./assertions.js";
import { routes } from "./routes";

routes.set('/btoa', () => {
  let error;
  // btoa
  {
    var everything = "";
    for (var i = 0; i < 256; i++) {
      everything += String.fromCharCode(i);
    }
    error = assert(btoa(everything), 'AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==');
    if (error) { return error; }

    error = assert(btoa(42), btoa('42'));
    if (error) { return error; }
    error = assert(btoa(null), btoa('null'));
    if (error) { return error; }
    error = assert(btoa({ x: 1 }), btoa('[object Object]'));
    if (error) { return error; }

    error = assertThrows(() => btoa(), TypeError);
    if (error) { return error; }
    error = assertThrows(() => btoa('🐱'));
    if (error) { return error; }

    error = assert(btoa(""), "", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa("ab"), "YWI=", `btoa("ab")`)
    if (error) { return error; }
    error = assert(btoa("abc"), "YWJj", `btoa("abc")`)
    if (error) { return error; }
    error = assert(btoa("abcd"), "YWJjZA==", `btoa("abcd")`)
    if (error) { return error; }
    error = assert(btoa("abcde"), "YWJjZGU=", `btoa("abcde")`)
    if (error) { return error; }
    error = assert(btoa("ÿÿÀ"), "///A", `btoa("ÿÿÀ")`)
    if (error) { return error; }
    error = assert(btoa("\0a"), "AGE=", `btoa("\\0a")`)
    if (error) { return error; }
    error = assert(btoa("a\0b"), "YQBi", `btoa("a\\0b")`)
    if (error) { return error; }
    error = assert(btoa(undefined), "dW5kZWZpbmVk", `btoa(undefined)`)
    if (error) { return error; }
    error = assert(btoa(null), "bnVsbA==", `btoa(null)`)
    if (error) { return error; }
    error = assert(btoa(7), "Nw==", `btoa(7)`)
    if (error) { return error; }
    error = assert(btoa(12), "MTI=", `btoa(12)`)
    if (error) { return error; }
    error = assert(btoa(1.5), "MS41", `btoa(1.5)`)
    if (error) { return error; }
    error = assert(btoa(true), "dHJ1ZQ==", `btoa(true)`)
    if (error) { return error; }
    error = assert(btoa(false), "ZmFsc2U=", `btoa(false)`)
    if (error) { return error; }
    error = assert(btoa(NaN), "TmFO", `btoa(NaN)`)
    if (error) { return error; }
    error = assert(btoa(Infinity), "SW5maW5pdHk=", `btoa(Infinity)`)
    if (error) { return error; }
    error = assert(btoa(-Infinity), "LUluZmluaXR5", `btoa(-Infinity)`)
    if (error) { return error; }
    error = assert(btoa(0), "MA==", `btoa(0)`)
    if (error) { return error; }
    error = assert(btoa(-0), "MA==", `btoa(-0)`)
    if (error) { return error; }
    if (error) { return error; }
    error = assert(btoa("\0"), "AA==", `btoa("\\0")`)
    if (error) { return error; }
    error = assert(btoa("\x01"), "AQ==", `btoa("\\x01")`)
    if (error) { return error; }
    error = assert(btoa("\x02"), "Ag==", `btoa("\\x02")`)
    if (error) { return error; }
    error = assert(btoa("\x03"), "Aw==", `btoa("\\x03")`)
    if (error) { return error; }
    error = assert(btoa("\x04"), "BA==", `btoa("\\x04")`)
    if (error) { return error; }
    error = assert(btoa("\x05"), "BQ==", `btoa("\\x05")`)
    if (error) { return error; }
    error = assert(btoa("\x06"), "Bg==", `btoa("\\x06")`)
    if (error) { return error; }
    error = assert(btoa("\x07"), "Bw==", `btoa("\\x07")`)
    if (error) { return error; }
    error = assert(btoa("\b"), "CA==", `btoa("\\b")`)
    if (error) { return error; }
    error = assert(btoa("\t"), "CQ==", `btoa("\\t")`)
    if (error) { return error; }
    error = assert(btoa("\n"), "Cg==", `btoa("\\n")`)
    if (error) { return error; }
    error = assert(btoa("\v"), "Cw==", `btoa("\\v")`)
    if (error) { return error; }
    error = assert(btoa("\f"), "DA==", `btoa("\\f")`)
    if (error) { return error; }
    error = assert(btoa("\r"), "DQ==", `btoa("\\r")`)
    if (error) { return error; }
    error = assert(btoa("\x0e"), "Dg==", `btoa("\\x0e")`)
    if (error) { return error; }
    error = assert(btoa("\x0f"), "Dw==", `btoa("\\x0f")`)
    if (error) { return error; }
    error = assert(btoa("\x10"), "EA==", `btoa("\\x10")`)
    if (error) { return error; }
    error = assert(btoa("\x11"), "EQ==", `btoa("\\x11")`)
    if (error) { return error; }
    error = assert(btoa("\x12"), "Eg==", `btoa("\\x12")`)
    if (error) { return error; }
    error = assert(btoa("\x13"), "Ew==", `btoa("\\x13")`)
    if (error) { return error; }
    error = assert(btoa("\x14"), "FA==", `btoa("\\x14")`)
    if (error) { return error; }
    error = assert(btoa("\x15"), "FQ==", `btoa("\\x15")`)
    if (error) { return error; }
    error = assert(btoa("\x16"), "Fg==", `btoa("\\x16")`)
    if (error) { return error; }
    error = assert(btoa("\x17"), "Fw==", `btoa("\\x17")`)
    if (error) { return error; }
    error = assert(btoa("\x18"), "GA==", `btoa("\\x18")`)
    if (error) { return error; }
    error = assert(btoa("\x19"), "GQ==", `btoa("\\x19")`)
    if (error) { return error; }
    error = assert(btoa("\x1a"), "Gg==", `btoa("\\x1a")`)
    if (error) { return error; }
    error = assert(btoa("\x1b"), "Gw==", `btoa("\\x1b")`)
    if (error) { return error; }
    error = assert(btoa("\x1c"), "HA==", `btoa("\\x1c")`)
    if (error) { return error; }
    error = assert(btoa("\x1d"), "HQ==", `btoa("\\x1d")`)
    if (error) { return error; }
    error = assert(btoa("\x1e"), "Hg==", `btoa("\\x1e")`)
    if (error) { return error; }
    error = assert(btoa("\x1f"), "Hw==", `btoa("\\x1f")`)
    if (error) { return error; }
    error = assert(btoa(" "), "IA==", `btoa(" ")`)
    if (error) { return error; }
    error = assert(btoa("!"), "IQ==", `btoa("!")`)
    if (error) { return error; }
    error = assert(btoa("#"), "Iw==", `btoa("#")`)
    if (error) { return error; }
    error = assert(btoa("$"), "JA==", `btoa("$")`)
    if (error) { return error; }
    error = assert(btoa("%"), "JQ==", `btoa("%")`)
    if (error) { return error; }
    error = assert(btoa("&"), "Jg==", `btoa("&")`)
    if (error) { return error; }
    error = assert(btoa("'"), "Jw==", `btoa("'")`)
    if (error) { return error; }
    error = assert(btoa("("), "KA==", `btoa("(")`)
    if (error) { return error; }
    error = assert(btoa(")"), "KQ==", `btoa(")")`)
    if (error) { return error; }
    error = assert(btoa("*"), "Kg==", `btoa("*")`)
    if (error) { return error; }
    error = assert(btoa("+"), "Kw==", `btoa("+")`)
    if (error) { return error; }
    error = assert(btoa(","), "LA==", `btoa(",")`)
    if (error) { return error; }
    error = assert(btoa("-"), "LQ==", `btoa("-")`)
    if (error) { return error; }
    error = assert(btoa("."), "Lg==", `btoa(".")`)
    if (error) { return error; }
    error = assert(btoa("/"), "Lw==", `btoa("/")`)
    if (error) { return error; }
    error = assert(btoa("0"), "MA==", `btoa("0")`)
    if (error) { return error; }
    error = assert(btoa("1"), "MQ==", `btoa("1")`)
    if (error) { return error; }
    error = assert(btoa("2"), "Mg==", `btoa("2")`)
    if (error) { return error; }
    error = assert(btoa("3"), "Mw==", `btoa("3")`)
    if (error) { return error; }
    error = assert(btoa("4"), "NA==", `btoa("4")`)
    if (error) { return error; }
    error = assert(btoa("5"), "NQ==", `btoa("5")`)
    if (error) { return error; }
    error = assert(btoa("6"), "Ng==", `btoa("6")`)
    if (error) { return error; }
    error = assert(btoa("7"), "Nw==", `btoa("7")`)
    if (error) { return error; }
    error = assert(btoa("8"), "OA==", `btoa("8")`)
    if (error) { return error; }
    error = assert(btoa("9"), "OQ==", `btoa("9")`)
    if (error) { return error; }
    error = assert(btoa(":"), "Og==", `btoa(":")`)
    if (error) { return error; }
    error = assert(btoa(";"), "Ow==", `btoa(";")`)
    if (error) { return error; }
    error = assert(btoa("<"), "PA==", `btoa("<")`)
    if (error) { return error; }
    error = assert(btoa("="), "PQ==", `btoa("=")`)
    if (error) { return error; }
    error = assert(btoa(">"), "Pg==", `btoa(">")`)
    if (error) { return error; }
    error = assert(btoa("?"), "Pw==", `btoa("?")`)
    if (error) { return error; }
    error = assert(btoa("@"), "QA==", `btoa("@")`)
    if (error) { return error; }
    error = assert(btoa("A"), "QQ==", `btoa("A")`)
    if (error) { return error; }
    error = assert(btoa("B"), "Qg==", `btoa("B")`)
    if (error) { return error; }
    error = assert(btoa("C"), "Qw==", `btoa("C")`)
    if (error) { return error; }
    error = assert(btoa("D"), "RA==", `btoa("D")`)
    if (error) { return error; }
    error = assert(btoa("E"), "RQ==", `btoa("E")`)
    if (error) { return error; }
    error = assert(btoa("F"), "Rg==", `btoa("F")`)
    if (error) { return error; }
    error = assert(btoa("G"), "Rw==", `btoa("G")`)
    if (error) { return error; }
    error = assert(btoa("H"), "SA==", `btoa("H")`)
    if (error) { return error; }
    error = assert(btoa("I"), "SQ==", `btoa("I")`)
    if (error) { return error; }
    error = assert(btoa("J"), "Sg==", `btoa("J")`)
    if (error) { return error; }
    error = assert(btoa("K"), "Sw==", `btoa("K")`)
    if (error) { return error; }
    error = assert(btoa("L"), "TA==", `btoa("L")`)
    if (error) { return error; }
    error = assert(btoa("M"), "TQ==", `btoa("M")`)
    if (error) { return error; }
    error = assert(btoa("N"), "Tg==", `btoa("N")`)
    if (error) { return error; }
    error = assert(btoa("O"), "Tw==", `btoa("O")`)
    if (error) { return error; }
    error = assert(btoa("P"), "UA==", `btoa("P")`)
    if (error) { return error; }
    error = assert(btoa("Q"), "UQ==", `btoa("Q")`)
    if (error) { return error; }
    error = assert(btoa("R"), "Ug==", `btoa("R")`)
    if (error) { return error; }
    error = assert(btoa("S"), "Uw==", `btoa("S")`)
    if (error) { return error; }
    error = assert(btoa("T"), "VA==", `btoa("T")`)
    if (error) { return error; }
    error = assert(btoa("U"), "VQ==", `btoa("U")`)
    if (error) { return error; }
    error = assert(btoa("V"), "Vg==", `btoa("V")`)
    if (error) { return error; }
    error = assert(btoa("W"), "Vw==", `btoa("W")`)
    if (error) { return error; }
    error = assert(btoa("X"), "WA==", `btoa("X")`)
    if (error) { return error; }
    error = assert(btoa("Y"), "WQ==", `btoa("Y")`)
    if (error) { return error; }
    error = assert(btoa("Z"), "Wg==", `btoa("Z")`)
    if (error) { return error; }
    error = assert(btoa("["), "Ww==", `btoa("[")`)
    if (error) { return error; }
    error = assert(btoa("\\"), "XA==", `btoa("\\\\")`)
    if (error) { return error; }
    error = assert(btoa("]"), "XQ==", `btoa("]")`)
    if (error) { return error; }
    error = assert(btoa("^"), "Xg==", `btoa("^")`)
    if (error) { return error; }
    error = assert(btoa("_"), "Xw==", `btoa("_")`)
    if (error) { return error; }
    error = assert(btoa("a"), "YQ==", `btoa("a")`)
    if (error) { return error; }
    error = assert(btoa("b"), "Yg==", `btoa("b")`)
    if (error) { return error; }
    error = assert(btoa("c"), "Yw==", `btoa("c")`)
    if (error) { return error; }
    error = assert(btoa("d"), "ZA==", `btoa("d")`)
    if (error) { return error; }
    error = assert(btoa("e"), "ZQ==", `btoa("e")`)
    if (error) { return error; }
    error = assert(btoa("f"), "Zg==", `btoa("f")`)
    if (error) { return error; }
    error = assert(btoa("g"), "Zw==", `btoa("g")`)
    if (error) { return error; }
    error = assert(btoa("h"), "aA==", `btoa("h")`)
    if (error) { return error; }
    error = assert(btoa("i"), "aQ==", `btoa("i")`)
    if (error) { return error; }
    error = assert(btoa("j"), "ag==", `btoa("j")`)
    if (error) { return error; }
    error = assert(btoa("k"), "aw==", `btoa("k")`)
    if (error) { return error; }
    error = assert(btoa("l"), "bA==", `btoa("l")`)
    if (error) { return error; }
    error = assert(btoa("m"), "bQ==", `btoa("m")`)
    if (error) { return error; }
    error = assert(btoa("n"), "bg==", `btoa("n")`)
    if (error) { return error; }
    error = assert(btoa("o"), "bw==", `btoa("o")`)
    if (error) { return error; }
    error = assert(btoa("p"), "cA==", `btoa("p")`)
    if (error) { return error; }
    error = assert(btoa("q"), "cQ==", `btoa("q")`)
    if (error) { return error; }
    error = assert(btoa("r"), "cg==", `btoa("r")`)
    if (error) { return error; }
    error = assert(btoa("s"), "cw==", `btoa("s")`)
    if (error) { return error; }
    error = assert(btoa("t"), "dA==", `btoa("t")`)
    if (error) { return error; }
    error = assert(btoa("u"), "dQ==", `btoa("u")`)
    if (error) { return error; }
    error = assert(btoa("v"), "dg==", `btoa("v")`)
    if (error) { return error; }
    error = assert(btoa("w"), "dw==", `btoa("w")`)
    if (error) { return error; }
    error = assert(btoa("x"), "eA==", `btoa("x")`)
    if (error) { return error; }
    error = assert(btoa("y"), "eQ==", `btoa("y")`)
    if (error) { return error; }
    error = assert(btoa("z"), "eg==", `btoa("z")`)
    if (error) { return error; }
    error = assert(btoa("{"), "ew==", `btoa("{")`)
    if (error) { return error; }
    error = assert(btoa("|"), "fA==", `btoa("|")`)
    if (error) { return error; }
    error = assert(btoa("}"), "fQ==", `btoa("}")`)
    if (error) { return error; }
    error = assert(btoa("~"), "fg==", `btoa("~")`)
    if (error) { return error; }
    error = assert(btoa(""), "fw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "gA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "gQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "gg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "gw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "hA==", `btoa("")`)
    if (error) { return error; }
    // eslint-disable-next-line no-irregular-whitespace
    error = assert(btoa(""), "hQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "hg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "hw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "iA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "iQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "ig==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "iw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "jA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "jQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "jg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "jw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "kA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "kQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "kg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "kw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "lA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "lQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "lg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "lw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "mA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "mQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "mg==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "mw==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "nA==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "nQ==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "ng==", `btoa("")`)
    if (error) { return error; }
    error = assert(btoa(""), "nw==", `btoa("")`)
    if (error) { return error; }
    // eslint-disable-next-line no-irregular-whitespace
    error = assert(btoa(" "), "oA==", `btoa(" ")`)
    if (error) { return error; }
    error = assert(btoa("¡"), "oQ==", `btoa("¡")`)
    if (error) { return error; }
    error = assert(btoa("¢"), "og==", `btoa("¢")`)
    if (error) { return error; }
    error = assert(btoa("£"), "ow==", `btoa("£")`)
    if (error) { return error; }
    error = assert(btoa("¤"), "pA==", `btoa("¤")`)
    if (error) { return error; }
    error = assert(btoa("¥"), "pQ==", `btoa("¥")`)
    if (error) { return error; }
    error = assert(btoa("¦"), "pg==", `btoa("¦")`)
    if (error) { return error; }
    error = assert(btoa("§"), "pw==", `btoa("§")`)
    if (error) { return error; }
    error = assert(btoa("¨"), "qA==", `btoa("¨")`)
    if (error) { return error; }
    error = assert(btoa("©"), "qQ==", `btoa("©")`)
    if (error) { return error; }
    error = assert(btoa("ª"), "qg==", `btoa("ª")`)
    if (error) { return error; }
    error = assert(btoa("«"), "qw==", `btoa("«")`)
    if (error) { return error; }
    error = assert(btoa("¬"), "rA==", `btoa("¬")`)
    if (error) { return error; }
    error = assert(btoa("­"), "rQ==", `btoa("­")`)
    if (error) { return error; }
    error = assert(btoa("®"), "rg==", `btoa("®")`)
    if (error) { return error; }
    error = assert(btoa("¯"), "rw==", `btoa("¯")`)
    if (error) { return error; }
    error = assert(btoa("°"), "sA==", `btoa("°")`)
    if (error) { return error; }
    error = assert(btoa("±"), "sQ==", `btoa("±")`)
    if (error) { return error; }
    error = assert(btoa("²"), "sg==", `btoa("²")`)
    if (error) { return error; }
    error = assert(btoa("³"), "sw==", `btoa("³")`)
    if (error) { return error; }
    error = assert(btoa("´"), "tA==", `btoa("´")`)
    if (error) { return error; }
    error = assert(btoa("µ"), "tQ==", `btoa("µ")`)
    if (error) { return error; }
    error = assert(btoa("¶"), "tg==", `btoa("¶")`)
    if (error) { return error; }
    error = assert(btoa("·"), "tw==", `btoa("·")`)
    if (error) { return error; }
    error = assert(btoa("¸"), "uA==", `btoa("¸")`)
    if (error) { return error; }
    error = assert(btoa("¹"), "uQ==", `btoa("¹")`)
    if (error) { return error; }
    error = assert(btoa("º"), "ug==", `btoa("º")`)
    if (error) { return error; }
    error = assert(btoa("»"), "uw==", `btoa("»")`)
    if (error) { return error; }
    error = assert(btoa("¼"), "vA==", `btoa("¼")`)
    if (error) { return error; }
    error = assert(btoa("½"), "vQ==", `btoa("½")`)
    if (error) { return error; }
    error = assert(btoa("¾"), "vg==", `btoa("¾")`)
    if (error) { return error; }
    error = assert(btoa("¿"), "vw==", `btoa("¿")`)
    if (error) { return error; }
    error = assert(btoa("À"), "wA==", `btoa("À")`)
    if (error) { return error; }
    error = assert(btoa("Á"), "wQ==", `btoa("Á")`)
    if (error) { return error; }
    error = assert(btoa("Â"), "wg==", `btoa("Â")`)
    if (error) { return error; }
    error = assert(btoa("Ã"), "ww==", `btoa("Ã")`)
    if (error) { return error; }
    error = assert(btoa("Ä"), "xA==", `btoa("Ä")`)
    if (error) { return error; }
    error = assert(btoa("Å"), "xQ==", `btoa("Å")`)
    if (error) { return error; }
    error = assert(btoa("Æ"), "xg==", `btoa("Æ")`)
    if (error) { return error; }
    error = assert(btoa("Ç"), "xw==", `btoa("Ç")`)
    if (error) { return error; }
    error = assert(btoa("È"), "yA==", `btoa("È")`)
    if (error) { return error; }
    error = assert(btoa("É"), "yQ==", `btoa("É")`)
    if (error) { return error; }
    error = assert(btoa("Ê"), "yg==", `btoa("Ê")`)
    if (error) { return error; }
    error = assert(btoa("Ë"), "yw==", `btoa("Ë")`)
    if (error) { return error; }
    error = assert(btoa("Ì"), "zA==", `btoa("Ì")`)
    if (error) { return error; }
    error = assert(btoa("Í"), "zQ==", `btoa("Í")`)
    if (error) { return error; }
    error = assert(btoa("Î"), "zg==", `btoa("Î")`)
    if (error) { return error; }
    error = assert(btoa("Ï"), "zw==", `btoa("Ï")`)
    if (error) { return error; }
    error = assert(btoa("Ð"), "0A==", `btoa("Ð")`)
    if (error) { return error; }
    error = assert(btoa("Ñ"), "0Q==", `btoa("Ñ")`)
    if (error) { return error; }
    error = assert(btoa("Ò"), "0g==", `btoa("Ò")`)
    if (error) { return error; }
    error = assert(btoa("Ó"), "0w==", `btoa("Ó")`)
    if (error) { return error; }
    error = assert(btoa("Ô"), "1A==", `btoa("Ô")`)
    if (error) { return error; }
    error = assert(btoa("Õ"), "1Q==", `btoa("Õ")`)
    if (error) { return error; }
    error = assert(btoa("Ö"), "1g==", `btoa("Ö")`)
    if (error) { return error; }
    error = assert(btoa("×"), "1w==", `btoa("×")`)
    if (error) { return error; }
    error = assert(btoa("Ø"), "2A==", `btoa("Ø")`)
    if (error) { return error; }
    error = assert(btoa("Ù"), "2Q==", `btoa("Ù")`)
    if (error) { return error; }
    error = assert(btoa("Ú"), "2g==", `btoa("Ú")`)
    if (error) { return error; }
    error = assert(btoa("Û"), "2w==", `btoa("Û")`)
    if (error) { return error; }
    error = assert(btoa("Ü"), "3A==", `btoa("Ü")`)
    if (error) { return error; }
    error = assert(btoa("Ý"), "3Q==", `btoa("Ý")`)
    if (error) { return error; }
    error = assert(btoa("Þ"), "3g==", `btoa("Þ")`)
    if (error) { return error; }
    error = assert(btoa("ß"), "3w==", `btoa("ß")`)
    if (error) { return error; }
    error = assert(btoa("à"), "4A==", `btoa("à")`)
    if (error) { return error; }
    error = assert(btoa("á"), "4Q==", `btoa("á")`)
    if (error) { return error; }
    error = assert(btoa("â"), "4g==", `btoa("â")`)
    if (error) { return error; }
    error = assert(btoa("ã"), "4w==", `btoa("ã")`)
    if (error) { return error; }
    error = assert(btoa("ä"), "5A==", `btoa("ä")`)
    if (error) { return error; }
    error = assert(btoa("å"), "5Q==", `btoa("å")`)
    if (error) { return error; }
    error = assert(btoa("æ"), "5g==", `btoa("æ")`)
    if (error) { return error; }
    error = assert(btoa("ç"), "5w==", `btoa("ç")`)
    if (error) { return error; }
    error = assert(btoa("è"), "6A==", `btoa("è")`)
    if (error) { return error; }
    error = assert(btoa("é"), "6Q==", `btoa("é")`)
    if (error) { return error; }
    error = assert(btoa("ê"), "6g==", `btoa("ê")`)
    if (error) { return error; }
    error = assert(btoa("ë"), "6w==", `btoa("ë")`)
    if (error) { return error; }
    error = assert(btoa("ì"), "7A==", `btoa("ì")`)
    if (error) { return error; }
    error = assert(btoa("í"), "7Q==", `btoa("í")`)
    if (error) { return error; }
    error = assert(btoa("î"), "7g==", `btoa("î")`)
    if (error) { return error; }
    error = assert(btoa("ï"), "7w==", `btoa("ï")`)
    if (error) { return error; }
    error = assert(btoa("ð"), "8A==", `btoa("ð")`)
    if (error) { return error; }
    error = assert(btoa("ñ"), "8Q==", `btoa("ñ")`)
    if (error) { return error; }
    error = assert(btoa("ò"), "8g==", `btoa("ò")`)
    if (error) { return error; }
    error = assert(btoa("ó"), "8w==", `btoa("ó")`)
    if (error) { return error; }
    error = assert(btoa("ô"), "9A==", `btoa("ô")`)
    if (error) { return error; }
    error = assert(btoa("õ"), "9Q==", `btoa("õ")`)
    if (error) { return error; }
    error = assert(btoa("ö"), "9g==", `btoa("ö")`)
    if (error) { return error; }
    error = assert(btoa("÷"), "9w==", `btoa("÷")`)
    if (error) { return error; }
    error = assert(btoa("ø"), "+A==", `btoa("ø")`)
    if (error) { return error; }
    error = assert(btoa("ù"), "+Q==", `btoa("ù")`)
    if (error) { return error; }
    error = assert(btoa("ú"), "+g==", `btoa("ú")`)
    if (error) { return error; }
    error = assert(btoa("û"), "+w==", `btoa("û")`)
    if (error) { return error; }
    error = assert(btoa("ü"), "/A==", `btoa("ü")`)
    if (error) { return error; }
    error = assert(btoa("ý"), "/Q==", `btoa("ý")`)
    if (error) { return error; }
    error = assert(btoa("þ"), "/g==", `btoa("þ")`)
    if (error) { return error; }
    error = assert(btoa("ÿ"), "/w==", `btoa("ÿ")`)
    if (error) { return error; }
  }

  // atob
  {
    error = assert(atob(""), "", `atob("")`)
    if (error) { return error; }
    error = assert(atob("abcd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob(" abcd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("abcd "), 'i·\x1D')
    if (error) { return error; }
    error = assertThrows(() => atob(" abcd==="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd=== "))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd ==="))
    if (error) { return error; }
    error = assertThrows(() => atob("a"))
    if (error) { return error; }
    error = assert(atob("ab"), 'i')
    if (error) { return error; }
    error = assert(atob("abc"), 'i·')
    if (error) { return error; }
    error = assertThrows(() => atob("abcde"))
    if (error) { return error; }
    error = assertThrows(() => atob("𐀀"))
    if (error) { return error; }
    error = assertThrows(() => atob("="))
    if (error) { return error; }
    error = assertThrows(() => atob("=="))
    if (error) { return error; }
    error = assertThrows(() => atob("==="))
    if (error) { return error; }
    error = assertThrows(() => atob("===="))
    if (error) { return error; }
    error = assertThrows(() => atob("====="))
    if (error) { return error; }
    error = assertThrows(() => atob("a="))
    if (error) { return error; }
    error = assertThrows(() => atob("a=="))
    if (error) { return error; }
    error = assertThrows(() => atob("a==="))
    if (error) { return error; }
    error = assertThrows(() => atob("a===="))
    if (error) { return error; }
    error = assertThrows(() => atob("a====="))
    if (error) { return error; }
    error = assertThrows(() => atob("ab="))
    if (error) { return error; }
    error = assert(atob("ab=="), 'i')
    if (error) { return error; }
    error = assertThrows(() => atob("ab==="))
    if (error) { return error; }
    error = assertThrows(() => atob("ab===="))
    if (error) { return error; }
    error = assertThrows(() => atob("ab====="))
    if (error) { return error; }
    error = assert(atob("abc="), 'i·')
    if (error) { return error; }
    error = assertThrows(() => atob("abc=="))
    if (error) { return error; }
    error = assertThrows(() => atob("abc==="))
    if (error) { return error; }
    error = assertThrows(() => atob("abc===="))
    if (error) { return error; }
    error = assertThrows(() => atob("abc====="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd=="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd==="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd===="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd====="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcde="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcde=="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcde==="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcde===="))
    if (error) { return error; }
    error = assertThrows(() => atob("abcde====="))
    if (error) { return error; }
    error = assertThrows(() => atob("=a"))
    if (error) { return error; }
    error = assertThrows(() => atob("=a="))
    if (error) { return error; }
    error = assertThrows(() => atob("a=b"))
    if (error) { return error; }
    error = assertThrows(() => atob("a=b="))
    if (error) { return error; }
    error = assertThrows(() => atob("ab=c"))
    if (error) { return error; }
    error = assertThrows(() => atob("ab=c="))
    if (error) { return error; }
    error = assertThrows(() => atob("abc=d"))
    if (error) { return error; }
    error = assertThrows(() => atob("abc=d="))
    if (error) { return error; }
    error = assertThrows(() => atob("ab\u000Bcd"))
    if (error) { return error; }
    error = assertThrows(() => atob("ab\u3000cd"))
    if (error) { return error; }
    error = assertThrows(() => atob("ab\u3001cd"))
    if (error) { return error; }
    error = assert(atob("ab\tcd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("ab\ncd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("ab\fcd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("ab\rcd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("ab cd"), 'i·\x1D')
    if (error) { return error; }
    error = assertThrows(() => atob("ab\u00a0cd"))
    if (error) { return error; }
    error = assert(atob("ab\t\n\f\r cd"), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob(" \t\n\f\r ab\t\n\f\r cd\t\n\f\r "), 'i·\x1D')
    if (error) { return error; }
    error = assert(atob("ab\t\n\f\r =\t\n\f\r =\t\n\f\r "), 'i')
    if (error) { return error; }
    error = assertThrows(() => atob("A"))
    if (error) { return error; }
    error = assert(atob("/A"), 'ü')
    if (error) { return error; }
    error = assert(atob("//A"), 'ÿð')
    if (error) { return error; }
    error = assert(atob("///A"), 'ÿÿÀ')
    if (error) { return error; }
    error = assertThrows(() => atob("////A"))
    if (error) { return error; }
    error = assertThrows(() => atob("/"))
    if (error) { return error; }
    error = assert(atob("A/"), '\x03')
    if (error) { return error; }
    error = assert(atob("AA/"), '\x00\x0F')
    if (error) { return error; }
    error = assertThrows(() => atob("AAAA/"))
    if (error) { return error; }
    error = assert(atob("AAA/"), '\x00\x00?')
    if (error) { return error; }
    error = assertThrows(() => atob("\u0000nonsense"))
    if (error) { return error; }
    error = assertThrows(() => atob("abcd\u0000nonsense"))
    if (error) { return error; }
    error = assert(atob("YQ"), 'a')
    if (error) { return error; }
    error = assert(atob("YR"), 'a')
    if (error) { return error; }
    error = assertThrows(() => atob("~~"))
    if (error) { return error; }
    error = assertThrows(() => atob(".."))
    if (error) { return error; }
    error = assertThrows(() => atob("--"))
    if (error) { return error; }
  }

  return pass('ok')
});

routes.set('/btoa-doubleunderscore', () => {
  let error;
  
  error = assertThrows(() => atob("__"))
  if (error) { return error; }

  return pass('ok');
});
