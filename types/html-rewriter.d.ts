declare module 'fastly:html-rewriter' {
  /**
   * Lets you rewrite HTML by registering callbacks on CSS selectors. When an element matching the
   * selector is encountered, the rewriter calls your callback, which can manipulate the element's
   * attributes and add or remove content from the immediate context.
   *
   * `HTMLRewritingStream` implements `TransformStream`, so you can pipe an HTML response body
   * through it using `pipeThrough`.
   *
   * @example
   * ```js
   * import { HTMLRewritingStream } from 'fastly:html-rewriter';
   *
   * async function handleRequest(event) {
   *   let transformer = new HTMLRewritingStream()
   *     .onElement("h1", e => e.prepend("Header: "))
   *     .onElement("div", e => e.setAttribute("special-attribute", "top-secret"));
   *   let body = (await fetch("https://example.com/")).body.pipeThrough(transformer);
   *
   *   return new Response(body, {
   *     status: 200,
   *     headers: new Headers({
   *       "content-type": "text/html; charset=utf-8",
   *     })
   *   });
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));
   * ```
   *
   * @version 3.35.0
   */
  export class HTMLRewritingStream implements TransformStream {
    constructor();
    /**
     * Registers a callback for elements matching the given CSS selector. The callback is called
     * once for each matching element. Elements added by handlers will not be processed by other
     * handlers.
     *
     * Supported CSS selectors:
     *
     * | Pattern | Description |
     * |---|---|
     * | `*` | Any element |
     * | `E` | All elements of type `E` |
     * | `E F` | `F` elements inside `E` elements |
     * | `E > F` | `F` elements directly inside `E` elements |
     * | `E:nth-child(n)` | The n-th child of type `E` |
     * | `E:first-child` | First child of type `E` |
     * | `E:nth-of-type(n)` | The n-th sibling of type `E` |
     * | `E:first-of-type` | First sibling of type `E` |
     * | `E:not(s)` | Type `E` elements not matching selector `s` |
     * | `E.myclass` | Type `E` elements with class `"myclass"` |
     * | `E#myid` | Type `E` elements with ID `"myid"` |
     * | `E[attr]` | Type `E` elements with attribute `attr` |
     * | `E[attr="val"]` | Type `E` elements where `attr` is `"val"` |
     * | `E[attr="val" i]` | Case-insensitive match |
     * | `E[attr="val" s]` | Case-sensitive match |
     * | `E[attr~="val"]` | `attr` contains `"val"` in a space-separated list |
     * | <code>E[attr\|="val"]</code> | `attr` is hyphen-separated and starts with `"val"` |
     * | `E[attr^="val"]` | `attr` starts with `"val"` |
     * | `E[attr$="val"]` | `attr` ends with `"val"` |
     * | `E[attr*="val"]` | `attr` contains `"val"` |
     *
     * @param selector CSS selector string
     * @param handler Function called with each matching Element
     * @returns The HTMLRewritingStream instance for chaining
     * @throws `Error` If the provided selector is not a valid CSS selector or 
     * if the handler is not a function.
     */
    onElement(selector: string, handler: (element: Element) => void): this;

    /**
     * The writable stream to which HTML content should be written.
     */
    writable: WritableStream;
    /**
     * The readable stream from which transformed HTML content can be read.
     */
    readable: ReadableStream;
  }

  /**
   * Options for content insertion and replacement methods on {@link Element}.
   *
   * @version 3.35.0
   */
  export interface ElementRewriterOptions {
    /**
     * If `true`, any HTML markup in the content will be escaped so it is safe to insert as text.
     */
    escapeHTML?: boolean;
  }

  /**
   * Represents an HTML element encountered during rewriting. Passed to the callback registered
   * via {@link HTMLRewritingStream.onElement}.
   *
   * @version 3.35.0
   */
  export class Element {
    /**
     * The CSS selector that matched this element.
     */
    readonly selector: string;
    /**
     * The tag name of this element.
     */
    readonly tag: string;

    /**
     * Sets an attribute on the element. If the attribute already exists, its value is updated.
     * @param name Attribute name
     * @param value Attribute value
     */
    setAttribute(name: string, value: string): void;
    /**
     * Gets the value of an attribute.
     * @param name Attribute name
     * @returns The attribute value, or `null` if the attribute is not present.
     */
    getAttribute(name: string): string | null;
    /**
     * Removes an attribute from the element.
     * @param name Attribute name
     */
    removeAttribute(name: string): void;
    /**
     * Replaces the element and its children with new content.
     * @param content Replacement HTML or text
     * @param options Optional rewriting options
     */
    replaceWith(content: string, options?: ElementRewriterOptions): void;
    /**
     * Replaces the element's children with new content, keeping the element's tags.
     * @param content Replacement HTML or text
     * @param options Optional rewriting options
     */
    replaceChildren(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content before the element's opening tag.
     * @param content HTML or text to insert
     * @param options Optional rewriting options
     */
    before(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content after the element's closing tag.
     * @param content HTML or text to insert
     * @param options Optional rewriting options
     */
    after(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content at the beginning of the element's children.
     * @param content HTML or text to prepend
     * @param options Optional rewriting options
     */
    prepend(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content at the end of the element's children.
     * @param content HTML or text to append
     * @param options Optional rewriting options
     */
    append(content: string, options?: ElementRewriterOptions): void;
  }
}
