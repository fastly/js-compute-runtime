

// These are copied from the default theme's lib.tsx, which is not exported by the typedoc package

import { DeclarationReflection, DefaultThemeRenderContext, i18n, JSX, ProjectReflection, Reflection, SignatureReflection, TypeParameterReflection } from "typedoc";

export function wbr(str: string): (string | JSX.Element)[] {
    const ret: (string | JSX.Element)[] = [];
    const re = /[\s\S]*?(?:[^_\-][_\-](?=[^_\-])|[^A-Z](?=[A-Z][^A-Z]))/g;
    let match: RegExpExecArray | null;
    let i = 0;
    while ((match = re.exec(str))) {
        ret.push(match[0], <wbr />);
        i += match[0].length;
    }
    ret.push(str.slice(i));
    return ret;
}

export function classNames(names: Record<string, boolean | null | undefined>, extraCss?: string): string | undefined {
    const css = Object.keys(names).filter(key => names[key]).concat(extraCss || []).join(' ');
    return css.length ? css : undefined;
}

export function getDisplayName(refl: Reflection): string {
    let version = "";
    if ((refl instanceof DeclarationReflection || refl instanceof ProjectReflection) && refl.packageVersion) {
        version = ` - v${refl.packageVersion}`;
    }

    return `${refl.name}${version}`;
}

export function anchorIcon(context: DefaultThemeRenderContext, anchor: string | undefined) {
    if (!anchor) return <></>;

    return (
        <a href={`#${anchor}`} aria-label={i18n.theme_permalink()} class="tsd-anchor-icon" >
            {context.icons.anchor()}
        </a >
    );
}

export function join<T>(joiner: JSX.Children, list: readonly T[], cb: (x: T) => JSX.Children) {
    const result: JSX.Children = [];

    for (const item of list) {
        if (result.length > 0) {
            result.push(joiner);
        }
        result.push(cb(item));
    }

    return <>{result} </>;
}

export function hasTypeParameters(
    reflection: Reflection,
): reflection is Reflection & { typeParameters: TypeParameterReflection[] } {
    return (
        (reflection instanceof DeclarationReflection || reflection instanceof SignatureReflection) &&
        reflection.typeParameters != null &&
        reflection.typeParameters.length > 0
    );
}