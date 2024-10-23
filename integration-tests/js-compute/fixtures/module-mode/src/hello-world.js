import { routes } from './routes';

routes.set('/hello-world', (evt) => {
  console.log(evt.request);
  const res = new Response('hello world');
  return res;
});
