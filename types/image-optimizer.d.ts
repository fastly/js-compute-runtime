declare module 'fastly:image-optimizer' {
    /**
     * A color, either a 3/6 character hex string or an rgb(a) object.
     */
    type Color = string | {
        r: number;
        g: number;
        b: number;
        a?: number;
    }
    /**
     * A percentage, expressed as a string such as '100%'
     */
    type Percentage = string;
    /**
     * The size of a region, either expressed as absolute values (either integer pixel values or percentage strings), or as a ratio of integers.
     */
    interface Size {
        absolute?: {
            width: number | Percentage;
            height: number | Percentage;
        };
        ratio?: {
            width: number;
            height: number;
        }
    }
    /**
     * The position of a region, with x and y components expressed as either integer pixel values/percentage strings, or offset percentages.
     */
    interface Position {
        x?: number | Percentage;
        offsetX?: number;
        y?: number | Percentage;
        offsetY?: number;
    }

    interface Sides {
        top: number | Percentage;
        bottom: number | Percentage;
        left: number | Percentage;
        right: number | Percentage;
    }

    var Region: {
        UsEast: 'us_east';
        UsCentral: 'us_central';
        UsWest: 'us_west';
        EuCentral: 'eu_central';
        Asia: 'asia';
        Australia: 'australia';
    }
    var Auto: {
        AVIF: 'avif';
        WEBP: 'webp';
    }
    var BWAlgorithm: {
        Threshold: 'threshold';
        Atkinson: 'atkinson';
    }
    var CropMode: {
        Smart: 'smart';
        Safe: 'safe';
    }
    var Disable: {
        Upscale: 'upscale';
    }
    var Enable: {
        Upscale: 'upscale';
    }
    var Fit: {
        Bounds: "bounds";
        Cover: "cover";
        Crop: "crop";
    }
    var Metadata: {
        Copyright: "copyright";
        C2PA: "c2pa";
        CopyrightAndC2PA: "copyright,c2pa";
    }
    var Optimize: {
        Low: "low";
        Medium: "medium";
        High: "high";
    }
    var Orient: {
        Default: "1";
        FlipHorizontal: "2";
        FlipHorizontalAndVertical: "3";
        FlipVertical: "4";
        FlipHorizontalOrientLeft: "5";
        OrientRight: "6";
        FlipHorizontalOrientRight: "7";
        OrientLeft: "8";
    }
    var Profile: {
        Baseline: "baseline";
        Main: "main";
        High: "high";
    }
    var ResizeFilter: {
        Nearest: "nearest";
        Bilinear: "bilinear";
        Linear: "linear";
        Bicubic: "bicubic";
        Cubic: "cubic";
        Lanczos2: "lanczos2";
        Lanczos3: "lanczos3";
        Lanczos: "lanczos";
    }

    interface ImageOptimizerOptions {
        /**
         * 
         */
        region: 'us_east' | 'us_central' | 'us_west' | 'eu_central' | 'asia' | 'australia';
        /**
         * Enable optimization features automatically.
         */
        auto?: 'avif' | 'webp';
        /**
         * Set the background color of an image.
         */
        bgColor?: Color;
        /**
         * Set the blurriness of the output image (0.5-1000).
         */
        blur?: number | string;
        /**
         * Set the brightness of the output image.
         */
        brightness?: number | string;
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
        }
        /**
         * Set the contrast of the output image (-100-100).
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
         * Disable functionality that is enabled by default.
         */
        disable?: 'upscale';
        /** 
         * Ratio between physical pixels and logical pixels. 
         */
        dpr?: number;
        /**
         * Enable functionality that is disabled by default.
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
        'auto' |
        'avif' |
        'bjpg' |
        'gif' |
        'jpg' |
        'jxl' |
        'mp4' |
        'pjpg' |
        'pjxl' |
        'png' |
        'png8' |
        'svg' |
        'webp' |
        'webpll' |
        'webply';
        /**
         * Extract the first frame from an animated image.
         */
        frame?: 1;
        /**
         * Resize the height of the image.
         */
        height?: number | Percentage;
        /**
         * Specify the level constraints when converting to video.
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
         * Add pixels to the edge of an image
         */
        pad?: Sides,
        /**
         * Remove pixels from an image before any other transformations occur.
         */
        precrop?: {
            size: Size;
            position?: Position;
            mode?: 'smart' | 'safe';
        };
        /**
         * Specify the profile class of application when converting to video.
         */
        profile?: 'baseline' | 'main' | 'high';
        /**
         * Optimize the image to the given compresion level for lossy file formatted images (1-100).
         */
        quality?: number;
        /**
         * Specify the resize filter used when resizing images.
         */
        resizeFilter?: 'nearest' | 'bilinear' | 'linear' | 'bicubic' | 'cubic' | 'lanczos2' | 'lanczos3' | 'lanczos';
        /** 
         * Set the saturation of the output image (-100-100). 
         */
        saturation?: number;
        /**
         * Set the sharpness of the output image.
         */
        sharpen?: {
            amount: number;
            radius: number;
            threshold: number;
        };
        /** 
         * Remove pixels from the edge of an image.
         */
        trim?: Sides;
        /**
         * Remove explicit width and height properties in SVG output.
         */
        viewbox?: 1;
        /**
         * Resize the width of the image.
         */
        width?: number | Percentage;
    }
}