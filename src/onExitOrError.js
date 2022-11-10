export function onExitOrError(childProcess) {
  return new Promise((resolve, reject) => {
    childProcess.once("exit", resolve);
    childProcess.once("error", reject);
  });
}
