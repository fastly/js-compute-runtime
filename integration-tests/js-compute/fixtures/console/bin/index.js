/// <reference path="../../../../../types/index.d.ts" />
addEventListener("fetch", () => {
    console.log("Happy", "birthday", "Aki", "and", "Yuki!");
    let arg;
    arg = new Map();
    arg.set({a:1,b:{c:2}}, 2)
    arg.set(function foo () {}, {})
    console.log('Map:', arg)
    arg = new Set([{a:1,b:{c:2}}, 2, 3])
    console.log('Set:',arg)
    arg = [1, 2, 3, [], 5]
    console.log('Array:',arg)
    // eslint-disable-next-line getter-return
    arg = { a: 1, b: 2, c: 3, d(){}, get f(){}, g: function bar() {} }
    console.log('Object:',arg)
    arg = function () { }
    console.log('function:',arg)
    arg = true
    console.log('boolean:',arg)
    arg = undefined
    console.log('undefined:',arg)
    arg = null
    console.log('null:',arg)
    arg = new Proxy({a:21}, {})
    console.log('proxy:',arg)
    arg = Infinity
    console.log('Infinity:',arg)
    arg = NaN
    console.log('NaN:',arg)
    arg = Symbol('wow')
    console.log('Symbol:',arg)
    arg = new Error('uh oh')
    console.log('Error:',arg)
    arg = 1
    console.log('Number:',arg)
    arg = 1.111
    console.log('Number:',arg)
    arg = 10n
    console.log('BigInt:',arg)
    arg = new Date('2022-08-18T09:57:47.120Z')
    console.log('Date:',arg)
    arg = 'cake'
    console.log('string:',arg)
    arg = /magic/
    console.log('RegExp:',arg)
    arg = Int8Array.from([1,3,4,2,5,6,99999])
    console.log('Int8Array:',arg)
    arg = Uint8Array.from([1,3,4,2,5,6,99999])
    console.log('Uint8Array:', arg)
    arg = Uint8ClampedArray.from([1,3,4,2,5,6,99999])
    console.log('Uint8ClampedArray:', arg)
    arg = Int16Array.from([1,3,4,2,5,6,99999])
    console.log('Int16Array:', arg)
    arg = Uint16Array.from([1,3,4,2,5,6,99999])
    console.log('Uint16Array:', arg)
    arg = Int32Array.from([1,3,4,2,5,6,99999])
    console.log('Int32Array:', arg)
    arg = Uint32Array.from([1,3,4,2,5,6,99999])
    console.log('Uint32Array:', arg)
    arg = Float32Array.from([1,3,4,2,5,6,99999])
    console.log('Float32Array:', arg)
    arg = Float64Array.from([1,3,4,2,5,6,99999])
    console.log('Float64Array:', arg)
    arg = BigInt64Array.from([1n,3n,4n,2n,5n,6n,99999n])
    console.log('BigInt64Array:', arg)
    arg = BigUint64Array.from([1n,3n,4n,2n,5n,6n,99999n])
    console.log('BigUint64Array:', arg)
    arg = new WeakMap
    console.log('WeakMap:', arg)
    arg = new WeakSet
    console.log('WeakSet:', arg)
    arg = new Promise(() => { })
    console.log('Promise:', arg)
    arg = Promise.resolve(9)
    console.log('resolved promise:', arg)
    arg = Promise.reject(new Error('oops'))
    console.log('rejected promise:', arg)
    arg.catch(()=> {})
    arg = new Response('Me? I am the response')
    console.log('Response:', arg)
    arg = new Request('https://www.fastly.com', {body:'I am the body', method: 'POST'})
    console.log('Request:', arg)
    arg = new ReadableStream
    console.log('ReadableStream:', arg)
    arg = new TransformStream
    console.log('TransformStream:', arg)
    arg = new WritableStream
    console.log('WritableStream:', arg)
    arg = new URL('https://www.test.com:123/asdf?some&params=val')
    console.log('URL:', arg)
});
