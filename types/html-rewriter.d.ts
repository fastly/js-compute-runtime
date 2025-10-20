declare module 'fastly:html-rewriter' {
  /**
   * Stream for rewriting HTML content.
   */
  export class HTMLRewritingStream implements TransformStream {
    constructor();
    /**
     * Registers a callback for elements matching the selector.
     * @param selector CSS selector string
     * @param handler Function called with each matching Element
     * @returns The HTMLRewritingStream instance for chaining
     * @throws {Error} If the selector or handler is invalid
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
   * Options for rewriting HTML elements.
   */
  export interface ElementRewriterOptions {
    /**
     * Whether to escape HTML in rewritten content.
     */
    escapeHTML?: boolean;
  }

  /**
   * Represents an HTML element in the rewriting stream.
   */
  export class Element {
    /**
     * Sets an attribute on the element.
     * @param name Attribute name
     * @param value Attribute value
     */
    setAttribute(name: string, value: string): void;
    /**
     * Gets the value of an attribute.
     * @param name Attribute name
     * @returns Attribute value or null if not present
     */
    getAttribute(name: string): string | null;
    /**
     * Removes an attribute from the element.
     * @param name Attribute name
     */
    removeAttribute(name: string): void;
    /**
     * Replaces the element with new content.
     * @param content Replacement HTML or text
     * @param options Optional rewriting options
     */
    replaceWith(content: string, options?: ElementRewriterOptions): void;
    /**
     * Replaces the element's children with new content.
     * @param content Replacement HTML or text
     * @param options Optional rewriting options
     */
    replaceChildren(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content before the element.
     * @param content HTML or text to insert
     * @param options Optional rewriting options
     */
    before(content: string, options?: ElementRewriterOptions): void;
    /**
     * Inserts content after the element.
     * @param content HTML or text to insert
     * @param options Optional rewriting options
     */
    after(content: string, options?: ElementRewriterOptions): void;
    /**
     * Prepends content to the element's children.
     * @param content HTML or text to prepend
     * @param options Optional rewriting options
     */
    prepend(content: string, options?: ElementRewriterOptions): void;
    /**
     * Appends content to the element's children.
     * @param content HTML or text to append
     * @param options Optional rewriting options
     */
    append(content: string, options?: ElementRewriterOptions): void;
  }
}
