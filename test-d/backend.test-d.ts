/// <reference path="../types/backend.d.ts" />
import { Backend, BackendConfiguration } from "fastly:backend";
import {expectError, expectType} from 'tsd';

// Backend
{
    expectError(Backend())
    expectError(new Backend())
    expectError(new Backend({name: 'eu'}))
    expectError(new Backend({target: 'www.example.com'}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com'}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', hostOverride: 'example.com', }))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', connectTimeout: 5000, }))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', firstByteTimeout: 5000, }))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', betweenBytesTimeout: 5000, }))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', useSSL: true,}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', dontPool: true,}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', tlsMinVersion: 1.2,}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', tlsMaxVersion: 1.2,}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', certificateHostname: 'example.com',}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', caCertificate: '',}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com', ciphers: 'DEFAULT',}))
    expectType<Backend>(new Backend({name: 'eu', target: 'www.example.com',  sniHostname: 'example.com',}))
    const backend = new Backend({name: 'eu', target: 'www.example.com'})
    expectType<string>(backend.toString())
}
