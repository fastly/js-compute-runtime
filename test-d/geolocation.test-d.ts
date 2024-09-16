/// <reference path="../types/geolocation.d.ts" />
import { Geolocation, getGeolocationForIpAddress } from "fastly:geolocation";
import { expectType } from 'tsd';

expectType<(address: string)=>Geolocation | null>(getGeolocationForIpAddress)

const geo = {} as Geolocation
expectType<string | null>(geo.as_name)
expectType<number | null>(geo.as_number)
expectType<number | null>(geo.area_code)
expectType<string | null>(geo.city)
expectType<string | null>(geo.conn_speed)
expectType<string | null>(geo.conn_type)
expectType<string | null>(geo.continent)
expectType<string | null>(geo.country_code)
expectType<string | null>(geo.country_code3)
expectType<string | null>(geo.gmt_offset)
expectType<number | null>(geo.latitude)
expectType<number | null>(geo.longitude)
expectType<number | null>(geo.metro_code)
expectType<string | null>(geo.postal_code)
expectType<string | null>(geo.proxy_description)
expectType<string | null>(geo.proxy_type)
expectType<string | null>(geo.region)
expectType<number | null>(geo.utc_offset)