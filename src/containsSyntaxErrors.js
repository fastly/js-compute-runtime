import { spawnSync } from "node:child_process";

// TODO: We should check that the syntax used is supported by our version of SpiderMonkey
export function containsSyntaxErrors(input) {
    let nodeProcess = spawnSync(
        process.execPath,
        [
            "--check",
            input,
        ],
        {
            stdio: [null, null, null],
            shell: true,
            encoding: "utf-8",
        }
    );
    if (nodeProcess.status === 0) {
        return false;
    } else {
        console.error(nodeProcess.stderr.split('\nSyntaxError: Invalid or unexpected token\n')[0] + '\nSyntaxError: Invalid or unexpected token\n')
        return true;
    }
}