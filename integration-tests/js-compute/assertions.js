/* global ReadableStream */

// Testing/Assertion functions //

// TODO: Implement ReadableStream getIterator() and [@@asyncIterator]() methods
export async function streamToString(stream) {
    const decoder = new TextDecoder();
    let string = '';
    let reader = stream.getReader()
    // eslint-disable-next-line no-constant-condition
    while (true) {
        const { done, value } = await reader.read();
        if (done) {
            return string;
        }
        string += decoder.decode(value)
    }
}

export function iteratableToStream(iterable) {
    return new ReadableStream({
        async pull(controller) {
            for await (const value of iterable) {
                controller.enqueue(value);
            }
            controller.close();
        }
    });
}

export function pass(message = '') {
    return new Response(message)
}

export function fail(message = '') {
    return new Response(message, { status: 500 })
}
function prettyPrintSymbol (a) {
    if (typeof a === "symbol") {
         return String(a)
    }
    return a
}
export function assert(actual, expected, code) {
    if (!deepEqual(actual, expected)) {
        return fail(`Expected \`${code}\` to equal \`${JSON.stringify(prettyPrintSymbol(expected))}\` - Found \`${JSON.stringify(prettyPrintSymbol(actual))}\``)
    }
}

export async function assertResolves(func) {
    try {
        await func()
    } catch (error) {
        return fail(`Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`)
    }
}

export async function assertRejects(func, errorClass, errorMessage) {
    try {
        await func()
        return fail(`Expected \`${func.toString()}\` to reject - Found it did not reject`)
    } catch (error) {
        if (errorClass) {
            if ((error instanceof errorClass) === false) {
                return fail(`Expected \`${func.toString()}\` to reject instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
            }
        }

        if (errorMessage) {
            if (error.message !== errorMessage) {
                return fail(`Expected \`${func.toString()}\` to reject error message of \`${errorMessage}\` - Found \`${error.message}\``)
            }
        }
    }
}


export function assertThrows(func, errorClass, errorMessage) {
    try {
        func()
        return fail(`Expected \`${func.toString()}\` to throw - Found it did not throw`)
    } catch (error) {
        if (errorClass) {
            if ((error instanceof errorClass) === false) {
                return fail(`Expected \`${func.toString()}\` to throw instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
            }
        }

        if (errorMessage) {
            if (error.message !== errorMessage) {
                return fail(`Expected \`${func.toString()}\` to throw error message of \`${errorMessage}\` - Found \`${error.message}\``)
            }
        }
    }
}

export function assertDoesNotThrow(func) {
    try {
        func()
    } catch (error) {
        return fail(`Expected \`${func.toString()}\` to not throw - Found it did throw: ${error.name}: ${error.message}`)
    }
}

export function deepEqual(a, b) {
    var aKeys;
    var bKeys;
    var typeA;
    var typeB;
    var key;
    var i;

    typeA = typeof a;
    typeB = typeof b;
    if (a === null || typeA !== 'object') {
        if (b === null || typeB !== 'object') {
            return a === b;
        }
        return false;
    }
    // Case: `a` is of type 'object'
    if (typeB !== 'object') {
        return false;
    }
    if (Object.getPrototypeOf(a) !== Object.getPrototypeOf(b)) {
        return false;
    }
    if (a instanceof Date) {
        return a.getTime() === b.getTime();
    }
    if (a instanceof RegExp) {
        return a.source === b.source && a.flags === b.flags;
    }
    if (a instanceof Error) {
        if (a.message !== b.message || a.name !== b.name) {
            return false;
        }
    }

    aKeys = Object.keys(a);
    bKeys = Object.keys(b);
    if (aKeys.length !== bKeys.length) {
        return false;
    }
    aKeys.sort();
    bKeys.sort();

    // Cheap key test:
    for (i = 0; i < aKeys.length; i++) {
        if (aKeys[i] !== bKeys[i]) {
            return false;
        }
    }
    // Possibly expensive deep equality test for each corresponding key:
    for (i = 0; i < aKeys.length; i++) {
        key = aKeys[i];
        if (!deepEqual(a[key], b[key])) {
            return false;
        }
    }
    return typeA === typeB;
}
