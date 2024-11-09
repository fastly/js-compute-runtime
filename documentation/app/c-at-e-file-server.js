import parseRange from 'range-parser'

/**
 * Attempt to locate the requested resource from a Fastly Object-Store,
 * If the request is a GET or HEAD request and a resource was found in the Object-Store, this will return a `Response`.
 * If request is not GET or HEAD, or no resource was found in the Object-Store, this will return `null`
 * @param {string} store_name The name of the Fastly Object-Store to search within.
 * @param {Request} request The request to attempt to match against a resource within the Object-Store.
 * @returns {Promise<Response | null>} Returns a `Response` if a resource was found, else returns `null`.
 */
export async function get(store_name, request) {
    const isHeadRequest = request.method === 'HEAD'
    // static files should only respond on HEAD and GET requests
    if (!isHeadRequest && request.method !== 'GET') {
        return null
    }

    // if path ends in / or does not have an extension
    // then append /index.html to the end so we can serve a page
    let path = new URL(request.url).pathname
    if (path.endsWith('/')) {
        path += 'index.html'
    } else if (!path.includes('.')) {
        path += '/index.html'
    }

    const metadataPath = path + '__metadata__'

    let metadata = await (new KVStore(store_name)).get(metadataPath)
    if (metadata == null) {
        return null
    }
    metadata = await metadata.json();
    const responseHeaders = metadata;
    responseHeaders['accept-ranges'] = 'bytes'

    const response = checkPreconditions(request, responseHeaders);
    if (response) {
        return response;
    }
    
    const item = await (new KVStore(store_name)).get(path)
    
    if (item == null) {
        return null
    }
    
    let range = request.headers.get("range");
    if (range == null) {
        return new Response(isHeadRequest ? null : item.body, { status: 200, headers: responseHeaders })
    } else {
        return handleRangeRequest(item, range, responseHeaders, isHeadRequest)
    }
}

async function handleRangeRequest(item, range, headers, isHeadRequest) {
    /**
     * @type {Uint8Array}
     */
    const itemBuffer = new Uint8Array(await item.arrayBuffer())
    const total = itemBuffer.byteLength
    const subranges = parseRange(total, range)

    // -1 signals an unsatisfiable range
    if (subranges == -1) {
        headers['content-range'] = `bytes */${total}`
        return new Response(null, { status: 416, headers })
    }
    // -2 signals a malformed header string
    if (subranges == -2) {
        headers['content-length'] = String(total)
        return new Response(isHeadRequest ? null : itemBuffer, { status: 200, headers })
    }

    if (subranges.length == 1) {
        const { start, end } = subranges[0]
        headers['content-range'] = `bytes ${start}-${end}/${total}`
        headers['content-length'] = String(end - start + 1)

        return new Response(isHeadRequest ? null : itemBuffer.slice(start, end), { status: 206, headers })
    } else {
        const mime = headers['Content-Type']
        headers['Content-Type'] = 'multipart/byteranges; boundary=3d6b6a416f9b5'
        const enc = new TextEncoder();
        const boundaryString = '--3d6b6a416f9b5';
        const type = mime ? enc.encode(`Content-Type: ${mime}\n`) : null
        const results = []
        let bufferLength = 0
        let boundary = enc.encode(`\n${boundaryString}\n`)
        subranges.forEach(function ({ start, end }) {
            {
                bufferLength += boundary.byteLength
                results.push(boundary)
            }
            if (type) {
                results.push(type)
                bufferLength += type.byteLength
            }
            {
                let content_range = enc.encode(`Content-Range: bytes ${start}-${end}/${total}\n\n`)
                bufferLength += content_range.byteLength
                results.push(content_range)
            }
            {
                let content = itemBuffer.slice(start, end)
                bufferLength += content.byteLength
                results.push(content)
            }
        })
        {
            results.push(boundary)
            bufferLength += boundary.byteLength
        }
        const body = concat(results, bufferLength)
        const length = body.byteLength
        headers['content-length'] = String(length)
        return new Response(isHeadRequest ? null : body, { status: 206, headers })
    }
}

function concat(views, length) {
    console.log({length})
    const buf = new Uint8Array(length)
    let offset = 0
    for (const v of views) {
        const uint8view = new Uint8Array(v.buffer, v.byteOffset, v.byteLength)
        buf.set(uint8view, offset)
        offset += uint8view.byteLength
    }

    return buf
}

function checkPreconditions(request, responseHeaders) {
    // https://httpwg.org/specs/rfc9110.html#rfc.section.13.2.2
    // A recipient cache or origin server MUST evaluate the request preconditions defined by this specification in the following order:
    // 1. When recipient is the origin server and If-Match is present, evaluate the If-Match precondition:
    // - if true, continue to step 3
    // - if false, respond 412 (Precondition Failed) unless it can be determined that the state-changing request has already succeeded (see Section 13.1.1)
    let header = request.headers.get("if-match");
    if (typeof header === 'string') {
        console.log("!ifMatch(responseHeaders, header)", !ifMatch(responseHeaders, header));
        if (!ifMatch(responseHeaders, header)) {
            return new Response(null, { status: 412 });
        }
    // } else {
    //     // 2. When recipient is the origin server, If-Match is not present, and If-Unmodified-Since is present, evaluate the If-Unmodified-Since precondition:
    //     // - if true, continue to step 3
    //     // - if false, respond 412 (Precondition Failed) unless it can be determined that the state-changing request has already succeeded (see Section 13.1.4)
    //     header = request.headers.get("if-unmodified-since");
    //     if (typeof header === 'string') {
    //         // console.log("!ifUnmodifiedSince(responseHeaders, header)", !ifUnmodifiedSince(responseHeaders, header));
    //         if (!ifUnmodifiedSince(responseHeaders, header)) {
    //             return new Response(null, { status: 412 });
    //         }
    //     }
    }

    // 3. When If-None-Match is present, evaluate the If-None-Match precondition:
    // - if true, continue to step 5
    // - if false for GET/HEAD, respond 304 (Not Modified)
    // - if false for other methods, respond 412 (Precondition Failed)
    header = request.headers.get("if-none-match");
    const method = request.method;
    const get = "GET";
    const head = "HEAD";
    if (typeof header === 'string') {
        // console.log("!ifNoneMatch(responseHeaders, header)", !ifNoneMatch(responseHeaders, header));
        if (!ifNoneMatch(responseHeaders, header)) {
            if (method === get || method === head) {
                return new Response(null, { status: 304, headers: responseHeaders })
            }
            return new Response(null, { status: 412 });
        }
    } else {
        // 4. When the method is GET or HEAD, If-None-Match is not present, and If-Modified-Since is present, evaluate the If-Modified-Since precondition:
        // - if true, continue to step 5
        // - if false, respond 304 (Not Modified)
        if (method === get || method === head) {
            header = request.headers.get("if-modified-since");
            if (typeof header === 'string') {
                // console.log("!ifModifiedSince(responseHeaders, header)", !ifModifiedSince(responseHeaders, header));
                if (!ifModifiedSince(responseHeaders, header)) {
                    return new Response(null, { status: 304, headers: responseHeaders })
                }
            }
        }
    }

    // 5. When the method is GET and both Range and If-Range are present, evaluate the If-Range precondition:
    // - if true and the Range is applicable to the selected representation, respond 206 (Partial Content)
    // - otherwise, ignore the Range header field and respond 200 (OK)
    if (method === get) {
        if (request.headers.get("range")) {
            header = request.headers.get("if-range");
            if (typeof header === 'string') {
                // console.log("!ifRange(responseHeaders, header)", !ifRange(responseHeaders, header));
                if (!ifRange(responseHeaders, header)) {
                    // We delete the range headers so that the `get` function will return the full body
                    request.headers.delete("range")
                }
            }
        }
    }

    // 6. Otherwise,
    // - perform the requested method and respond according to its success or failure.
    return null;
}

function isWeak(etag) {
    return etag.startsWith("W/\"");
}

function isStrong(etag) {
    return etag.startsWith("\"");
}

function opaqueTag(etag) {
    if (isWeak(etag)) {
        return etag.substring(2);
    }
    return etag;
}
function weakMatch(a, b) {
    // https://httpwg.org/specs/rfc9110.html#entity.tag.comparison
    // two entity tags are equivalent if their opaque-tags match character-by-character, regardless of either or both being tagged as "weak".
    return opaqueTag(a) === opaqueTag(b);
}

function strongMatch(a, b) {
    // https://httpwg.org/specs/rfc9110.html#entity.tag.comparison
    // two entity tags are equivalent if both are not weak and their opaque-tags match character-by-character.
    return isStrong(a) && isStrong(b) && a === b;
}

function splitList(value) {
    return value.split(",").map(s => s.trim());
}

// https://httpwg.org/specs/rfc9110.html#field.if-match
function ifMatch(validationFields, header) {
    if (validationFields.ETag === undefined) {
        return true;
    }

    // 1. If the field value is "*", the condition is true if the origin server has a current representation for the target resource.
    if (header === "*") {
        if (validationFields.ETag !== undefined) {
            return true;
        }
    } else {
        // 2. If the field value is a list of entity tags, the condition is true if any of the listed tags match the entity tag of the selected representation.
        // An origin server MUST use the strong comparison function when comparing entity tags for If-Match (Section 8.8.3.2), 
        // since the client intends this precondition to prevent the method from being applied if there have been any changes to the representation data.
        if (splitList(header).some(etag => {
            console.log(`strongMatch(${etag}, ${validationFields.ETag}) -- ${strongMatch(etag, validationFields.ETag)}`);
            return strongMatch(etag, validationFields.ETag)
        })) {
            return true;
        }
    }

    // 3. Otherwise, the condition is false.
    return false;
}

// https://httpwg.org/specs/rfc9110.html#field.if-none-match
function ifNoneMatch(validationFields, header) {
    // 1. If the field value is "*", the condition is false if the origin server has a current representation for the target resource.
    if (header === "*") {
        if (validationFields.ETag !== undefined) {
            return false;
        }
    } else {
        // 2. If the field value is a list of entity tags, the condition is false if one of the listed tags matches the entity tag of the selected representation.
        // A recipient MUST use the weak comparison function when comparing entity tags for If-None-Match (Section 8.8.3.2), since weak entity tags can be used for cache validation even if there have been changes to the representation data.
        if (splitList(header).some(etag => weakMatch(etag, validationFields.ETag))) {
            return false;
        }
    }

    // 3. Otherwise, the condition is true.
    return true;
}

// https://httpwg.org/specs/rfc9110.html#field.if-modified-since
function ifModifiedSince(validationFields, header) {
    // A recipient MUST ignore the If-Modified-Since header field if the received field value is not a valid HTTP-date, the field value has more than one member, or if the request method is neither GET nor HEAD.
    const date = new Date(header);
    if (isNaN(date)) {
        return true;
    }

    // 1. If the selected representation's last modification date is earlier or equal to the date provided in the field value, the condition is false.
    if (new Date(validationFields["Last-Modified"]) <= date) {
        return false;
    }
    // 2. Otherwise, the condition is true.
    return true;
}

// https://httpwg.org/specs/rfc9110.html#field.if-unmodified-since
// function ifUnmodifiedSince(req, validationFields, header) {
//     // A recipient MUST ignore the If-Unmodified-Since header field if the received field value is not a valid HTTP-date (including when the field value appears to be a list of dates).
//     const date = new Date(header);
//     if (isNaN(date)) {
//         return true;
//     }

//     // 1. If the selected representation's last modification date is earlier than or equal to the date provided in the field value, the condition is true.
//     if (new Date(validationFields["Last-Modified"]) <= date) {
//         return true;
//     }
//     // 2. Otherwise, the condition is false.
//     return false;
// }

// https://httpwg.org/specs/rfc9110.html#field.if-range
function ifRange(validationFields, header) {
    const date = new Date(header);
    console.log(new Date(validationFields["Last-Modified"]), date);
    console.log(new Date(validationFields["Last-Modified"]).getTime() === date.getTime());
    if (!isNaN(date)) {
        // To evaluate a received If-Range header field containing an HTTP-date:
        // 1. If the HTTP-date validator provided is not a strong validator in the sense defined by Section 8.8.2.2, the condition is false.
        // 2. If the HTTP-date validator provided exactly matches the Last-Modified field value for the selected representation, the condition is true.
        if (new Date(validationFields["Last-Modified"]).getTime() === date.getTime()) {
            return true;
        }
        // 3. Otherwise, the condition is false.
        return false;
    } else {
        // To evaluate a received If-Range header field containing an entity-tag:
        // 1. If the entity-tag validator provided exactly matches the ETag field value for the selected representation using the strong comparison function (Section 8.8.3.2), the condition is true.
        if (strongMatch(header, validationFields.ETag)) {
            return true;
        }
        // 2. Otherwise, the condition is false.
        return false;
    }
}
