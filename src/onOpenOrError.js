export function onOpenOrError(stream) {
  return new Promise((resolve, reject) => {
    stream.once('open', resolve);
    stream.once('error', reject);
  });
}
