use std::env::var_os;
use std::path::{Path, PathBuf};
use std::process::Command;

fn main() {
    let out_dir = var_os("OUT_DIR").unwrap();
    let out_dir = Path::new(&out_dir);

    if let Some(prebuilt_engine) = var_os("PREBUILT_ENGINE") {
        let src = std::env::current_dir().unwrap().join(&prebuilt_engine);
        copy_engine(&src, &out_dir);
    } else {
        let src = Path::new("c-dependencies").canonicalize().unwrap();
        build_engine(&src, &out_dir);
    }
}

fn copy_engine(src: &PathBuf, out_dir: &Path) {
    let status = Command::new("cp")
        .current_dir(out_dir)
        .arg(src)
        .arg("js-compute-runtime.wasm")
        .status()
        .unwrap();

    if !status.success() {
        eprintln!("Copying prebuilt engine: {}", status);
        std::process::exit(-1);
    }
}

fn build_engine(src: &PathBuf, out_dir: &Path) {
    println!("cargo:rerun-if-changed={}", src.join("**").display());
    let makefile_path = src.join("js-compute-runtime").join("Makefile");
    let status = Command::new("make")
        .current_dir(out_dir)
        .arg("-j")
        .arg("2")
        .arg("-f")
        .arg(makefile_path)
        .status()
        .unwrap();

    if !status.success() {
        eprintln!("Make failed with status: {}", status);
        std::process::exit(1);
    }
}
