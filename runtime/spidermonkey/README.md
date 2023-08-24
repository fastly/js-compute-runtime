Scripts for building SpiderMonkey, compiled to wasm32-wasi as a static library.

## Building from upstream source
The `build-engine.sh` script can be used to build either release or debug builds. It's recommended that this script only be used in CI environments, as it bootstraps a build environment for SpiderMonkey on each invokation.


### Building for release or debug
The script can compile both release and debug builds, with `release` being the default. To compile a debug build, pass `debug` as the first argument:
```sh
sh build-engine.sh debug
```

### Build output
Running the build script will result in three different subdirectories in the current directory:
- `include`, containing the public header files
- `lib`, containing the object files and static libraries needed to statically link SpiderMonkey
- `obj`, the output directory the SpiderMonkey build system creates. This can be ignored or deleted

