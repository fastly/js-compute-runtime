import { pass, assert } from "./assertions.js";
import { routes } from "./routes.js";
import { sdkVersion } from "fastly:experimental";

const starlingmonkey = sdkVersion.includes('starlingmonkey');

routes.set("/oom", async () => {
    if (starlingmonkey) {
        return pass()
    }
    try {
        const objects = [];
        for (let i = 0; i < 1000; i++) {
            objects.push(new BigUint64Array(1_000_000));
        }
    }
    return fail('expected out of memory error')
});
