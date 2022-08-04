import {expectError, expectType} from 'tsd';
import { addEventListener, CacheOverride, CacheOverrideInit, CacheOverrideMode, ClientInfo, Dictionary, Env, FetchEvent, FetchEventListener, Geolocation, Request, Response, URL, URLSearchParams } from ".";

// addEventListener
{
    // Both parameters are required:
    //  - The first parameter _has_ to be 'fetch'
    //  - The second parameter _has_ to be a function
    expectError(addEventListener('magic'));
    expectError(addEventListener('fetch'));
    expectError<void>(addEventListener('magic', () => {}));
    expectType<void>(addEventListener('fetch', () => {}));
    expectType<void>(addEventListener('fetch', (event) => {
        expectType<FetchEvent>(event)
    }));
}

// FetchEventListener
{
    let listener = (()=>{}) as FetchEventListener;
    expectType<(event: FetchEvent) => void>(listener)
}

// FetchEvent
{
    const event = {} as FetchEvent
    expectType<ClientInfo>(event.client)
    expectType<Request>(event.request)
    expectType<(response: Response | Promise<Response>) => void>(event.respondWith)
    expectType<(promise: Promise<any>) => void>(event.waitUntil)
}

// CacheOverride
{
    const cacheOverride = {} as CacheOverride
    expectType<CacheOverrideMode>(cacheOverride.mode)
    expectType<boolean>(cacheOverride.pci)
    expectType<number>(cacheOverride.ttl)
    expectType<number>(cacheOverride.swr)
    expectType<string>(cacheOverride.surrogateKey)
}

// CacheOverrideMode
{
    const cacheOverride = {} as CacheOverrideMode
    expectType<"none" | "pass" | "override">(cacheOverride)
}

// CacheOverrideInit
{
    const cacheOverrideInit = {} as CacheOverrideInit
    expectType<boolean>(cacheOverrideInit.pci)
    expectType<number>(cacheOverrideInit.ttl)
    expectType<number>(cacheOverrideInit.swr)
    expectType<string>(cacheOverrideInit.surrogateKey)
}

// ClientInfo
{
    const client = {} as ClientInfo
    expectType<string>(client.address)
    expectType<Geolocation>(client.geo)
}

// Dictionary
{
    expectType<Dictionary>(new Dictionary('example'))
    expectError(new Dictionary())
    expectError(Dictionary('example'))
    expectError(Dictionary())
    expectType<(key:string) => string>(new Dictionary('example').get)
}

// Env
{
    expectType<Env>(new Env())
    expectError(new Env('example'))
    expectError(Env())
    expectError(Env('example'))
    expectType<(key:string) => string>(new Env().get)
}

// Geolocation
{
    const geo = {} as Geolocation
    expectType<string|null>(geo.as_name)
    expectType<number|null>(geo.as_number)
    expectType<number|null>(geo.area_code)
    expectType<string|null>(geo.city)
    expectType<string|null>(geo.conn_speed)
    expectType<string|null>(geo.conn_type)
    expectType<string|null>(geo.continent)
    expectType<string|null>(geo.country_code)
    expectType<string|null>(geo.country_code3)
    expectType<string|null>(geo.gmt_offset)
    expectType<number|null>(geo.latitude)
    expectType<number|null>(geo.longitude)
    expectType<number|null>(geo.metro_code)
    expectType<string|null>(geo.postal_code)
    expectType<string|null>(geo.proxy_description)
    expectType<string|null>(geo.proxy_type)
    expectType<string|null>(geo.region)
    expectType<number|null>(geo.utc_offset)
}

// URL
{
    expectType<URL>(new URL('example'))
    expectType<URL>(new URL('example', 'base'))
    expectType<URL>(new URL('example', new URL('example')))
    expectError(new URL())
    expectError(URL('example'))
    expectError(URL('example', 'base'))
    expectError(URL('example', new URL('example')))
    expectError(URL())
    expectType<string>(new URL('').href)
    new URL('').href = 'example'
    expectError((new URL('').href = 7))

    expectType<string>(new URL('').origin)

    expectType<string>(new URL('').protocol)
    expectType<string>((new URL('').protocol = '7'))
    expectError((new URL('').protocol = 7))

    expectType<string>(new URL('').username)
    expectType<string>((new URL('').username = '7'))
    expectError((new URL('').username = 7))

    expectType<string>(new URL('').password)
    expectType<string>((new URL('').password = '7'))
    expectError((new URL('').password = 7))

    expectType<string>(new URL('').host)
    expectType<string>((new URL('').host = '7'))
    expectError((new URL('').host = 7))

    expectType<string>(new URL('').hostname)
    expectType<string>((new URL('').hostname = '7'))
    expectError((new URL('').hostname = 7))

    expectType<string>(new URL('').port)
    expectType<string>((new URL('').port = '7'))
    expectError((new URL('').port = 7))

    expectType<string>(new URL('').pathname)
    expectType<string>((new URL('').pathname = '7'))
    expectError((new URL('').pathname = 7))

    expectType<string>(new URL('').search)
    expectType<string>((new URL('').search = '7'))
    expectError((new URL('').search = 7))

    expectType<URLSearchParams>(new URL('').searchParams)

    expectType<string>(new URL('').hash)
    expectType<string>((new URL('').hash = '7'))
    expectError((new URL('').hash = 7))
}