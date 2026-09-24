---
hide_title: false
hide_table_of_contents: false
pagination_next: null
pagination_prev: null
---

# respondWithNewView()

The **`respondWithNewView()`** method of the `ReadableStreamBYOBRequest` interface specifies a new view that the consumer of the associated readable byte stream should write to instead of `ReadableStreamBYOBRequest.view`.

The new view must be a `TypedArray` or a `DataView` that provides a view onto the same backing memory region as `ReadableStreamBYOBRequest.view`.
After this method is called, the view that was passed into the method will be transferred and no longer modifiable.

The method is intended for use cases where an underlying byte source needs to transfer a `byobRequest.view` internally before finishing its response.
For example, the source may transfer the BYOB view to a separate worker thread, and wait for the worker to transfer it back once it has been filled.

## Syntax

```js
respondWithNewView(view)
```

### Parameters

- `view`

  - : A `TypedArray` or a `DataView` that the consumer of the associated readable byte stream should write to instead of `ReadableStreamBYOBRequest.view`.

    This must be a view onto the same backing memory region as `ReadableStreamBYOBRequest.view` and occupy the same or less memory.
    Specifically, it must be either the view's buffer or a transferred version, must have the same `byteOffset`, and a `byteLength` (number of bytes written) that is less than or equal to that of the view.

### Return value

`undefined`

### Exceptions

- `TypeError`

  - : Thrown if the source object is not a `ReadableStreamBYOBRequest`, or there is no associated controller, or the associated internal array buffer is non-existent or detached.
    It may also be thrown if the `view` is zero-length when there is an active reader, or non-zero when called on a closed stream.

- `RangeError`
  - : Thrown if the new `view` does not match the backing memory region of `ReadableStreamBYOBRequest.view`.
    For example, it is not the same buffer (or a transferred version), has a different `byteOffset`, or is larger than the memory available to the backing view.
