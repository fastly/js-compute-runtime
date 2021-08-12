use anyhow::Context;
use std::fs;
use std::mem::ManuallyDrop;
use std::path::{Path, PathBuf};
use std::process::Command;
use structopt::StructOpt;
use toml::Value;
use wizer::Wizer;

#[derive(StructOpt)]
pub struct Options {
    /// The input JS script's file path.
    #[structopt(default_value = "bin/index.js", parse(from_os_str))]
    input: PathBuf,

    /// The file path to write the output Wasm module to.
    #[structopt(default_value = "bin/index.wasm")]
    output: PathBuf,

    /// The JS engine Wasm file path.
    #[structopt(long, parse(from_os_str))]
    engine_wasm: Option<PathBuf>,

    /// Skip creation of deployable bundle.
    #[structopt(long)]
    skip_pkg: bool,

    #[structopt(long = "internal-wizening-mode")]
    wizen: bool,
}

fn main() -> anyhow::Result<()> {
    env_logger::init();
    let options = Options::from_args();

    if !(options.wizen || options.skip_pkg) && !Path::exists(Path::new("fastly.toml")) {
        eprintln!("Error: fastly.toml not found.\n");
        Options::clap().print_help()?;
        std::process::exit(-1);
    }

    if options.wizen {
        log::debug!("Wizerize subprocess start");
        wizen(&options.engine_wasm, &options.output)?;
        log::debug!("Wizerize subprocess complete");
        return Ok(());
    }

    initialize_js(&options.input, &options.output, options.engine_wasm)?;
    if !options.skip_pkg {
        create_bundle(&options.input, &options.output)?;
    }

    Ok(())
}

fn initialize_js(input_path: &PathBuf,
                 output_path: &PathBuf,
                 engine_wasm_path: Option<PathBuf>) -> anyhow::Result<()>
{
    log::debug!("Creating Wasm file for JS input");
    let js_file = fs::File::open(input_path)
    .with_context(|| format!("failed to open JS file: {}", input_path.display()))?;

    let self_path = std::env::args().next().unwrap();
    let mut command = Command::new(self_path);

    if let Some(path) = engine_wasm_path {
        command
        .arg("--engine-wasm")
        .arg(path);
    }

    let status = command
    .arg("--internal-wizening-mode")
    .arg(input_path)
    .arg(output_path)
    .stdin(js_file)
    .status()
    .expect("Failed to invoke wizening mode");

    if !status.success() {
        eprintln!("Wizer failed with status: {}", status);
        std::process::exit(-1);
    }

    Ok(())
}

fn create_bundle(_input_path: &PathBuf, wasm_path: &PathBuf) -> anyhow::Result<()> {
    log::debug!("Creating bundle for Fastly deploy");
    let fastly_toml = fs::read_to_string("fastly.toml")?.parse::<Value>()?;
    let name = fastly_toml["name"].as_str().unwrap();
    let name = sanitize_filename::sanitize(&name).replace(" ", "-");
    log::debug!("sanitized project name: {}", name);

    let tmpdir = ManuallyDrop::new(tempfile::tempdir()?);
    log::debug!("Bundling in temp directory: {}", tmpdir.path().display());
    let build_dir = tmpdir.path().join(&name);
    log::debug!("Build dir: {}", build_dir.display());
    fs::create_dir(&build_dir)?;
    let bin_dir = build_dir.join("bin");
    fs::create_dir(&bin_dir)?;

    fs::copy(wasm_path, bin_dir.join("main.wasm"))?;
    fs::copy("fastly.toml", &build_dir.join("fastly.toml"))?;

    let pkgdir = Path::new(".").canonicalize()?.join("pkg");
    if !Path::exists(&pkgdir) {
        fs::create_dir(&pkgdir)?;
    }

    let pkgpath = pkgdir.join(format!("{}.tar.gz", name));
    log::debug!("Creating tarball: {}", pkgpath.display());

    let status = Command::new("tar")
        .current_dir(tmpdir.path())
        .arg("-czf")
        .arg(pkgpath)
        .arg(name)
        .status()
        .expect("Failed to invoke tar");

    if !status.success() {
        eprintln!("Tar failed with status: {}", status);
        std::process::exit(-1);
    }

    if std::env::var("FASTLY_JS_DEBUG_TEMP_DIR").is_ok() {
        log::debug!(
            "FASTLY_JS_DEBUG_TEMP_DIR is set; will not clean up {}",
            tmpdir.path().display()
        );
    } else {
        log::debug!("Cleaning up {}", tmpdir.path().display());
        drop(ManuallyDrop::into_inner(tmpdir));
    }

    Ok(())
}

fn wizen(engine_path: &Option<PathBuf>, output_path: &Path) -> anyhow::Result<()> {
    let engine_wasm_bytes = match engine_path {
        None => {
            std::include_bytes!(concat!(env!("OUT_DIR"), "/js-compute-runtime.wasm")).to_vec()
        },
        Some(path) => {
            fs::read(path)
                .with_context(|| format!("failed to read JS engine wasm {}", path.display()))?
        }
    };

    let mut wizer = Wizer::new();
    wizer.allow_wasi(true);
    wizer.dir("");
    wizer.func_rename("_start", "wizer.resume");
    let initialized_wasm = wizer.run(&engine_wasm_bytes).with_context(|| {
        format!("failed to initialize JS")
    })?;

    if let Some(parent) = output_path.parent() {
        fs::create_dir_all(parent)
            .with_context(|| format!("failed to create directory: {}", parent.display()))?;
    }
    fs::write(&output_path, initialized_wasm)
        .with_context(|| format!("failed to write {}", output_path.display()))?;
    Ok(())
}
