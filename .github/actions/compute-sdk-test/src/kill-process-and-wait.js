// Kill a spawned process (e.g Viceroy), and (a)wait for it
// to be completely killed
const killProcessAndWait = (childProcess) => {
  return new Promise((resolve, reject) => {
    childProcess.on('exit', (code) => {
      resolve('stopped with code ' + code);
    });
    childProcess.kill('SIGINT');
  });
}

export default killProcessAndWait;
