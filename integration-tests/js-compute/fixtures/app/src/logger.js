import { Logger, configureConsole } from 'fastly:logger';
import { routes, isRunningLocally } from './routes';

configureConsole({ prefixing: false, stderr: true });

const earlyLogger = new Logger('AnotherLog');

routes.set('/logger', () => {
  if (isRunningLocally()) {
    let logger = new Logger('ComputeLog');
    logger.log('Hello!');
    earlyLogger.log('World!');
  }

  console.log('LOG');
  console.error('ERROR');

  return new Response();
});
