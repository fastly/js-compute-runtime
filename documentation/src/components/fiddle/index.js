import React, { useState, useEffect } from "react";

function hexString(buffer) {
    return [...(new Uint8Array(buffer))].map(val => val.toString(16).padStart(2, '0')).join('');
}

function compat(data) {
    let deps = {}
    if (data.src.deps) {
        deps = JSON.parse(data.src.deps)
    }
    deps["@fastly/js-compute"] = "^3";
    data.src.deps = JSON.stringify(deps);
    if (!('requests' in data) || ('reqUrl' in data)) {
        const {
            reqUrl, reqMethod, reqHeaders, reqBody, purgeFirst, enableCluster, enableShield, useH2, requests, ...retainedProps
        } = data;
        purgeFirst; // Discard this
        return {
            ...retainedProps,
            requests: (requests || []).concat({
                path: reqUrl,
                method: reqMethod,
                headers: reqHeaders,
                body: reqBody,
                enableCluster,
                enableShield,
                enableWAF: false,
                connType: useH2 ? 'h2' : 'h1'
            })
        };
    } else if ('requests' in data && data.requests[0] && 'useH2' in data.requests[0]) {
        const {
            requests, ...retainedProps
        } = data;
        return {
            ...retainedProps,
            requests: requests.map(({
                useH2, ...retainedProps2
            }) => ({
                ...retainedProps2,
                connType: useH2 ? 'h2' : 'h1'
            }))
        };
    } else {
        return data;
    }
}

async function getFrameUrl(config) {
    let data = compat(config);

    const hashInp = JSON.stringify(data);
    const hashInpBuf = (new TextEncoder()).encode(hashInp);
    const hash = await globalThis.crypto.subtle.digest('SHA-256', hashInpBuf);
    const immutID = 'x' + hexString(hash).substr(0, 7);
    const fiddleHost = 'https://fiddle.fastly.dev';
    const fiddleCheck = await fetch(fiddleHost + '/fiddle/' + immutID, {
        headers: {
            Accept: 'application/json'
        }
    });
    if (fiddleCheck.ok) {
        return fiddleHost + '/fiddle/' + immutID + '/embedded';
    }

    const createRes = await fetch(fiddleHost + '/fiddle', {
        method: 'POST',
        body: JSON.stringify({
            ...data,
            immutable: true
        }),
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        }
    });
    const createData = await createRes.json();
    if (createData.fiddle) {
        return fiddleHost + '/fiddle/' + createData.fiddle.id + '/embedded';
    }
    return false;
}

let idx = 0;

export function Fiddle({config, children}) {
    const [frameUrl, setFrameUrl] = useState(null);

    async function fetchFrameUrl(config) {
        setFrameUrl(await getFrameUrl(config));
    }
    
    useEffect(() => {
        fetchFrameUrl(config);
    }, [config]);
    
    if (!frameUrl) {
        return <>{children}</>
    }
    
    const embedID = 'embed-' + idx;
    idx += 1;
    const queryParams = [`embedID=${embedID}`];
    const src = `${frameUrl}?${queryParams.join('&')}`;
    return <div id={embedID} className="fiddle" data-state='loaded'>
        <iframe aria-hidden className='fiddle-frame' src={src}></iframe>
        <div aria-hidden className='fiddle-loader'>Loading example code...</div>
    </div>;
}
