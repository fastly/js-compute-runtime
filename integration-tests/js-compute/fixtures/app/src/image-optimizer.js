/* eslint-env serviceworker */

import { routes } from './routes.js';
import {
    Region,
    Auto,
    BWAlgorithm,
    CropMode,
    Disable,
    Enable,
    Fit,
    Metadata,
    Optimize,
    Orient,
    Profile,
    ResizeFilter,
    optionsToQueryString,
} from 'fastly:image-optimizer';
import { assert, assertThrows } from './assertions.js';

// Enums
routes.set('/image-optimizer/options/region', () => {
    assert(optionsToQueryString({ region: Region.UsEast }), 'region=us_east');
    assert(optionsToQueryString({ region: Region.Asia }), 'region=asia');
    assertThrows(() => optionsToQueryString({ region: 'invalid' }), TypeError);
    assertThrows(() => optionsToQueryString({}), TypeError);
});
routes.set('/image-optimizer/options/auto', () => {
    assert(optionsToQueryString({ region: Region.Asia, auto: Auto.AVIF }), 'region=asia&auto=avif');
    assert(optionsToQueryString({ region: Region.Asia, auto: Auto.WEBP }), 'region=asia&auto=webp');
    assertThrows(() => optionsToQueryString({ region: Region.Asia, auto: 'invalid' }), TypeError);
});
routes.set('/image-optimizer/options/bw', () => {
    assert(optionsToQueryString({ region: Region.EuCentral, bw: BWAlgorithm.Threshold }), 'region=eu_central&bw=threshold');
    assert(optionsToQueryString({ region: Region.EuCentral, bw: BWAlgorithm.Atkinson }), 'region=eu_central&bw=atkinson');
    assertThrows(() => optionsToQueryString({ region: Region.EuCentral, bw: 'invalid' }), TypeError);
});
routes.set('/image-optimizer/options/crop-mode', () => {
    const qs = optionsToQueryString({ region: Region.UsWest, crop: { size: { absolute: { width: 100, height: 100 } }, mode: CropMode.Smart } });
    assert(qs.includes('smart'), true);
    assertThrows(() => optionsToQueryString({ region: Region.UsWest, crop: { size: { absolute: { width: 100, height: 100 } }, mode: 'bad' } }), TypeError);
});
routes.set('/image-optimizer/options/disable', () => {
    assert(optionsToQueryString({ region: Region.Australia, disable: Disable.Upscale }), 'region=australia&disable=upscale');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, disable: 'invalid' }));
});
routes.set('/image-optimizer/options/enable', () => {
    assert(optionsToQueryString({ region: Region.Australia, enable: Enable.Upscale }), 'region=australia&enable=upscale');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, enable: 'invalid' }));
});
routes.set('/image-optimizer/options/fit', () => {
    assert(optionsToQueryString({ region: Region.Australia, fit: Fit.Crop }), 'region=australia&fit=crop');
    assert(optionsToQueryString({ region: Region.Australia, fit: Fit.Cover }), 'region=australia&fit=cover');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, fit: 'invalid' }));
});
routes.set('/image-optimizer/options/metadata', () => {
    assert(optionsToQueryString({ region: Region.Australia, metadata: Metadata.C2PA }), 'region=australia&metadata=c2pa');
    assert(optionsToQueryString({ region: Region.Australia, metadata: Metadata.CopyrightAndC2PA }), 'region=australia&metadata=copyright,c2pa');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, metadata: 'invalid' }));
});
routes.set('/image-optimizer/options/orient', () => {
    assert(optionsToQueryString({ region: Region.Australia, orient: Orient.Default }), 'region=australia&orient=1');
    assert(optionsToQueryString({ region: Region.Australia, orient: Orient.FlipHorizontalOrientRight }), 'region=australia&orient=7');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, orient: 'invalid' }));
});
routes.set('/image-optimizer/options/profile', () => {
    assert(optionsToQueryString({ region: Region.Australia, profile: Profile.Baseline }), 'region=australia&profile=baseline');
    assert(optionsToQueryString({ region: Region.Australia, profile: Profile.Main }), 'region=australia&profile=main');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, profile: 'invalid' }));
});
routes.set('/image-optimizer/options/resizeFilter', () => {
    assert(optionsToQueryString({ region: Region.Australia, resizeFilter: ResizeFilter.Bicubic }), 'region=australia&resize-filter=bicubic');
    assert(optionsToQueryString({ region: Region.Australia, resizeFilter: ResizeFilter.Lanczos2 }), 'region=australia&resize-filter=lanczos2');
    assertThrows(() => optionsToQueryString({ region: Region.Australia, resizeFilter: 'invalid' }));
});

// Other options
routes.set('/image-optimizer/options/bgColor', () => {
    // Hex strings
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: '123456'
    }), 'region=asia&bg-color=123456');
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: 'a2345e'
    }), 'region=asia&bg-color=a2345e');
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: '123'
    }), 'region=asia&bg-color=123');
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: 'a2e'
    }), 'region=asia&bg-color=a2e');
    assertThrows(() => optionsToQueryString({
        region: Region.Asia,
        bgColor: '12'
    }), TypeError);
    assertThrows(() => optionsToQueryString({
        region: Region.Asia,
        bgColor: '12j'
    }), TypeError);

    // RGB(A)
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: {
            r: 255,
            g: 0,
            b: 128,
        }
    }), 'region=asia&bg-color=255,0,128');
    assert(optionsToQueryString({
        region: Region.Asia,
        bgColor: {
            r: 255,
            g: 0,
            b: 128,
            a: 0.5
        }
    }), 'region=asia&bg-color=255,0,128,0.500000');
    assertThrows(() => optionsToQueryString({
        region: Region.Asia,
        bgColor: {
            r: 12,
            b: 12
        }
    }), TypeError);
    assertThrows(() => optionsToQueryString({
        region: Region.Asia,
        bgColor: {
            r: 12,
            b: 1212,
            g: 12,
        }
    }), TypeError);

    assertThrows(() => optionsToQueryString({
        region: Region.Asia,
        bgColor: 123123
    }), TypeError);
});

routes.set('/image-optimizer/options/blur', () => {
    assert(optionsToQueryString({ region: Region.Asia, blur: 0.5 }), 'region=asia&blur=0.500000');
    assert(optionsToQueryString({ region: Region.Asia, blur: 1000 }), 'region=asia&blur=1000.000000');
    assertThrows(() => optionsToQueryString({ region: Region.Asia, blur: 1001 }), TypeError);
    assertThrows(() => optionsToQueryString({ region: Region.Asia, blur: 0.4 }), TypeError);

    assert(optionsToQueryString({ region: Region.Asia, blur: "10%" }), 'region=asia&blur=10.000000p');
    assert(optionsToQueryString({ region: Region.Asia, blur: "0.5%" }), 'region=asia&blur=0.500000p');
});
routes.set('/image-optimizer/options/brightness', () => {
    assert(optionsToQueryString({ region: Region.Asia, brightness: 0.5 }), 'region=asia&brightness=0.500000');
    assert(optionsToQueryString({ region: Region.Asia, brightness: 100 }), 'region=asia&brightness=100.000000');
    assert(optionsToQueryString({ region: Region.Asia, brightness: -100 }), 'region=asia&brightness=-100.000000');
    assertThrows(() => optionsToQueryString({ region: Region.Asia, brightness: 1001 }), TypeError);
    assertThrows(() => optionsToQueryString({ region: Region.Asia, brightness: -101 }), TypeError);
});
routes.set('/image-optimizer/options/canvas', () => {
});
routes.set('/image-optimizer/options/crop', () => {
});
routes.set('/image-optimizer/options/dpr', () => {
});
routes.set('/image-optimizer/options/blur', () => {
});
routes.set('/image-optimizer/options/frame', () => {
});
routes.set('/image-optimizer/options/height', () => {
});
routes.set('/image-optimizer/options/level', () => {
});
routes.set('/image-optimizer/options/pad', () => {
});
routes.set('/image-optimizer/options/quality', () => {
});
routes.set('/image-optimizer/options/saturation', () => {
});
routes.set('/image-optimizer/options/sharpen', () => {
});
routes.set('/image-optimizer/options/trim', () => {
});
routes.set('/image-optimizer/options/viewbox', () => {
});
routes.set('/image-optimizer/options/width', () => {
});