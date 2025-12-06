export function unknownArgument(cliInput: string) {
  console.error(`error: Found argument '${cliInput}' which wasn't expected, or isn't valid in this context

USAGE:
    js-compute-runtime [FLAGS] [OPTIONS] [ARGS]

For more information try --help`);
  process.exit(1);
}
