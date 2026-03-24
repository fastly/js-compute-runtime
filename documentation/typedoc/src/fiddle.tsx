import { JSX } from 'typedoc';

let idx = 0;

export function Fiddle({ config }: { config: { src?: { main?: string }, [key: string]: unknown } }): JSX.Element {
    const fallbackCode = config.src?.main ?? '';
    return (
        <>
            <JSX.Raw html={`<script type="application/json+fiddle">${JSON.stringify(config)}</script>`} />
            <noscript>
                <JSX.Raw html={'```js\n' + fallbackCode + '\n```'} />
            </noscript>
        </>
    );
}

export function FiddleClientScript(): JSX.Element {
    return <script async defer src="https://fiddle.fastly.dev/embed.js"></script>;
}
