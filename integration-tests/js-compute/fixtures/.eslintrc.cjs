module.exports = {
    "env": {
        "es2021": true,
        "node": false,
        "serviceworker": true,
        "worker": true,
    },
    "extends": "eslint:recommended",
    "parserOptions": {
        "ecmaVersion": "latest",
        "sourceType": "module"
    },
    "globals": {
        crypto: true,
        CompressionStream: true,
        Crypto: true,
        CryptoKey: true,
        ObjectStore: true,
        ObjectStoreEntry: true,
        ReadableStream: true,
        SubtleCrypto: true,
        TransformStream: true,
        WritableStream: true,
    }
}
