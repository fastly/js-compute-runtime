function Test(name) {
    this.name = name;
    tests.push(this);
}

/**
 * Enum of possible test statuses.
 *
 * :values:
 *   - ``PASS``
 *   - ``FAIL``
 *   - ``TIMEOUT``
 *   - ``NOTRUN``
 *   - ``PRECONDITION_FAILED``
 */
Test.statuses = {
    PASS: 0,
    FAIL: 1,
    TIMEOUT: 2,
    NOTRUN: 3,
    PRECONDITION_FAILED: 4
};

Test.prototype = merge({}, Test.statuses);

Test.prototype.phases = {
    INITIAL: 0,
    STARTED: 1,
    HAS_RESULT: 2,
    CLEANING: 3,
    COMPLETE: 4
};

Test.prototype.status_formats = {
    0: "Pass",
    1: "Fail",
    2: "Timeout",
    3: "Not Run",
    4: "Optional Feature Unsupported",
}

Test.prototype.step = function (func) {
    try {
        console.log(8888888)
        let res = func.apply(this, Array.prototype.slice.call(arguments, 2));
        console.log(9999999)
        console.log(91, typeof res)
        console.log(92, res)
        console.log({ res })
        return res
    } catch (e) {
        var status = this.FAIL;
        var message = String((typeof e === "object" && e !== null) ? e.message : e);
        var stack = e.stack ? e.stack : null;
        this.set_status(status, message, stack);
        this.phase = this.phases.HAS_RESULT;
        this.done();
    } finally {
        this.current_test = null;
    }
};

Test.prototype.set_status = function (status, message, stack) {
    this.status = status;
    this.message = message;
    this.stack = stack ? stack : null;
};

Test.prototype.done = function () {
    if (this.phase >= this.phases.CLEANING) {
        return;
    }

    if (this.phase <= this.phases.STARTED) {
        this.set_status(this.PASS, null);
    }

    console.log("TEST DONE",
        this.status,
        this.name);

    this.cleanup();
};

/*
 * Invoke all specified cleanup functions. If one or more produce an error,
 * the context is in an unpredictable state, so all further testing should
 * be cancelled.
 */
Test.prototype.cleanup = function () {
    var errors = [];
    function on_error(e) {
        errors.push(e);
    }
    var this_obj = this;
    var results = [];

    this.phase = this.phases.CLEANING;

    all_async(results,
        function (result, done) {
            if (result && typeof result.then === "function") {
                result
                    .then(null, on_error)
                    .then(done);
            } else {
                done();
            }
        },
        function () {
            cleanup_done(this_obj);
        });
};


function cleanup_done(test) {
    test.phase = test.phases.COMPLETE;
    tests.result(test);
}

/**
 * @class
 * Status of the overall harness
 */
function TestsStatus() {
    /** The status code */
    this.status = null;
    /** Message in case of failure */
    this.message = null;
    /** Stack trace in case of an exception. */
    this.stack = null;
}

/**
 * Enum of possible harness statuses.
 *
 * :values:
 *   - ``OK``
 *   - ``ERROR``
 *   - ``TIMEOUT``
 *   - ``PRECONDITION_FAILED``
 */
TestsStatus.statuses = {
    OK: 0,
    ERROR: 1,
    TIMEOUT: 2,
    PRECONDITION_FAILED: 3
};

TestsStatus.prototype = merge({}, TestsStatus.statuses);

TestsStatus.prototype.formats = {
    0: "OK",
    1: "Error",
    2: "Timeout",
    3: "Optional Feature Unsupported"
};

function Tests() {
    this.tests = [];
    this.num_pending = 0;

    this.phases = {
        INITIAL: 0,
        SETUP: 1,
        HAVE_TESTS: 2,
        HAVE_RESULTS: 3,
        COMPLETE: 4
    };
    this.phase = this.phases.INITIAL;

    this.properties = {};

    this.wait_for_finish = false;
    this.processing_callbacks = false;


    this.all_done_callbacks = [];

    this.current_test = null;
    this.asserts_run = [];

    this.status = new TestsStatus();
}

Tests.prototype.setup = function () {
    this.wait_for_finish = true;
};

Tests.prototype.set_status = function (status, message, stack) {
    this.status.status = status;
    this.status.message = message;
    this.status.stack = stack ? stack : null;
};

Tests.prototype.push = function (test) {
    if (this.phase < this.phases.HAVE_TESTS) {
        this.phase = this.phases.HAVE_TESTS;
    }
    this.num_pending++;
    test.index = this.tests.push(test);
};

Tests.prototype.all_done = function () {
    return true
};


Tests.prototype.result = function () {
    this.num_pending--;
    this.notify_result();
};

Tests.prototype.notify_result = function () {
    this.notify_complete();
};

Tests.prototype.notify_complete = function () {
    var this_obj = this;

    forEach(this.all_done_callbacks,
        function (callback) {
            callback(this_obj.tests, this_obj.status, this_obj.asserts_run);
        });
};

/*
 * Utility functions
 */
function assert(expected_true, function_name, error) {
    if (expected_true !== true) {
        throw new Error(`${function_name}: ${error}`);
    }
}

function forEach(array, callback, thisObj) {
    for (var i = 0; i < array.length; i++) {
        if (array.hasOwnProperty(i)) {//eslint-disable-line no-prototype-builtins
            callback.call(thisObj, array[i], i, array);
        }
    }
}

function all_async(values, iter_callback, done_callback) {
    var remaining = values.length;

    if (remaining === 0) {
        done_callback();
    }

    forEach(values,
        function (element) {
            var invoked = false;
            var elDone = function () {
                if (invoked) {
                    return;
                }

                invoked = true;
                remaining -= 1;

                if (remaining === 0) {
                    done_callback();
                }
            };

            iter_callback(element, elDone);
        });
}

function merge(a, b) {
    var rv = {};
    var p;
    for (p in a) {
        rv[p] = a[p];
    }
    for (p in b) {
        rv[p] = b[p];
    }
    return rv;
}

/**
 * Setup globals
 */

var tests = new Tests();

tests.setup({ explicit_done: true });

async function handleRequest() {
    try {
        var test = new Test('meow');
        Promise.resolve().then(function () {
            return new Promise(function () {
                var promise = test.step(async function () { return 'chris'});

                console.log(12, { promise })
                console.log(123, promise instanceof Promise)

                test.step(function () {
                    assert(!!promise, "promise_test",
                        `test body must return a 'thenable' object (received ${promise})`);
                    assert(typeof promise.then === "function", "promise_test",
                        "test body must return a 'thenable' object (received an object with no `then` method)");
                });
            });
        });
        tests.wait_for_finish = false;

        let results = await new Promise((resolve) => {
            tests.all_done_callbacks.push((tests) => resolve(tests));
        });

        return new Response(JSON.stringify(results, null, 2), { headers: { "content-encoding": "application/json" } });
    } catch (e) {
        console.log(`error: ${e}, stack:\n${e.stack}`);
        return new Response(`{
      "error": {
        "message": ${JSON.stringify(e.message)},
        "stack": ${JSON.stringify(e.stack)}
      }
    }`, { status: 500 });
    }
}


addEventListener("fetch", event => event.respondWith(handleRequest(event)));
