import * as path from 'path';
import { fileURLToPath } from 'url';
import { Application, Context, Converter, DeclarationReflection, DefaultTheme, DefaultThemeRenderContext, JSX, ReflectionKind, SignatureReflection } from 'typedoc';
import { anchorIcon, classNames, getDisplayName, join, wbr, hasTypeParameters } from './utils.js';
import { Fiddle, FiddleClientScript } from './fiddle.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

function getVersionElement(refl: DeclarationReflection | SignatureReflection): JSX.Element {
    let version = refl.version;
    if (!version && refl instanceof DeclarationReflection) {
        version = (refl as DeclarationReflection).signatures?.[0]?.version;
    }
    return version ? <sup class="tsd-version" style="font-size: 0.6em"> since v{version}</sup> : <></>;
}

class FastlyThemeContext extends DefaultThemeRenderContext {
    constructor(...args: ConstructorParameters<typeof DefaultThemeRenderContext>) {
        super(...args);
        
        // Mostly copied from the default theme's header template, but with added support for rendering a version
        // string on the page title when the @version tag is used on a project/class/method
        this.header = (props) => {
            const opts = this.options.getValue("headings");

            // Don't render on the index page or the class hierarchy page
            // We should probably someday render on the class hierarchy page, but currently breadcrumbs
            // are entirely dependent on the reflection hierarchy, so it doesn't make sense today.
            const renderBreadcrumbs = props.url !== "index.html" && props.url !== "hierarchy.html";

            // Titles are always rendered on DeclarationReflection pages and the modules page for the project.
            // They are also rendered on the readme + document pages if configured to do so by the user.
            let renderTitle: boolean;
            let titleKindString = "";
            if (props.model.isProject()) {
                if (props.url === "index.html" && props.model.readme?.length) {
                    renderTitle = opts.readme;
                } else {
                    renderTitle = true;
                }
            } else if (props.model.isDocument()) {
                renderTitle = opts.document;
            } else {
                renderTitle = true;
                titleKindString = ReflectionKind.singularString(props.model.kind) + " ";
            }
            const version = (props.model instanceof DeclarationReflection || props.model instanceof SignatureReflection)
                ? getVersionElement(props.model)
                : <></>;
            return (
                <div class="tsd-page-title">
                    {renderBreadcrumbs && this.breadcrumbs(props.model)}
                    {renderTitle && (
                        <h1 class={classNames({ deprecated: props.model.isDeprecated() })}>
                            {titleKindString}
                            {getDisplayName(props.model)}
                            {hasTypeParameters(props.model) && (
                                <>
                                    {"<"}
                                    {join(", ", props.model.typeParameters, (item) => item.name)}
                                    {">"}
                                </>
                            )}
                            {version}
                            {this.reflectionFlags(props.model)}
                        </h1>
                    )}
                </div>
            );
        };

        // Mostly copied from the default theme's member template, but with added support for rendering a version string 
        this.member = (props) => {
            const anchor = this.getAnchor(props);

            this.page.pageHeadings.push({
                link: `#${anchor}`,
                text: getDisplayName(props),
                kind: props.kind,
                classes: this.getReflectionClasses(props),
                icon: this.theme.getReflectionIcon(props),
            });

            // With the default url derivation, we'll never hit this case as documents are always placed into their
            // own pages. Handle it here in case someone creates a custom url scheme which embeds guides within the page.
            if (props.isDocument()) {
                return (
                    <section class={classNames({ "tsd-panel": true, "tsd-member": true }, this.getReflectionClasses(props))}>
                        {!!props.name && (
                            <h3 class="tsd-anchor-link" id={anchor}>
                                {this.reflectionFlags(props)}
                                <span class={classNames({ deprecated: props.isDeprecated() })}>{wbr(props.name)}</span>
                                {anchorIcon(this, anchor)}
                            </h3>
                        )}
                        <div class="tsd-comment tsd-typography">
                            <JSX.Raw html={this.markdown(props.content)} />
                        </div>
                    </section>
                );
            }

            const version = getVersionElement(props);
            return (
                <section class={classNames({ "tsd-panel": true, "tsd-member": true }, this.getReflectionClasses(props))}>
                    {!!props.name && (
                        <h3 class="tsd-anchor-link" id={anchor}>
                            {this.reflectionFlags(props)}
                            <span class={classNames({ deprecated: props.isDeprecated() })}>{wbr(props.name)} {version}</span>
                            {anchorIcon(this, anchor)}
                        </h3>
                    )}
                    {props.signatures
                        ? this.memberSignatures(props)
                        : props.hasGetterOrSetter()
                            ? this.memberGetterSetter(props)
                            : this.memberDeclaration(props)}

                    {props.groups?.map((item) =>
                        item.children.map((item) => !this.router.hasOwnDocument(item) && this.member(item))
                    )}
                </section>
            );
        };

        // We don't have any JS source to link to, so disable the member sources section
        this.memberSources = () => <></>;

        // Render @fiddle block tags as Fiddle components; pass all other tags to the default renderer
        const superCommentTags = this.commentTags.bind(this);
        this.commentTags = (props) => {
            if (!props.comment?.blockTags) {
                return superCommentTags(props);
            }
            const fiddleTags = props.comment?.blockTags.filter(tag => tag.tag === '@fiddle') || [];
            props.comment.blockTags = props.comment?.blockTags.filter(tag => tag.tag !== '@fiddle');
            return <>
                {superCommentTags(props)}
                {fiddleTags.map(tag => { 
                    const json = tag.content.map(p => p.text).join('');
                    try {
                        const config = JSON.parse(json);
                        return <Fiddle config={config} />;
                    } catch {
                        return <></>;
                    }
                })}
            </>;
        };
    }
}

class FastlyTheme extends DefaultTheme {
    ContextClass = FastlyThemeContext;
}

declare module 'typedoc' {
    interface SignatureReflection {
        version?: string;
    }
    interface DeclarationReflection {
        version?: string;
    }
}

export function load(app: Application) {
    app.renderer.hooks.on('body.end', () => <>
        <FiddleClientScript />
    </>);

    // Pair each @fiddle tag with the preceding @example tag and merge the code
    // into the fiddle JSON as src.main, so the @example remains usable by other tools.
    app.converter.on(Converter.EVENT_RESOLVE_BEGIN, (context: Context) => {
        for (const reflection of Object.values(context.project.reflections)) {
            const comment = reflection.comment;
            if (!comment?.blockTags) continue;
            const tags = comment.blockTags;
            for (let i = 0; i < tags.length; i++) {
                if (tags[i].tag !== '@fiddle') continue;
                // Find the nearest preceding @example
                const exampleIndex = tags.slice(0, i).reverse().findIndex(t => t.tag === '@example');
                if (exampleIndex === -1) continue;
                const exampleTag = tags[i - 1 - exampleIndex];
                // Extract code from the fenced block in the example content
                const exampleText = exampleTag.content.map(p => p.text).join('');
                const codeMatch = exampleText.match(/```(?:js|javascript|ts|typescript)?\n([\s\S]*?)```/);
                if (!codeMatch) continue;
                const code = codeMatch[1];
                // Merge into fiddle JSON
                const fiddleRaw = tags[i].content.map(p => p.text).join('');
                const jsonStart = fiddleRaw.indexOf('{');
                if (jsonStart === -1) continue;
                const fiddleText = fiddleRaw.slice(jsonStart).trim();
                try {
                    const fiddleJson = JSON.parse(fiddleText);
                    // Expand 'request' shorthand into a full 'requests' array
                    if (typeof fiddleJson.request === 'string') {
                        fiddleJson.requests = [{ method: 'GET', path: fiddleJson.request }];
                        delete fiddleJson.request;
                    }
                    // Default src.deps if not provided
                    const defaultDeps = '{\n  "@fastly/js-compute": "^3.37.0"\n}';
                    const src = { deps: defaultDeps, ...fiddleJson.src, main: code };
                    const defaultFiddleHost = 'https://fiddle.fastly.dev';
                    const merged = { type: 'javascript', fiddleHost: defaultFiddleHost, ...fiddleJson, src };
                    tags[i].content = [{ kind: 'text', text: JSON.stringify(merged, null, 2) }];
                    tags.splice(i - 1 - exampleIndex, 1); // Remove the @example tag since its content is now in the @fiddle
                } catch {
                    // malformed JSON, leave as-is
                }
            }
        }
    });
    // Extract the version from the @version tag and store it on the reflection for later use in the theme
    const extractVersion = (_context: Context, reflection: SignatureReflection | DeclarationReflection) => {
        const comment = reflection.comment;
        if (!comment) return;
        const versionTag = comment.blockTags?.find(tag => tag.tag === '@version');
        if (!versionTag) return;
        const versionText = versionTag.content.map(part => part.text).join('').trim();
        comment.blockTags = comment.blockTags.filter(tag => tag.tag !== '@version');

        if (!comment.summary) {
            comment.summary = [];
        }
        reflection.version = versionText;
    }
    app.converter.on(Converter.EVENT_CREATE_SIGNATURE, extractVersion);
    app.converter.on(Converter.EVENT_CREATE_DECLARATION, extractVersion);

    // Remove signatures that originate from node_modules (e.g. @types/node overloads
    // that get merged with our own declarations)
    app.converter.on(Converter.EVENT_RESOLVE_BEGIN, (context: Context) => {
        for (const reflection of Object.values(context.project.reflections)) {
            if (!(reflection instanceof DeclarationReflection) || !reflection.signatures) continue;
            const filtered = reflection.signatures.filter(sig =>
                sig.sources?.every(src => !src.fullFileName?.includes('node_modules')) ?? true
            );
            if (filtered.length !== reflection.signatures.length) {
                reflection.signatures = filtered.length > 0 ? filtered : undefined;
            }
        }
    });

    app.renderer.defineTheme("fastly", FastlyTheme);
}
