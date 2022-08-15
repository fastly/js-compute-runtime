addEventListener("fetch", () => {
    console.log("Happy", "birthday", "Aki", "and", "Yuki!");
    let arg;
    arg = new Map();
    arg.set(1, 2)
    arg.set(function foo () {}, {})
    console.log('Map', arg)
    arg = new Set([1, 2, 3])
    console.log('Set',arg)
    arg = [1, 2, 3, 4, 5]
    console.log('Array',arg)
    arg = { a: 1, b: 2, c: 3 }
    console.log('Object',arg)
    arg = function () { }
    console.log('function',arg)
    arg = true
    console.log('boolean',arg)
    arg = undefined
    console.log('undefined',arg)
    arg = null
    console.log('null',arg)
    // arg = new Proxy({}, {})
    // console.log('proxy',arg)
    arg = Infinity
    console.log('Infinity',arg)
    arg = NaN
    console.log('NaN',arg)
    arg = Symbol('wow')
    console.log('Symbol',arg)
    arg = new Error('uh oh')
    console.log('Error',arg)
    arg = 1
    console.log('Number',arg)
    arg = 1.111
    console.log('Number',arg)
    arg = 10n
    console.log('BigInt',arg)
    arg = new Date
    console.log('Date',arg)
    arg = 'cake'
    console.log('string',arg)
    arg = /magic/
    console.log('RegExp',arg)
    // arg = new Int8Array
    // console.log('Int8Array',arg)
    // arg = new Uint8Array
    // console.log('Uint8Array', arg)
    // arg = new Uint8ClampedArray
    // console.log('Uint8ClampedArray', arg)
    // arg = new Int16Array
    // console.log('Int16Array', arg)
    // arg = new Uint16Array
    // console.log('Uint16Array', arg)
    // arg = new Int32Array
    // console.log('Int32Array', arg)
    // arg = new Uint32Array
    // console.log('Uint32Array', arg)
    // arg = new Float32Array
    // console.log('Float32Array', arg)
    // arg = new Float64Array
    // console.log('Float64Array', arg)
    // arg = new BigInt64Array
    // console.log('BigInt64Array', arg)
    // arg = new BigUint64Array
    // console.log('BigUint64Array', arg)
    arg = new WeakMap
    console.log('WeakMap', arg)
    // arg = new WeakSet
    // console.log('WeakSet', arg)
    // arg = new Promise((resolve, reject) => { })
    // console.log('Promise', arg)
    // arg = Promise.resolve(9)
    // console.log('resolved promise', arg)
    // // arg = Promise.reject(9)
    // // console.log('rejected promise', arg)
    // arg = new Response('')
    // console.log('Response', arg)
    // arg = new Request('')
    // console.log('Request', arg)
});
