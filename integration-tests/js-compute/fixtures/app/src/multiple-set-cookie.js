import { routes } from './routes.js';

routes.set('/multiple-set-cookie/response-init', async () => {
  let h = new Headers();

  h.append(
    'Set-Cookie',
    'test=1; expires=Tue, 06-Dec-2022 12:34:56 GMT; Max-Age=60; Path=/; HttpOnly; Secure, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT',
  );
  h.append('Set-Cookie', 'test2=2');
  h.append('Set-Cookie', 'test3=3');
  h.append('Set-Cookie', 'test4=4');
  h.append('Set-Cookie', 'test5=5');
  h.append('Set-Cookie', 'test6=6');
  h.append('Set-Cookie', 'test7=7');
  h.append('Set-Cookie', 'test8=8');
  h.append('Set-Cookie', 'test9=9');
  h.append('Set-Cookie', 'test10=10');
  h.append('Set-Cookie', 'test11=11');
  let r = new Response('Hello', {
    headers: h,
  });
  return r;
});
routes.set('/multiple-set-cookie/response-direct', async () => {
  let r = new Response('Hello');

  r.headers.append(
    'Set-Cookie',
    'test=1; expires=Tue, 06-Dec-2022 12:34:56 GMT; Max-Age=60; Path=/; HttpOnly; Secure, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT',
  );
  r.headers.append('Set-Cookie', 'test2=2');
  r.headers.append('Set-Cookie', 'test3=3');
  r.headers.append('Set-Cookie', 'test4=4');
  r.headers.append('Set-Cookie', 'test5=5');
  r.headers.append('Set-Cookie', 'test6=6');
  r.headers.append('Set-Cookie', 'test7=7');
  r.headers.append('Set-Cookie', 'test8=8');
  r.headers.append('Set-Cookie', 'test9=9');
  r.headers.append('Set-Cookie', 'test10=10');
  r.headers.append('Set-Cookie', 'test11=11');
  return r;
});
routes.set('/multiple-set-cookie/downstream', async () => {
  let response = await fetch(
    'https://http-me.fastly.dev/append-header=Set-Cookie:test1=1/append-header=Set-Cookie:test2=2/append-header=Set-Cookie:test3=3/anything',
    {
      backend: 'httpme',
    },
  );

  return new Response('', response);
});
