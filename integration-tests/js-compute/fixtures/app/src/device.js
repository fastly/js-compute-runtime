/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertThrows } from "./assertions.js";
import { Device } from 'fastly:device';
import { routes } from "./routes.js";

let error;
routes.set("/device/interface", () => {
    let actual = Reflect.ownKeys(Device)
    let expected = ["prototype", "lookup", "length", "name"]
    error = assert(actual, expected, `Reflect.ownKeys(Device)`)
    if (error) { return error }

    // Check the prototype descriptors are correct
    {
        actual = Reflect.getOwnPropertyDescriptor(Device, 'prototype')
        expected = {
            "value": Device.prototype,
            "writable": false,
            "enumerable": false,
            "configurable": false
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device, 'prototype')`)
        if (error) { return error }
    }

    // Check the constructor function's defined parameter length is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(Device, 'length')
        expected = {
            "value": 0,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device, 'length')`)
        if (error) { return error }
    }

    // Check the constructor function's name is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(Device, 'name')
        expected = {
            "value": "Device",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device, 'name')`)
        if (error) { return error }
    }

    // Check the prototype has the correct keys
    {
        actual = Reflect.ownKeys(Device.prototype)
        expected = ["constructor", "name", "brand", "model", "hardwareType", "isDesktop", "isGameConsole", "isMediaPlayer", "isMobile", "isSmartTV", "isTablet", "isTouchscreen", "toJSON", Symbol.toStringTag]
        error = assert(actual, expected, `Reflect.ownKeys(Device.prototype)`)
        if (error) { return error }
    }

    // Check the constructor on the prototype is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(Device.prototype, 'constructor')
        expected = { "writable": true, "enumerable": false, "configurable": true, value: Device.prototype.constructor }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.prototype, 'constructor')`)
        if (error) { return error }

        error = assert(typeof Device.prototype.constructor, 'function', `typeof Device.prototype.constructor`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'length')
        expected = {
            "value": 0,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'name')
        expected = {
            "value": "Device",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.prototype.constructor, 'name')`)
        if (error) { return error }
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(Device.prototype, Symbol.toStringTag)
        expected = { "writable": false, "enumerable": false, "configurable": true, value: "Device" }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.prototype, [Symbol.toStringTag])`)
        if (error) { return error }

        error = assert(typeof Device.prototype[Symbol.toStringTag], 'string', `typeof Device.prototype[Symbol.toStringTag]`)
        if (error) { return error }
    }

    // Check the lookup static method has correct descriptors, length and name
    {
        actual = Reflect.getOwnPropertyDescriptor(Device, 'lookup')
        expected = { "writable": true, "enumerable": true, "configurable": true, value: Device.lookup }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device, 'lookup')`)
        if (error) { return error }

        error = assert(typeof Device.lookup, 'function', `typeof Device.lookup`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(Device.lookup, 'length')
        expected = {
            "value": 1,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.lookup, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(Device.lookup, 'name')
        expected = {
            "value": "lookup",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Device.lookup, 'name')`)
        if (error) { return error }
    }

    for (let property of ["name", "brand", "model", "hardwareType", "isDesktop", "isGameConsole", "isMediaPlayer", "isMobile", "isSmartTV", "isTablet", "isTouchscreen"]) {
        const descriptors = Reflect.getOwnPropertyDescriptor(Device.prototype, property)
        expected = { "enumerable": true, "configurable": true }
        error = assert(descriptors.enumerable, true, `Reflect.getOwnPropertyDescriptor(Device, '${property}').enumerable`)
        error = assert(descriptors.configurable, true, `Reflect.getOwnPropertyDescriptor(Device, '${property}').configurable`)
        error = assert(descriptors.value, undefined, `Reflect.getOwnPropertyDescriptor(Device, '${property}').value`)
        error = assert(descriptors.set, undefined, `Reflect.getOwnPropertyDescriptor(Device, '${property}').set`)
        error = assert(typeof descriptors.get, 'function', `typeof Reflect.getOwnPropertyDescriptor(Device, '${property}').get`)
        if (error) { return error }
    }

    return pass('ok')
});

// Device constructor
{

    routes.set("/device/constructor/called-as-regular-function", () => {
        error = assertThrows(() => {
            Device()
        }, TypeError, `Illegal constructor`)
        if (error) { return error }
        return pass('ok')
    });
    routes.set("/device/constructor/throws", () => {
        error = assertThrows(() => new Device(), TypeError, `Illegal constructor`)
        if (error) { return error }
        return pass('ok')
    });
}

// Device lookup static method
// static lookup(useragent: string): DeviceEntry | null;
{
    routes.set("/device/lookup/called-as-constructor", () => {
        let error = assertThrows(() => {
            new Device.lookup('1')
        }, TypeError, `Device.lookup is not a constructor`)
        if (error) { return error }
        return pass('ok')
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/device/lookup/useragent-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol('sentinel');
            const useragent = {
                toString() {
                    throw sentinel;
                }
            }
            Device.lookup(useragent)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = assertThrows(() => {
            Device.lookup(Symbol())
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass('ok')
    });
    routes.set("/device/lookup/useragent-parameter-not-supplied", () => {
        let error = assertThrows(() => {
            Device.lookup()
        }, TypeError, `Device.lookup: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass('ok')
    });
    routes.set("/device/lookup/useragent-parameter-empty-string", () => {
        let error = assertThrows(() => {
            Device.lookup('')
        }, Error, `Device.lookup: useragent parameter can not be an empty string`)
        if (error) { return error }
        return pass('ok')
    });
    routes.set("/device/lookup/useragent-does-not-exist-returns-null", () => {
        let result = Device.lookup(Math.random())
        error = assert(result, null, `Device.lookup(Math.random()) === null`)
        if (error) { return error }
        return pass('ok')
    });
    routes.set("/device/lookup/useragent-exists-all-fields-identified", () => {
        let useragent = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0 FBAN/FBIOS;FBAV/8.0.0.28.18;FBBV/1665515;FBDV/iPhone4,1;FBMD/iPhone;FBSN/iPhone OS;FBSV/7.0.4;FBSS/2; FBCR/Telekom.de;FBID/phone;FBLC/de_DE;FBOP/5";
        let device = Device.lookup(useragent);

        error = assert(device instanceof Device, true, `Device.lookup(useragent) instanceof DeviceEntry`)
        if (error) { return error }

        error = assert(device.name, "iPhone", `device.name`)
        if (error) { return error }

        error = assert(device.brand, "Apple", `device.brand`)
        if (error) { return error }

        error = assert(device.model, "iPhone4,1", `device.model`)
        if (error) { return error }
        
        error = assert(device.hardwareType, "Mobile Phone", `device.hardwareType`)
        if (error) { return error }
        
        error = assert(device.isDesktop, false, `device.isDesktop`)
        if (error) { return error }
        
        error = assert(device.isGameConsole, false, `device.isGameConsole`)
        if (error) { return error }
        
        error = assert(device.isMediaPlayer, false, `device.isMediaPlayer`)
        if (error) { return error }
        
        error = assert(device.isMobile, true, `device.isMobile`)
        if (error) { return error }
        
        error = assert(device.isSmartTV, false, `device.isSmartTV`)
        if (error) { return error }
        
        error = assert(device.isTablet, false, `device.isTablet`)
        if (error) { return error }
        
        error = assert(device.isTouchscreen, true, `device.isTouchscreen`)
        if (error) { return error }

        return pass('ok')
    });
    routes.set("/device/lookup/useragent-exists-some-fields-identified", () => {
        let useragent = "ghosts-app/1.0.2.1 (ASUSTeK COMPUTER INC.; X550CC; Windows 8 (X86); en)";
        let device = Device.lookup(useragent);

        error = assert(device instanceof Device, true, `Device.lookup(useragent) instanceof DeviceEntry`)
        if (error) { return error }

        error = assert(device.name, "Asus TeK", `device.name`)
        if (error) { return error }

        error = assert(device.brand, "Asus", `device.brand`)
        if (error) { return error }

        error = assert(device.model, "TeK", `device.model`)
        if (error) { return error }
        
        error = assert(device.hardwareType, null, `device.hardwareType`)
        if (error) { return error }
        
        error = assert(device.isDesktop, false, `device.isDesktop`)
        if (error) { return error }
        
        error = assert(device.isGameConsole, null, `device.isGameConsole`)
        if (error) { return error }
        
        error = assert(device.isMediaPlayer, null, `device.isMediaPlayer`)
        if (error) { return error }

        error = assert(device.isMobile, null, `device.isMobile`)
        if (error) { return error }
        
        error = assert(device.isSmartTV, null, `device.isSmartTV`)
        if (error) { return error }
        
        error = assert(device.isTablet, null, `device.isTablet`)
        if (error) { return error }
        
        error = assert(device.isTouchscreen, null, `device.isTouchscreen`)
        if (error) { return error }

        error = assert(JSON.stringify(device), "{\"name\":\"Asus TeK\",\"brand\":\"Asus\",\"model\":\"TeK\",\"hardwareType\":null,\"isDesktop\":null,\"isGameConsole\":null,\"isMediaPlayer\":null,\"isMobile\":null,\"isSmartTV\":null,\"isTablet\":null,\"isTouchscreen\":null}", `JSON.stringify(device)`)
        if (error) { return error }

        return pass('ok')
    });
}
