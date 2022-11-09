import { dirname } from 'node:path'
import { spawn } from "node:child_process"
import { mkdir } from "node:fs/promises"
import { createReadStream } from "node:fs"
import { isFile } from './isFile.js'
import { isFileOrDoesNotExist } from './isFileOrDoesNotExist.js'
import { onOpenOrError } from './onOpenOrError.js'
import { onExitOrError } from './onExitOrError.js'
import wizer from '@bytecode-alliance/wizer'

export async function compileApplicationToWasm(input, output, wasmEngine) {
  try {
    if (!(await isFile(input))) {
      console.error(`Error: The \`input\` path does not point to a file: ${input}`)
      process.exit(1)
    }
  } catch (error) {
    console.error(`Error: The \`input\` path points to a non-existant file: ${input}`)
    process.exit(1)
  }

  const inputContents = createReadStream(input, { encoding: 'utf-8' })
  try {
    await onOpenOrError(inputContents)
  } catch (error) {
    console.error("Error: Failed to open the `input` (${input})", error.message)
    process.exit(1)
  }
  // TODO: precompile regexes in the input file
  // let source = fs::read_to_string(input_path)?;
  // let lits = regex::find_literals(&source);

  // let js_file = if !lits.is_empty() {
  //     let mut temp = tempfile::tempfile()?;
  //     writeln!(temp, "{}", &source)?;
  //     regex::precompile(&lits, &mut temp)?;
  //     temp.rewind()?;
  //     temp
  // } else {
  //     fs::File::open(input_path)
  //         .with_context(|| format!("failed to open JS file: {}", input_path.display()))?
  // };
  // TODO: Enhancement - Check that input is valid JavaScript syntax before Wizening
  try {
    if (!(await isFile(wasmEngine))) {
      console.error(`Error: The \`wasmEngine\` path does not point to a file: ${wasmEngine}`)
      process.exit(1)
    }
  } catch (error) {
    console.error(`Error: The \`wasmEngine\` path points to a non-existant file: ${wasmEngine}`)
    process.exit(1)
  }
  try {
    await mkdir(dirname(output), {
      recursive: true
    })
  } catch (error) {
    console.error(`Error: Failed to create the \`output\` (${output}) directory`, error.message)
    process.exit(1)
  }

  try {
    if (!(await isFileOrDoesNotExist(output))) {
      console.error(`Error: The \`output\` path does not point to a file: ${output}`)
      process.exit(1)
    }
  } catch (error) {
    console.error(`Error: Failed to check whether the \`output\` (${output}) is a file path`, error.message)
    process.exit(1)
  }

  const wizerProcess = spawn(wizer, ['--allow-wasi', '--dir=.', '-r _start=wizer.resume', `-o=${output}`, wasmEngine], {
    stdio: [inputContents, process.stdout, process.stderr],
    encoding: "utf-8",
  })
  try {
    const code = await onExitOrError(wizerProcess)
    process.exitCode = code
  } catch (err) {
    console.error(`Error: Failed to compile JavaScript to Wasm`, error.message)
    process.exit(1)
  }
}
