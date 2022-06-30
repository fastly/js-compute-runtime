import childProcess from 'child_process';

import isPortReachable from 'is-port-reachable';
import chalk from 'chalk';

import killProcessAndWait from './kill-process-and-wait.js';

async function timeout(millis, message) {
  if (message === undefined) {
    message = `timeout reached after ${millis} milliseconds`;
  }

  return new Promise((_resolve, reject) => setTimeout(() => reject(message), millis));
}

async function viceroyReady(viceroyHostname, viceroyPort) {
  let isViceroyReady = false;
  while (!isViceroyReady) {
    isViceroyReady = await isPortReachable(viceroyPort, {host: viceroyHostname});
  }
}

// Viceroy - JS Class to use Node's Child Process and Spawn and kill Viceroy
// processes, using the Viceroy CLI
class Viceroy {
  constructor() {
    this.viceroyProcess = null;
    this.logs = '';
  }

  async spawn(wasmPath, options) {
    if (this.viceroyProcess) {
      throw new Error('Viceroy process already spawned')
    }

    let viceroyArgs = [wasmPath];

    // Move our options into CLI arguments
    for (const option in options) {
      viceroyArgs = viceroyArgs.concat([`--${option}`, options[option]]);
    }
    // Add support for logging stdout and stderr, so viceroy won't error when we get logs
    viceroyArgs.push('--log-stdout');
    viceroyArgs.push('--log-stderr');

    let viceroyHostname = '127.0.0.1';
    let viceroyPort = '7878';
    if (options.addr) {
      let optionsAddrUrl = new URL(`http://${options.addr}`);
      viceroyHostname = optionsAddrUrl.hostname;
      viceroyPort = optionsAddrUrl.port;
    }

    this.viceroyProcess = childProcess.spawn(
      'viceroy',
      viceroyArgs,
      {
        cwd: process.cwd()
      }
    );
    this.viceroyProcess.stdout.on('data', (data) => {
      console.log(`${chalk.bold('[Viceroy]')} ${data.toString()}`);
      this.logs = `${this.logs}\n${data}`;
    });
    this.viceroyProcess.stderr.on('data', (data) => {
      console.error(`${chalk.bold('[Viceroy]')} ${data.toString()}`);
      this.logs = `${this.logs}\n${data}`;
    });

    // Wait for 10 seconds before deciding that viceroy has failed to start
    const VICEROY_READY_TIMEOUT = 10000;
    try {
      await Promise.race([
        viceroyReady(viceroyHostname, viceroyPort),
        timeout(VICEROY_READY_TIMEOUT, "Viceroy failed to start"),
      ]);
    } catch (err) {
      console.error(err);
      console.dir(err);
      await killProcessAndWait(this.viceroyProcess);
      process.exit(1);
    }
  }

  async kill() {
    if (!this.viceroyProcess) {
      throw new Error('No viceroy process spawned');
    }

    return await killProcessAndWait(this.viceroyProcess);
  }
}

export default Viceroy;
