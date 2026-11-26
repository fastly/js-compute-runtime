---
hide_title: false
hide_table_of_contents: false
pagination_next: null
pagination_prev: null
---

# read()

The **`read()`** method of the `ReadableStreamBYOBReader` interface is used to read data into a view on a user-supplied buffer from an associated readable byte stream.
A request for data will be satisfied from the stream's internal queues if there is any data present.
If the stream queues are empty, the request may be supplied as a zero-copy transfer from the underlying byte source.

The method takes as an argument a view on a buffer that supplied data is to be read into, and returns a `Promise`.
The promise fulfills with an object that has properties `value` and `done` when data comes available, or if the stream is cancelled.
If the stream is errored, the promise will be rejected with the relevant error object.

If a chunk of data is supplied, the `value` property will contain a new view.
This will be a view over the same buffer/backing memory (and of the same type) as the original `view` passed to the `read()` method, now populated with the new chunk of data.
Note that once the promise fulfills, the original `view` passed to the method will be detached and no longer usable.
The promise will fulfill with a `value: undefined` if the stream has been cancelled.
In this case the backing memory region of `view` is discarded and not returned to the caller (all previously read data in the view's buffer is lost).

The `done` property indicates whether or not more data is expected.
The value is set `true` if the stream is closed or cancelled, and `false` otherwise.

## Syntax

```js
read(view)
```

### Parameters

- `view`
  - : The view that data is to be read into.

### Return value

A `Promise`, which fulfills/rejects with a result depending on the state of the stream.

The following are possible:

- If a chunk is available and the stream is still active, the promise fulfills with an object of the form:

  ```
  { value: theChunk, done: false }
  ```

  `theChunk` is a view containing the new data.
  This is a view of the same type and over the same backing memory as the `view` passed to the `read()` method.
  The original `view` will be detached and no longer usable.

- If the stream is closed, the promise fulfills with an object of the form (where `theChunk` has the same properties as above):

  ```
  { value: theChunk, done: true }
  ```

- If the stream is cancelled, the promise fulfills with an object of the form:

  ```
  { value: undefined, done: true }
  ```

  In this case the backing memory is discarded.

- If the stream throws an error, the promise rejects with the relevant error.

### Exceptions

- `TypeError`
  - : The source object is not a `ReadableStreamBYOBReader`, the stream has no owner, the view is not an object or has become detached, the view's length is 0, or `ReadableStreamBYOBReader.releaseLock()` is called (when there's is a pending read request).
