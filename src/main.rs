use anyhow::Context;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;
use tempfile::NamedTempFile;
use std::io::Write;
use structopt::StructOpt;
use wizer::Wizer;

mod regex;

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

    #[structopt(long = "internal-wizening-mode")]
    wizen: bool,
}

fn main() -> anyhow::Result<()> {
    env_logger::init();
    let options = Options::from_args();

    if options.wizen {
        log::debug!("Wizerize subprocess start");
        wizen(&options.engine_wasm, &options.output)?;
        log::debug!("Wizerize subprocess complete");
        return Ok(());
    }

    initialize_js(&options.input, &options.output, options.engine_wasm)?;

    Ok(())
}

fn initialize_js(input_path: &PathBuf,
                 output_path: &PathBuf,
                 engine_wasm_path: Option<PathBuf>) -> anyhow::Result<()>
{
    log::debug!("Creating Wasm file for JS input");

    let self_path = std::env::args().next().unwrap();
    let mut command = Command::new(self_path);

    if let Some(path) = engine_wasm_path {
        command
        .arg("--engine-wasm")
        .arg(path);
    }

    command.arg("--internal-wizening-mode");

    let source = fs::read_to_string(input_path)?;
    let lits = regex::find_literals(&source);
    let mut temp = NamedTempFile::new()?;
    if !lits.is_empty() {

        write!(temp, "{}", &source)?;
        write!(temp, ";\nfunction precompileRegex(r) {{ r.exec('a'); r.exec('\\u1000'); }};\n")?;
        for regex in lits.into_iter() {
            write!(temp, "precompileRegex(/{}/);\n", regex)?;
        }

        let js_file = fs::File::open(temp.path())
        .with_context(|| format!("failed to open JS file: {}", input_path.display()))?;
        command.arg(temp.path()).stdin(js_file);
    } else {
        let js_file = fs::File::open(input_path)
        .with_context(|| format!("failed to open JS file: {}", input_path.display()))?;
        command.arg(input_path).stdin(js_file);
    }

    let status = command
            .arg(output_path)
            .status()
            .expect("Failed to invoke wizening mode");

    if !status.success() {
        eprintln!("Wizer failed with status: {}", status);
        std::process::exit(-1);
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
    wizer.allow_wasi(true)?;
    wizer.dir(".");
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
