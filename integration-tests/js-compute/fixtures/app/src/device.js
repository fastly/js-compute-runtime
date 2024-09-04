/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertThrows } from "./assertions-throwing.js";
import { Device } from "fastly:device";
import { routes } from "./routes.js";

let error;
routes.set("/device/interface", () => {
  let actual = Reflect.ownKeys(Device);
  let expected = ["prototype", "lookup", "length", "name"];
  assert(actual, expected, `Reflect.ownKeys(Device)`);

  // Check the prototype descriptors are correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Device, "prototype");
    expected = {
      value: Device.prototype,
      writable: false,
      enumerable: false,
      configurable: false,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device, 'prototype')`,
    );
  }

  // Check the constructor function's defined parameter length is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Device, "length");
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device, 'length')`,
    );
  }

  // Check the constructor function's name is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Device, "name");
    expected = {
      value: "Device",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device, 'name')`,
    );
  }

  // Check the prototype has the correct keys
  {
    actual = Reflect.ownKeys(Device.prototype);
    expected = [
      "constructor",
      "name",
      "brand",
      "model",
      "hardwareType",
      "isDesktop",
      "isGameConsole",
      "isMediaPlayer",
      "isMobile",
      "isSmartTV",
      "isTablet",
      "isTouchscreen",
      "toJSON",
      Symbol.toStringTag,
    ];
    assert(actual, expected, `Reflect.ownKeys(Device.prototype)`);
  }

  // Check the constructor on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Device.prototype, "constructor");
    expected = {
      writable: true,
      enumerable: false,
      configurable: true,
      value: Device.prototype.constructor,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.prototype, 'constructor')`,
    );

    assert(
      typeof Device.prototype.constructor,
      "function",
      `typeof Device.prototype.constructor`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Device.prototype.constructor,
      "length",
    );
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Device.prototype.constructor,
      "name",
    );
    expected = {
      value: "Device",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'name')`,
    );
  }

  // Check the Symbol.toStringTag on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      Device.prototype,
      Symbol.toStringTag,
    );
    expected = {
      writable: false,
      enumerable: false,
      configurable: true,
      value: "Device",
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.prototype, [Symbol.toStringTag])`,
    );

    assert(
      typeof Device.prototype[Symbol.toStringTag],
      "string",
      `typeof Device.prototype[Symbol.toStringTag]`,
    );
  }

  // Check the lookup static method has correct descriptors, length and name
  {
    actual = Reflect.getOwnPropertyDescriptor(Device, "lookup");
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: Device.lookup,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device, 'lookup')`,
    );

    assert(typeof Device.lookup, "function", `typeof Device.lookup`);

    actual = Reflect.getOwnPropertyDescriptor(Device.lookup, "length");
    expected = {
      value: 1,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.lookup, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(Device.lookup, "name");
    expected = {
      value: "lookup",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Device.lookup, 'name')`,
    );
  }

  for (let property of [
    "name",
    "brand",
    "model",
    "hardwareType",
    "isDesktop",
    "isGameConsole",
    "isMediaPlayer",
    "isMobile",
    "isSmartTV",
    "isTablet",
    "isTouchscreen",
  ]) {
    const descriptors = Reflect.getOwnPropertyDescriptor(
      Device.prototype,
      property,
    );
    expected = { enumerable: true, configurable: true };
    assert(
      descriptors.enumerable,
      true,
      `Reflect.getOwnPropertyDescriptor(Device, '${property}').enumerable`,
    );
    assert(
      descriptors.configurable,
      true,
      `Reflect.getOwnPropertyDescriptor(Device, '${property}').configurable`,
    );
    assert(
      descriptors.value,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Device, '${property}').value`,
    );
    assert(
      descriptors.set,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Device, '${property}').set`,
    );
    assert(
      typeof descriptors.get,
      "function",
      `typeof Reflect.getOwnPropertyDescriptor(Device, '${property}').get`,
    );
  }

  return pass("ok");
});

// Device constructor
{
  routes.set("/device/constructor/called-as-regular-function", () => {
    assertThrows(() => {
      Device();
    }, TypeError);
    return pass("ok");
  });
  routes.set("/device/constructor/throws", () => {
    assertThrows(() => new Device(), TypeError);
    return pass("ok");
  });
}

// Device lookup static method
// static lookup(useragent: string): DeviceEntry | null;
{
  routes.set("/device/lookup/called-as-constructor", () => {
    assertThrows(() => {
      new Device.lookup("1");
    }, TypeError);
    return pass("ok");
  });
  // https://tc39.es/ecma262/#sec-tostring
  routes.set("/device/lookup/useragent-parameter-calls-7.1.17-ToString", () => {
    let sentinel;
    const test = () => {
      sentinel = Symbol("sentinel");
      const useragent = {
        toString() {
          throw sentinel;
        },
      };
      Device.lookup(useragent);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, "thrownError === sentinel");
    }
    assertThrows(
      () => {
        Device.lookup(Symbol());
      },
      TypeError,
      `can't convert symbol to string`,
    );
    return pass("ok");
  });
  routes.set("/device/lookup/useragent-parameter-not-supplied", () => {
    assertThrows(
      () => {
        Device.lookup();
      },
      TypeError,
      `Device.lookup: At least 1 argument required, but only 0 passed`,
    );
    return pass("ok");
  });
  routes.set("/device/lookup/useragent-parameter-empty-string", () => {
    assertThrows(
      () => {
        Device.lookup("");
      },
      Error,
      `Device.lookup: useragent parameter can not be an empty string`,
    );
    return pass("ok");
  });
  routes.set("/device/lookup/useragent-does-not-exist-returns-null", () => {
    let result = Device.lookup(Math.random());
    assert(result, null, `Device.lookup(Math.random()) === null`);
    return pass("ok");
  });
  routes.set("/device/lookup/useragent-exists-all-fields-identified", () => {
    let useragent =
      "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0 FBAN/FBIOS;FBAV/8.0.0.28.18;FBBV/1665515;FBDV/iPhone4,1;FBMD/iPhone;FBSN/iPhone OS;FBSV/7.0.4;FBSS/2; FBCR/Telekom.de;FBID/phone;FBLC/de_DE;FBOP/5";
    let device = Device.lookup(useragent);

    assert(
      device instanceof Device,
      true,
      `Device.lookup(useragent) instanceof DeviceEntry`,
    );

    assert(device.name, "iPhone", `device.name`);
    assert(device.brand, "Apple", `device.brand`);
    assert(device.model, "iPhone4,1", `device.model`);
    assert(device.hardwareType, "Mobile Phone", `device.hardwareType`);
    assert(device.isDesktop, false, `device.isDesktop`);
    assert(device.isGameConsole, false, `device.isGameConsole`);
    assert(device.isMediaPlayer, false, `device.isMediaPlayer`);
    assert(device.isMobile, true, `device.isMobile`);
    assert(device.isSmartTV, false, `device.isSmartTV`);
    assert(device.isTablet, false, `device.isTablet`);
    assert(device.isTouchscreen, true, `device.isTouchscreen`);
    return pass("ok");
  });
  routes.set(
    "/device/lookup/useragent-exists-all-fields-identified-tojson",
    () => {
      let useragent =
        "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0 FBAN/FBIOS;FBAV/8.0.0.28.18;FBBV/1665515;FBDV/iPhone4,1;FBMD/iPhone;FBSN/iPhone OS;FBSV/7.0.4;FBSS/2; FBCR/Telekom.de;FBID/phone;FBLC/de_DE;FBOP/5";
      let device = Device.lookup(useragent);
      device = device.toJSON();

      assert(device.brand, "Apple", `device.brand`);
      assert(device.model, "iPhone4,1", `device.model`);
      assert(device.hardwareType, "Mobile Phone", `device.hardwareType`);
      assert(device.isDesktop, false, `device.isDesktop`);
      assert(device.isGameConsole, false, `device.isGameConsole`);
      assert(device.isMediaPlayer, false, `device.isMediaPlayer`);
      assert(device.isMobile, true, `device.isMobile`);
      assert(device.isSmartTV, false, `device.isSmartTV`);
      assert(device.isTablet, false, `device.isTablet`);
      assert(device.isTouchscreen, true, `device.isTouchscreen`);

      return pass("ok");
    },
  );
  routes.set("/device/lookup/useragent-exists-some-fields-identified", () => {
    let useragent =
      "ghosts-app/1.0.2.1 (ASUSTeK COMPUTER INC.; X550CC; Windows 8 (X86); en)";
    let device = Device.lookup(useragent);

    assert(
      device instanceof Device,
      true,
      `Device.lookup(useragent) instanceof DeviceEntry`,
    );
    assert(device.name, "Asus TeK", `device.name`);
    assert(device.brand, "Asus", `device.brand`);
    assert(device.model, "TeK", `device.model`);
    assert(device.hardwareType, null, `device.hardwareType`);
    assert(device.isDesktop, false, `device.isDesktop`);
    assert(device.isGameConsole, null, `device.isGameConsole`);
    assert(device.isMediaPlayer, null, `device.isMediaPlayer`);
    assert(device.isMobile, null, `device.isMobile`);
    assert(device.isSmartTV, null, `device.isSmartTV`);
    assert(device.isTablet, null, `device.isTablet`);
    assert(device.isTouchscreen, null, `device.isTouchscreen`);
    assert(
      JSON.stringify(device),
      '{"name":"Asus TeK","brand":"Asus","model":"TeK","hardwareType":null,"isDesktop":false,"isGameConsole":null,"isMediaPlayer":null,"isMobile":null,"isSmartTV":null,"isTablet":null,"isTouchscreen":null}',
      `JSON.stringify(device)`,
    );
    return pass("ok");
  });
}
