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
  assert(
    optionsToQueryString({ region: Region.Asia, auto: Auto.AVIF }),
    'region=asia&auto=avif',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, auto: Auto.WEBP }),
    'region=asia&auto=webp',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, auto: 'invalid' }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/bw', () => {
  assert(
    optionsToQueryString({
      region: Region.EuCentral,
      bw: BWAlgorithm.Threshold,
    }),
    'region=eu_central&bw=threshold',
  );
  assert(
    optionsToQueryString({
      region: Region.EuCentral,
      bw: BWAlgorithm.Atkinson,
    }),
    'region=eu_central&bw=atkinson',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.EuCentral, bw: 'invalid' }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/crop-mode', () => {
  const qs = optionsToQueryString({
    region: Region.UsWest,
    crop: {
      size: { absolute: { width: 100, height: 100 } },
      mode: CropMode.Smart,
    },
  });
  assert(qs.includes('smart'), true);
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.UsWest,
        crop: { size: { absolute: { width: 100, height: 100 } }, mode: 'bad' },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/disable', () => {
  assert(
    optionsToQueryString({
      region: Region.Australia,
      disable: Disable.Upscale,
    }),
    'region=australia&disable=upscale',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, disable: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/enable', () => {
  assert(
    optionsToQueryString({ region: Region.Australia, enable: Enable.Upscale }),
    'region=australia&enable=upscale',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, enable: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/fit', () => {
  assert(
    optionsToQueryString({ region: Region.Australia, fit: Fit.Crop }),
    'region=australia&fit=crop',
  );
  assert(
    optionsToQueryString({ region: Region.Australia, fit: Fit.Cover }),
    'region=australia&fit=cover',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, fit: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/metadata', () => {
  assert(
    optionsToQueryString({ region: Region.Australia, metadata: Metadata.C2PA }),
    'region=australia&metadata=c2pa',
  );
  assert(
    optionsToQueryString({
      region: Region.Australia,
      metadata: Metadata.CopyrightAndC2PA,
    }),
    'region=australia&metadata=copyright,c2pa',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, metadata: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/optimize', () => {
  assert(
    optionsToQueryString({ region: Region.Australia, optimize: Optimize.Low }),
    'region=australia&optimize=low',
  );
  assert(
    optionsToQueryString({ region: Region.Australia, optimize: Optimize.High }),
    'region=australia&optimize=high',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, optimize: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/orient', () => {
  assert(
    optionsToQueryString({ region: Region.Australia, orient: Orient.Default }),
    'region=australia&orient=1',
  );
  assert(
    optionsToQueryString({
      region: Region.Australia,
      orient: Orient.FlipHorizontalOrientRight,
    }),
    'region=australia&orient=7',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, orient: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/profile', () => {
  assert(
    optionsToQueryString({
      region: Region.Australia,
      profile: Profile.Baseline,
    }),
    'region=australia&profile=baseline',
  );
  assert(
    optionsToQueryString({ region: Region.Australia, profile: Profile.Main }),
    'region=australia&profile=main',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, profile: 'invalid' }),
  );
});
routes.set('/image-optimizer/options/resizeFilter', () => {
  assert(
    optionsToQueryString({
      region: Region.Australia,
      resizeFilter: ResizeFilter.Bicubic,
    }),
    'region=australia&resize-filter=bicubic',
  );
  assert(
    optionsToQueryString({
      region: Region.Australia,
      resizeFilter: ResizeFilter.Lanczos2,
    }),
    'region=australia&resize-filter=lanczos2',
  );
  assertThrows(() =>
    optionsToQueryString({ region: Region.Australia, resizeFilter: 'invalid' }),
  );
});

// Other options
routes.set('/image-optimizer/options/bgColor', () => {
  // Hex strings
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: '123456',
    }),
    'region=asia&bg-color=123456',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: 'a2345e',
    }),
    'region=asia&bg-color=a2345e',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: '123',
    }),
    'region=asia&bg-color=123',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: 'a2e',
    }),
    'region=asia&bg-color=a2e',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        bgColor: '12',
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        bgColor: '12j',
      }),
    TypeError,
  );

  // RGB(A)
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: {
        r: 255,
        g: 0,
        b: 128,
      },
    }),
    'region=asia&bg-color=255,0,128',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      bgColor: {
        r: 255,
        g: 0,
        b: 128,
        a: 0.5,
      },
    }),
    'region=asia&bg-color=255,0,128,0.500000',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        bgColor: {
          r: 12,
          b: 12,
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        bgColor: {
          r: 12,
          b: 1212,
          g: 12,
        },
      }),
    TypeError,
  );

  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        bgColor: 123123,
      }),
    TypeError,
  );
});

routes.set('/image-optimizer/options/blur', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, blur: 0.5 }),
    'region=asia&blur=0.500000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, blur: 1000 }),
    'region=asia&blur=1000.000000',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, blur: 1001 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, blur: 0.4 }),
    TypeError,
  );

  assert(
    optionsToQueryString({ region: Region.Asia, blur: '10%' }),
    'region=asia&blur=10.000000p',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, blur: '0.5%' }),
    'region=asia&blur=0.500000p',
  );
});
routes.set('/image-optimizer/options/brightness', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, brightness: 0.5 }),
    'region=asia&brightness=0.500000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, brightness: 100 }),
    'region=asia&brightness=100.000000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, brightness: -100 }),
    'region=asia&brightness=-100.000000',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, brightness: 1001 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, brightness: -101 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/canvas', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      canvas: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
      },
    }),
    'region=asia&canvas=100,10.000000p',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      canvas: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
      },
    }),
    'region=asia&canvas=4.000000:3.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      canvas: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
        position: {
          x: 10,
          offsetY: 10,
        },
      },
    }),
    'region=asia&canvas=100,10.000000p,x10,offset-y10.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      canvas: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
        position: {
          offsetX: 10,
          y: '10%',
        },
      },
    }),
    'region=asia&canvas=4.000000:3.000000,offset-x10.000000,y10.000000p',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        canvas: {
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        canvas: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        canvas: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            x: 100,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        canvas: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            y: '10%',
          },
        },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/crop', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      crop: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
      },
    }),
    'region=asia&crop=100,10.000000p',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      crop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
      },
    }),
    'region=asia&crop=4.000000:3.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      crop: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
        position: {
          x: 10,
          offsetY: 10,
        },
      },
    }),
    'region=asia&crop=100,10.000000p,x10,offset-y10.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      crop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
        position: {
          offsetX: 10,
          y: '10%',
        },
      },
    }),
    'region=asia&crop=4.000000:3.000000,offset-x10.000000,y10.000000p',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      crop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
        position: {
          offsetX: 10,
          y: '10%',
        },
        mode: CropMode.Safe,
      },
    }),
    'region=asia&crop=4.000000:3.000000,offset-x10.000000,y10.000000p,safe',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        crop: {
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        crop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        crop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            x: 100,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        crop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        crop: {
          size: {
            ratio: {
              width: 4,
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            y: '10%',
          },
          mode: 'invalid',
        },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/precrop', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      precrop: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
      },
    }),
    'region=asia&precrop=100,10.000000p',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      precrop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
      },
    }),
    'region=asia&precrop=4.000000:3.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      precrop: {
        size: {
          absolute: {
            width: 100,
            height: '10%',
          },
        },
        position: {
          x: 10,
          offsetY: 10,
        },
      },
    }),
    'region=asia&precrop=100,10.000000p,x10,offset-y10.000000',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      precrop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
        position: {
          offsetX: 10,
          y: '10%',
        },
      },
    }),
    'region=asia&precrop=4.000000:3.000000,offset-x10.000000,y10.000000p',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      precrop: {
        size: {
          ratio: {
            width: 4,
            height: 3,
          },
        },
        position: {
          offsetX: 10,
          y: '10%',
        },
        mode: CropMode.Safe,
      },
    }),
    'region=asia&precrop=4.000000:3.000000,offset-x10.000000,y10.000000p,safe',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        precrop: {
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        precrop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        precrop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            x: 100,
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        precrop: {
          size: {
            ratio: {
              width: '4%',
              height: 3,
            },
          },
          position: {
            y: '10%',
          },
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        precrop: {
          size: {
            ratio: {
              width: 4,
              height: 3,
            },
          },
          position: {
            offsetX: 10,
            y: '10%',
          },
          mode: 'invalid',
        },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/dpr', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, dpr: 1.5 }),
    'region=asia&dpr=1.500000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, dpr: 10 }),
    'region=asia&dpr=10.000000',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, dpr: '1001' }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, dpr: 1001 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, dpr: 0.5 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/frame', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, frame: 1 }),
    'region=asia&frame=1',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, frame: 2 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/height', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, height: 1000 }),
    'region=asia&height=1000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, height: '10%' }),
    'region=asia&height=10.000000p',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, height: '1001' }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, height: 100.5 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/level', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, level: '1.1' }),
    'region=asia&level=1.1',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, level: '5.1' }),
    'region=asia&level=5.1',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, level: 5.1 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, level: '7.1' }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/pad', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      pad: {
        top: 10,
        bottom: 20,
        left: '10%',
        right: '20%',
      },
    }),
    'region=asia&pad=10,20.000000p,20,10.000000p',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        pad: {
          top: 10,
          bottom: 20,
          left: '10',
          right: '20%',
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        pad: {
          top: 10,
          left: '10',
          right: '20%',
        },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/quality', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, quality: 1 }),
    'region=asia&quality=1',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, quality: 100 }),
    'region=asia&quality=100',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, quality: 1001 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, quality: 1.5 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, quality: 0.4 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/saturation', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, saturation: 1 }),
    'region=asia&saturation=1.000000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, saturation: 100 }),
    'region=asia&saturation=100.000000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, saturation: -100 }),
    'region=asia&saturation=-100.000000',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, saturation: 1001 }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, saturation: -101 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/sharpen', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: 10,
        radius: 10,
        threshold: 1,
      },
    }),
    'region=asia&sharpen=a10.000000,r10.000000,t1',
  );
  assert(
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: 0.1,
        radius: 0.5,
        threshold: 255,
      },
    }),
    'region=asia&sharpen=a0.100000,r0.500000,t255',
  );
  assertThrows(() => {
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: 0.1,
        radius: 0.5,
        threshold: 256,
      },
    });
  }, TypeError);
  assertThrows(() => {
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: 0.1,
        radius: 0.4,
        threshold: 255,
      },
    });
  }, TypeError);
  assertThrows(() => {
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: -1,
        radius: 0.5,
        threshold: 255,
      },
    });
  }, TypeError);
  assertThrows(() => {
    optionsToQueryString({
      region: Region.Asia,
      sharpen: {
        amount: 1,
        radius: 1,
      },
    });
  }, TypeError);
});
routes.set('/image-optimizer/options/trim', () => {
  assert(
    optionsToQueryString({
      region: Region.Asia,
      trim: {
        top: 10,
        bottom: 20,
        left: '10%',
        right: '20%',
      },
    }),
    'region=asia&trim=10,20.000000p,20,10.000000p',
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        trim: {
          top: 10,
          bottom: 20,
          left: '10',
          right: '20%',
        },
      }),
    TypeError,
  );
  assertThrows(
    () =>
      optionsToQueryString({
        region: Region.Asia,
        trim: {
          top: 10,
          left: '10',
          right: '20%',
        },
      }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/viewbox', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, viewbox: 1 }),
    'region=asia&viewbox=1',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, viewbox: 2 }),
    TypeError,
  );
});
routes.set('/image-optimizer/options/width', () => {
  assert(
    optionsToQueryString({ region: Region.Asia, width: 1000 }),
    'region=asia&width=1000',
  );
  assert(
    optionsToQueryString({ region: Region.Asia, width: '10%' }),
    'region=asia&width=10.000000p',
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, width: '1001' }),
    TypeError,
  );
  assertThrows(
    () => optionsToQueryString({ region: Region.Asia, width: 100.5 }),
    TypeError,
  );
});
