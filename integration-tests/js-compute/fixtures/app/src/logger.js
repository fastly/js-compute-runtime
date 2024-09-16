import { Logger } from 'fastly:logger';
import { routes, isRunningLocally } from './routes';

const earlyLogger = new Logger('AnotherLog');

routes.set('/logger', () => {
  if (isRunningLocally()) {
    let logger = new Logger('ComputeLog');
    logger.log('Hello!');
    earlyLogger.log('World!');
  }

  return new Response();
});
