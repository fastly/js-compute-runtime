import { pass, assert } from "./assertions.js";
import { routes } from "./routes.js";

function format(date, count, locale) {
  return `${new Intl.DateTimeFormat(locale).format(date)} ${new Intl.NumberFormat(
      locale,
    ).format(count)}`
}
routes.set('/intl', () => {
  const count = 26254.39
  const date = new Date("2012-05-24")
  let error

  error = assert(format(date, count, "en-US"), '5/24/2012 26,254.39');
  if (error) { return error; }

  error = assert(format(date, count, "de-DE"), '24.5.2012 26.254,39');
  if (error) { return error; }

  return pass('ok')
});
