# @fastly/wasmtime

Provides wasmtime binaries for Node.js applications.

## Usage

```javascript
import wasmtime from '@fastly/wasmtime';

const wasmtimePath = await wasmtime();
// wasmtimePath contains the full path to the wasmtime binary
```

## Supported Platforms

- macOS (x64, ARM64)
- Linux (x64, ARM64)
- Windows (x64)

## License

Apache-2.0
