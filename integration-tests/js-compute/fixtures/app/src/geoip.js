/* eslint-env serviceworker */
/* global fastly */
import { assert, assertThrows } from './assertions.js';
import { getGeolocationForIpAddress } from 'fastly:geolocation';
import { isRunningLocally, routes } from './routes.js';

routes.set('/fastly/getgeolocationforipaddress/interface', async function () {
  let actual = Reflect.getOwnPropertyDescriptor(
    fastly,
    'getGeolocationForIpAddress',
  );
  let expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: fastly.getGeolocationForIpAddress,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(fastly, 'getGeolocationForIpAddress)`,
  );

  assert(
    typeof fastly.getGeolocationForIpAddress,
    'function',
    `typeof fastly.getGeolocationForIpAddress`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    fastly.getGeolocationForIpAddress,
    'length',
  );
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    fastly.getGeolocationForIpAddress,
    'name',
  );
  expected = {
    value: 'getGeolocationForIpAddress',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'name')`,
  );
});

routes.set(
  '/fastly/getgeolocationforipaddress/called-as-constructor',
  async () => {
    assertThrows(
      () => {
        new fastly.getGeolocationForIpAddress('1.2.3.4');
      },
      TypeError,
      `fastly.getGeolocationForIpAddress is not a constructor`,
    );
  },
);
// https://tc39.es/ecma262/#sec-tostring
routes.set(
  '/fastly/getgeolocationforipaddress/parameter-calls-7.1.17-ToString',
  async () => {
    let sentinel;
    const test = () => {
      sentinel = Symbol();
      const key = {
        toString() {
          throw sentinel;
        },
      };
      fastly.getGeolocationForIpAddress(key);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, 'thrownError === sentinel');
    }
    assertThrows(
      () => {
        fastly.getGeolocationForIpAddress(Symbol());
      },
      Error,
      `can't convert symbol to string`,
    );
  },
);
routes.set(
  '/fastly/getgeolocationforipaddress/parameter-not-supplied',
  async () => {
    assertThrows(
      () => {
        fastly.getGeolocationForIpAddress();
      },
      TypeError,
      `fastly.getGeolocationForIpAddress: At least 1 argument required, but only 0 passed`,
    );
  },
);
routes.set(
  '/fastly/getgeolocationforipaddress/parameter-empty-string',
  async () => {
    assertThrows(
      () => {
        fastly.getGeolocationForIpAddress('');
      },
      Error,
      `Invalid address passed to fastly.getGeolocationForIpAddress`,
    );
  },
);

let geoFields = [
  'area_code',
  'as_name',
  'as_number',
  'city',
  'conn_speed',
  'conn_type',
  'continent',
  'country_code',
  'country_code3',
  'country_name',
  'gmt_offset',
  'latitude',
  'longitude',
  'metro_code',
  'postal_code',
  'proxy_description',
  'proxy_type',
  'region',
  'utc_offset',
];

routes.set(
  '/fastly/getgeolocationforipaddress/parameter-ipv4-string',
  async () => {
    if (isRunningLocally()) {
      let geo = fastly.getGeolocationForIpAddress('2.216.196.179');
      assert(
        Object.keys(geo),
        geoFields,
        `Object.keys(fastly.getGeolocationForIpAddress('2.216.196.179')) == geoFields`,
      );
    }
  },
);

routes.set(
  '/fastly/getgeolocationforipaddress/parameter-compressed-ipv6-string',
  async () => {
    if (isRunningLocally()) {
      let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4');
      assert(
        Object.keys(geo),
        geoFields,
        `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4')) == geoFields`,
      );
    }
  },
);
routes.set(
  '/fastly/getgeolocationforipaddress/parameter-shortened-ipv6-string',
  async () => {
    if (isRunningLocally()) {
      let geo = fastly.getGeolocationForIpAddress(
        '2607:f0d0:1002:0051:0:0:0:0004',
      );
      assert(
        Object.keys(geo),
        geoFields,
        `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0:0:0:0004')) == geoFields`,
      );
    }
  },
);
routes.set(
  '/fastly/getgeolocationforipaddress/parameter-expanded-ipv6-string',
  async () => {
    if (isRunningLocally()) {
      let geo = fastly.getGeolocationForIpAddress(
        '2607:f0d0:1002:0051:0000:0000:0000:0004',
      );
      assert(
        Object.keys(geo),
        geoFields,
        `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')) == geoFields`,
      );
    }
  },
);

routes.set('/fastly/getgeolocationforipaddress/called-unbound', async () => {
  if (isRunningLocally()) {
    let geo = fastly.getGeolocationForIpAddress.call(
      undefined,
      '2607:f0d0:1002:0051:0000:0000:0000:0004',
    );
    assert(
      Object.keys(geo),
      geoFields,
      `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')) == geoFields`,
    );
  }
});

routes.set('/fastly:geolocation', async () => {
  assert(
    getGeolocationForIpAddress,
    fastly.getGeolocationForIpAddress,
    'getGeolocationForIpAddress === fastly.getGeolocationForIpAddress',
  );
});
