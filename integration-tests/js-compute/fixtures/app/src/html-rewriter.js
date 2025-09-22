/* eslint-env serviceworker */

import { routes } from './routes.js';
import { HTMLRewritingStream } from 'fastly:html-rewriter';
import {
  assert,
  assertThrows,
  assertRejects,
  strictEqual,
} from './assertions.js';

routes.set('/html-rewriter/set-attribute', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1 class="a-class" id="an-id">Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1 class="a-rewritten-class" id="a-rewritten-id" custom-attr="custom-value">Hello, World!</h1></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.setAttribute('class', 'a-rewritten-class');
      e.setAttribute('id', 'a-rewritten-id');
      e.setAttribute('custom-attr', 'custom-value');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/get-attribute', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1 class="a-class" id="an-id">Hello, World!</h1></body></html>';
  let classAttr, idAttr;
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      classAttr = e.getAttribute('class');
      idAttr = e.getAttribute('id');
    }),
  );
  await new Response(body, { headers: { 'Content-Type': 'text/html' } }).text();
  strictEqual(classAttr, 'a-class');
  strictEqual(idAttr, 'an-id');
});

routes.set('/html-rewriter/remove-attribute', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1 class="a-class" id="an-id" custom-attr="custom-value">Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.removeAttribute('class');
      e.removeAttribute('id');
      e.removeAttribute('custom-attr');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/replace-with', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h2>Goodbye, World!</h2></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.replaceWith('<h2>Goodbye, World!</h2>');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/replace-children', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Goodbye, World!</h1></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.replaceChildren('Goodbye, World!');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/insert', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body>Before - <h1>Prefix - Hello, World! - Suffix</h1> - After</body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.before('Before - ');
      e.prepend('Prefix - ');
      e.append(' - Suffix');
      e.after(' - After');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/complex-selector', async () => {
  const toRewrite =
    "<!DOCTYPE html><html><head><title>Test</title></head><body><div class='a-class'><h1 id='an-id'>Hello, World!</h1></div><div class='a-class'><h1 id='another-id'>Hello again, World!</h1></div></body></html>";
  const expected =
    "<!DOCTYPE html><html><head><title>Test</title></head><body><div class='a-class'><h1 id='an-id' custom-attr=\"custom-value\">Hello, World!</h1></div><div class='a-class'><h1 id='another-id' custom-attr=\"custom-value\">Hello again, World!</h1></div></body></html>";
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('div.a-class > h1[id^="an"]', (e) => {
      e.setAttribute('custom-attr', 'custom-value');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/no-match-rewritten-content', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><div></div></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><div class="a-class"><h1>Hello, World!</h1></div></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream()
      .onElement('div', (e) => {
        e.setAttribute('class', 'a-class');
        e.append('<h1>Hello, World!</h1>');
      })
      .onElement('h1', (e) => {
        // should not be called, as h1 does not exist in original content
        e.setAttribute('custom-attr', 'custom-value');
      }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/multiple-handlers', async () => {
  const toRewrite =
    "<!DOCTYPE html><html><head><title>Test</title></head><body><div class='a-class'><h1 id='an-id'>Hello, World!</h1></div><div class='a-class'><h1 id='another-id'>Hello again, World!</h1></div></body></html>";
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><div class=\'a-class\' custom-attr="custom-value"><h1 id=\'an-id\' another-attr="another-value">Hello, World!</h1></div><div class=\'a-class\' custom-attr="custom-value"><h1 id=\'another-id\' another-attr="another-value">Hello again, World!</h1></div></body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream()
      .onElement('div.a-class', (e) => {
        e.setAttribute('custom-attr', 'custom-value');
      })
      .onElement('h1', (e) => {
        e.setAttribute('another-attr', 'another-value');
      }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/invalid-selector', async () => {
  assertThrows(() => {
    new HTMLRewritingStream().onElement('div..a-class', (e) => {
      e.setAttribute('custom-attr', 'custom-value');
    });
  }, Error);
});

routes.set('/html-rewriter/invalid-handler', async () => {
  assertThrows(() => {
    new HTMLRewritingStream().onElement(
      'div.a-class',
      'this is not a function',
    );
  }, Error);
});

routes.set('/html-rewriter/throw-in-handler', async () => {
  const toRewrite =
    "<!DOCTYPE html><html><head><title>Test</title></head><body><div class='a-class'><h1 id='an-id'>Hello, World!</h1></div></body></html>";
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('div.a-class', (e) => {
      throw new Error('This is an error from the handler');
    }),
  );
  assertRejects(async () => {
    await new Response(body, {
      headers: { 'Content-Type': 'text/html' },
    }).text();
  }, Error);
});

routes.set('/html-rewriter/invalid-html', async () => {
  const toRewrite = 'This is not HTML content';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('div.a-class', (e) => {
      e.setAttribute('custom-attr', 'custom-value');
    }),
  );
  assertRejects(async () => {
    await new Response(body, {
      headers: { 'Content-Type': 'text/plain' },
    }).text();
  }, Error);
});

routes.set('/html-rewriter/insertion-order', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, World!</h1></body></html>';
  const expected =
    '<!DOCTYPE html><html><head><title>Test</title></head><body>First - Before - <h1>Prefix - Other Prefix - Hello, World! - Suffix - Other Suffix</h1> - After - Last</body></html>';
  let body = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('h1', (e) => {
      e.before('First - ');
      e.before('Before - ');
      // The insertion position is maintained, so prepends are inserted in reverse order
      e.prepend('Other Prefix - ');
      e.prepend('Prefix - ');
      e.append(' - Suffix');
      e.append(' - Other Suffix');
      // The insertion position is maintained, so appends are inserted in reverse order
      e.after(' - Last');
      e.after(' - After');
    }),
  );
  let text = await new Response(body, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(text, expected);
});

routes.set('/html-rewriter/escape-html', async () => {
  const toRewrite =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, <em>World!</em></h1></body></html>';
  const expectedNoEscape =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, <strong>Beautiful</strong> <em>World!</em></h1></body></html>';
  const expectedEscape =
    '<!DOCTYPE html><html><head><title>Test</title></head><body><h1>Hello, &lt;strong&gt;Beautiful&lt;/strong&gt; <em>World!</em></h1></body></html>';
  let bodyNoEscape = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('em', (e) => {
      e.before('<strong>Beautiful</strong> ', { escapeHTML: false });
    }),
  );
  let textNoEscape = await new Response(bodyNoEscape, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(textNoEscape, expectedNoEscape);

  let bodyEscape = new Response(toRewrite, {
    headers: { 'Content-Type': 'text/html' },
  }).body.pipeThrough(
    new HTMLRewritingStream().onElement('em', (e) => {
      e.before('<strong>Beautiful</strong> ', { escapeHTML: true });
    }),
  );
  let textEscape = await new Response(bodyEscape, {
    headers: { 'Content-Type': 'text/html' },
  }).text();
  strictEqual(textEscape, expectedEscape);
});
