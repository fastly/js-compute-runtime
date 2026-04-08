declare module 'fastly:image-optimizer' {
  /**
   * A color, either a 3 or 6 character hexadecimal string, or an object with RGB(A) components.
   *
   * When specified as an object:
   * - `r`, `g`, `b` are integers (0-255)
   * - `a` is an optional alpha value (0.0-1.0)
   */
  type Color =
    | string
    | {
        r: number;
        g: number;
        b: number;
        a?: number;
      };
  /**
   * A percentage, expressed as a string containing a number suffixed with a percent sign
   * (e.g. `'50%'`).
   */
  type Percentage = string;
  /**
   * The size of a region, specified as either absolute dimensions or a ratio.
   * Exactly one of `absolute` or `ratio` must be provided.
   */
  interface Size {
    /**
     * Absolute dimensions, with width and height as integer pixel values or percentage strings.
     */
    absolute?: {
      width: number | Percentage;
      height: number | Percentage;
    };
    /**
     * Aspect ratio, with width and height as numbers.
     */
    ratio?: {
      width: number;
      height: number;
    };
  }
  /**
   * The position of a region. Provide exactly one of `x`/`offsetX` and exactly one of
   * `y`/`offsetY`.
   */
  interface Position {
    /** Absolute x position as integer pixels or percentage string. */
    x?: number | Percentage;
    /** X offset as a percentage number. */
    offsetX?: number;
    /** Absolute y position as integer pixels or percentage string. */
    y?: number | Percentage;
    /** Y offset as a percentage number. */
    offsetY?: number;
  }

  /**
   * Pixel or percentage values for each side of an image.
   */
  interface Sides {
    top: number | Percentage;
    bottom: number | Percentage;
    left: number | Percentage;
    right: number | Percentage;
  }

  /**
   * Where image optimizations should occur.
   *
   * @version 3.36.0
   */
  var Region: {
    UsEast: 'us_east';
    UsCentral: 'us_central';
    UsWest: 'us_west';
    EuCentral: 'eu_central';
    EuWest: 'eu_west';
    Asia: 'asia';
    Australia: 'australia';
  };
  /**
   * Enable optimization features automatically based on browser support.
   *
   * @version 3.36.0
   */
  var Auto: {
    /** If the browser's Accept header indicates compatibility, deliver an AVIF image. */
    AVIF: 'avif';
    /** If the browser's Accept header indicates compatibility, deliver a WebP image. */
    WEBP: 'webp';
  };
  /**
   * Algorithm for converting an image to black and white.
   *
   * @version 3.36.0
   */
  var BWAlgorithm: {
    /** Uses a luminance threshold to convert the image to black and white. */
    Threshold: 'threshold';
    /** Uses Atkinson dithering to convert the image to black and white. */
    Atkinson: 'atkinson';
  };
  /**
   * Mode for content-aware cropping.
   *
   * @version 3.36.0
   */
  var CropMode: {
    /** Content-aware cropping that intelligently focuses on the most important visual content, including face detection. */
    Smart: 'smart';
    /** Allow cropping out-of-bounds regions. */
    Safe: 'safe';
  };
  /**
   * Disable features that are enabled by default.
   *
   * @version 3.36.0
   */
  var Disable: {
    /** Prevent images being resized larger than the source image. */
    Upscale: 'upscale';
  };
  /**
   * Enable features that are disabled by default.
   *
   * @version 3.36.0
   */
  var Enable: {
    /** Allow images to be resized larger than the source image. */
    Upscale: 'upscale';
  };
  /**
   * How the image will fit within the size bounds provided.
   *
   * @version 3.36.0
   */
  var Fit: {
    /** Resize to fit entirely within the specified region, making one dimension smaller if needed. */
    Bounds: 'bounds';
    /** Resize to entirely cover the specified region, making one dimension larger if needed. */
    Cover: 'cover';
    /** Resize and crop centrally to exactly fit the specified region. */
    Crop: 'crop';
  };
  /**
   * Output image format.
   *
   * @version 3.36.0
   */
  var Format: {
    /** Automatically use the best format based on browser support and image/transform characteristics. */
    Auto: 'auto';
    /** AVIF. */
    AVIF: 'avif';
    /** Baseline JPEG. */
    BJPG: 'bjpg';
    /** Graphics Interchange Format. */
    GIF: 'gif';
    /** JPEG. */
    JPG: 'jpg';
    /** JPEG XL. */
    JXL: 'jxl';
    /** MP4 (H.264). */
    MP4: 'mp4';
    /** Progressive JPEG. */
    PJPG: 'pjpg';
    /** Progressive JPEG XL. */
    PJXL: 'pjxl';
    /** Portable Network Graphics. */
    PNG: 'png';
    /** PNG palette image with 256 colors and 8-bit transparency. */
    PNG8: 'png8';
    /** Scalable Vector Graphics. */
    SVG: 'svg';
    /** WebP. */
    WEBP: 'webp';
    /** WebP (Lossless). */
    WEBPLL: 'webpll';
    /** WebP (Lossy). */
    WEBPLY: 'webply';
  };
  /**
   * Which metadata fields to preserve during transformation.
   *
   * @version 3.36.0
   */
  var Metadata: {
    /** Preserve copyright notice, creator, credit line, licensor, and web statement of rights fields. */
    Copyright: 'copyright';
    /** Preserve the C2PA manifest and add any transformations performed by Fastly Image Optimizer. */
    C2PA: 'c2pa';
    /** Preserve both copyright and C2PA metadata. */
    CopyrightAndC2PA: 'copyright,c2pa';
  };
  /**
   * Automatic quality compression level.
   *
   * @version 3.36.0
   */
  var Optimize: {
    /** Output image quality will be similar to the input image quality. */
    Low: 'low';
    /** More optimization is allowed; attempts to preserve visual quality. */
    Medium: 'medium';
    /** Minor visual artifacts may be visible; produces the smallest file. */
    High: 'high';
  };
  /**
   * Cardinal orientation of the image (EXIF orientation values).
   *
   * @version 3.36.0
   */
  var Orient: {
    Default: '1';
    FlipHorizontal: '2';
    FlipHorizontalAndVertical: '3';
    FlipVertical: '4';
    FlipHorizontalOrientLeft: '5';
    OrientRight: '6';
    FlipHorizontalOrientRight: '7';
    OrientLeft: '8';
  };
  /**
   * H.264 profile class when converting to video.
   *
   * @version 3.36.0
   */
  var Profile: {
    /** Recommended for video conferencing and mobile applications. (Default) */
    Baseline: 'baseline';
    /** Recommended for standard-definition broadcasts. */
    Main: 'main';
    /** Recommended for high-definition broadcasts. */
    High: 'high';
  };
  /**
   * Resize filter algorithm.
   *
   * @version 3.36.0
   */
  var ResizeFilter: {
    /** Uses the value of nearby translated pixel values. */
    Nearest: 'nearest';
    /** Uses an average of a 2x2 environment of pixels. */
    Bilinear: 'bilinear';
    /** Same as Bilinear. */
    Linear: 'linear';
    /** Uses an average of a 4x4 environment of pixels, weighing the innermost pixels higher. */
    Bicubic: 'bicubic';
    /** Same as Bicubic. */
    Cubic: 'cubic';
    /** Uses the Lanczos filter with sinc resampling for edge and linear feature detection. */
    Lanczos2: 'lanczos2';
    /** Uses a better approximation of the sinc resampling function. (Default) */
    Lanczos3: 'lanczos3';
    /** Same as Lanczos3. */
    Lanczos: 'lanczos';
  };

  /**
   * Options for the Fastly Image Optimizer, specified via the `imageOptimizerOptions` property
   * in the `Request` constructor. See the
   * [Image Optimizer reference docs](https://www.fastly.com/documentation/reference/io/) for
   * detailed documentation on all options.
   *
   * @example
   * ```js
   * import { Format, Orient, CropMode, Region } from 'fastly:image-optimizer';
   *
   * async function handleRequest(event) {
   *   return await fetch('https://www.w3.org/Graphics/PNG/text2.png', {
   *     imageOptimizerOptions: {
   *       region: Region.UsEast,
   *       format: Format.PNG,
   *       bgColor: { r: 100, g: 255, b: 9, a: 0.5 },
   *       brightness: -20,
   *       orient: Orient.FlipVertical,
   *       crop: { size: { ratio: { width: 4, height: 3 } }, mode: CropMode.Safe },
   *     },
   *     backend: 'my-backend'
   *   });
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));
   * ```
   *
   * @version 3.36.0
   */
  interface ImageOptimizerOptions {
    /**
     * Specify the region where image optimizations should occur. This is the only required option.
     */
    region:
      | 'us_east'
      | 'us_central'
      | 'us_west'
      | 'eu_central'
      | 'eu_west'
      | 'asia'
      | 'australia';
    /**
     * Enable optimization features automatically based on browser support.
     */
    auto?: 'avif' | 'webp';
    /**
     * Set the background color of an image.
     */
    bgColor?: Color;
    /**
     * Set the blurriness of the output image. Number values are in the range 0.5-1000;
     * percentage strings are also accepted.
     */
    blur?: number | Percentage;
    /**
     * Set the brightness of the output image (-100 to 100).
     */
    brightness?: number;
    /**
     * Convert an image to black and white using a given algorithm.
     */
    bw?: 'threshold' | 'atkinson';
    /**
     * Increase the size of the canvas around an image.
     */
    canvas?: {
      size: Size;
      position?: Position;
    };
    /**
     * Set the contrast of the output image (-100 to 100).
     */
    contrast?: number;
    /**
     * Remove pixels from an image.
     */
    crop?: {
      size: Size;
      position?: Position;
      mode?: 'smart' | 'safe';
    };
    /**
     * Disable features that are enabled by default.
     */
    disable?: 'upscale';
    /**
     * Set the ratio between physical pixels and logical pixels (1-10).
     */
    dpr?: number;
    /**
     * Enable features that are disabled by default.
     */
    enable?: 'upscale';
    /**
     * Set how the image will fit within the size bounds provided.
     */
    fit?: 'bounds' | 'cover' | 'crop';
    /**
     * Specify the output format to convert the image to.
     */
    format?:
      | 'auto'
      | 'avif'
      | 'bjpg'
      | 'gif'
      | 'jpg'
      | 'jxl'
      | 'mp4'
      | 'pjpg'
      | 'pjxl'
      | 'png'
      | 'png8'
      | 'svg'
      | 'webp'
      | 'webpll'
      | 'webply';
    /**
     * Extract the first frame from an animated image sequence.
     */
    frame?: 1;
    /**
     * Resize the height of the image, as integer pixels or a percentage string.
     */
    height?: number | Percentage;
    /**
     * Specify the level constraints when converting to video. Must be one of:
     * `"1.0"`, `"1.1"`, `"1.2"`, `"1.3"`, `"2.0"`, `"2.1"`, `"2.2"`, `"3.0"`, `"3.1"`,
     * `"3.2"`, `"4.0"`, `"4.1"`, `"4.2"`, `"5.0"`, `"5.1"`, `"5.2"`, `"6.0"`, `"6.1"`,
     * or `"6.2"`.
     */
    level?: string;
    /**
     * Control which metadata fields are preserved during transformation.
     */
    metadata?: 'copyright' | 'c2pa' | 'copyright,c2pa';
    /**
     * Automatically apply optimal quality compression.
     */
    optimize?: 'low' | 'medium' | 'high';
    /**
     * Change the cardinal orientation of the image.
     */
    orient?: '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8';
    /**
     * Add pixels to the edge of an image.
     */
    pad?: Sides;
    /**
     * Remove pixels from an image before any other transformations occur.
     */
    precrop?: {
      size: Size;
      position?: Position;
      mode?: 'smart' | 'safe';
    };
    /**
     * Specify the H.264 profile class when converting to video.
     */
    profile?: 'baseline' | 'main' | 'high';
    /**
     * Optimize the image to the given compression level for lossy file formatted images (0-100).
     */
    quality?: number;
    /**
     * Specify the resize filter used when resizing images.
     */
    resizeFilter?:
      | 'nearest'
      | 'bilinear'
      | 'linear'
      | 'bicubic'
      | 'cubic'
      | 'lanczos2'
      | 'lanczos3'
      | 'lanczos';
    /**
     * Set the saturation of the output image (-100 to 100).
     */
    saturation?: number;
    /**
     * Set the sharpness of the output image.
     */
    sharpen?: {
      /** Sharpening amount (0-10). */
      amount: number;
      /** Sharpening radius (0.5-1000). */
      radius: number;
      /** Sharpening threshold (0-255). */
      threshold: number;
    };
    /**
     * Remove pixels from the edge of an image.
     */
    trim?: Sides;
    /**
     * Trim pixels from the image edges that match the specified color. Either a {@link Color}
     * value, or an object with `color` and optional `threshold` (0-1) properties.
     */
    trimColor?:
      | Color
      | {
          color: Color;
          /** Tolerance for color matching (0-1). */
          threshold?: number;
        };
    /**
     * Remove explicit width and height properties in SVG output.
     */
    viewbox?: 1;
    /**
     * Resize the width of the image, as integer pixels or a percentage string.
     */
    width?: number | Percentage;
  }
  /**
   * Convert image optimizer options into the query string that is sent to the image optimizer,
   * for logging and debugging purposes.
   *
   * @version 3.36.0
   */
  function optionsToQueryString(options: ImageOptimizerOptions): string;
}
