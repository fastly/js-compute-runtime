import { strictEqual } from './assertions.js';
import { routes } from './routes.js';

// Googlebot is a well-known verified bot that Fastly's bot detection recognises.
const BOT_UA =
  'Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)';
const HUMAN_UA = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36';

// --- botAnalyzed ---
routes.set('/request/botAnalyzed/bot', (event) => {
  strictEqual(event.request.getBotAnalyzed(), true, 'getBotAnalyzed() is true for bot UA');
});
routes.set('/request/botAnalyzed/human', (event) => {
  strictEqual(typeof event.request.getBotAnalyzed(), 'boolean', 'getBotAnalyzed() returns boolean for human UA');
});

// --- botDetected ---
routes.set('/request/botDetected/bot', (event) => {
  strictEqual(event.request.getBotDetected(), true, 'getBotDetected() is true for bot UA');
});
routes.set('/request/botDetected/human', (event) => {
  strictEqual(event.request.getBotDetected(), false, 'getBotDetected() is false for human UA');
});

// --- botName ---
routes.set('/request/botName/bot', (event) => {
  const result = event.request.getBotName();
  strictEqual(typeof result, 'string', 'getBotName() is a string for bot UA');
  strictEqual(result.length > 0, true, 'getBotName() is non-empty for bot UA');
});
routes.set('/request/botName/human', (event) => {
  strictEqual(event.request.getBotName(), null, 'getBotName() is null for human UA');
});

// --- botCategory ---
routes.set('/request/botCategory/bot', (event) => {
  const result = event.request.getBotCategory();
  strictEqual(typeof result, 'string', 'getBotCategory() is a string for bot UA');
  strictEqual(result.length > 0, true, 'getBotCategory() is non-empty for bot UA');
});
routes.set('/request/botCategory/human', (event) => {
  strictEqual(event.request.getBotCategory(), null, 'getBotCategory() is null for human UA');
});

// --- botCategoryKind ---
routes.set('/request/botCategoryKind/bot', (event) => {
  const result = event.request.getBotCategoryKind();
  strictEqual(typeof result, 'number', 'getBotCategoryKind() is a number for bot UA');
});
routes.set('/request/botCategoryKind/human', (event) => {
  strictEqual(event.request.getBotCategoryKind(), null, 'getBotCategoryKind() is null for human UA');
});

// --- botVerified ---
routes.set('/request/botVerified/bot', (event) => {
  strictEqual(event.request.getBotVerified(), true, 'getBotVerified() is true for Googlebot');
});
routes.set('/request/botVerified/human', (event) => {
  strictEqual(event.request.getBotVerified(), false, 'getBotVerified() is false for human UA');
});
