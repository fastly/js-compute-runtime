declare module "fastly:geolocation" {
  /**
   * Retrieve geolocation information about the given IP address.
   *
   * @param address The IPv4 or IPv6 address to query
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   */
  function getGeolocationForIpAddress(address: string): Geolocation;
  /**
   * [Fastly Geolocation](https://developer.fastly.com/reference/vcl/variables/geolocation/)
   * information about an IP address
   *
   * Can be retrieved for the incoming request's client IP address using the
   * {@linkcode "globals".ClientInfo#geo} accessor, and for arbitrary addresses using
   * {@linkcode "fastly:geolocation".getGeolocationForIpAddress}.
   */
  interface Geolocation {
    /**
     * The name of the organization associated with as_number.
     *
     * For example, fastly is the value given for IP addresses under AS-54113.
     */
    as_name: string | null;

    /**
     * [Autonomous system](https://en.wikipedia.org/wiki/Autonomous_system_(Internet)) (AS) number.
     */
    as_number: number | null;

    /**
     * The telephone area code associated with an IP address.
     *
     * These are only available for IP addresses in the United States, its territories, and Canada.
     */
    area_code: number | null;

    /**
     * City or town name.
     */
    city: string | null;

    /**
     * Connection speed.
     */
    conn_speed: string | null;

    /**
     * Connection type.
     */
    conn_type: string | null;

    /**
     * Continent.
     */
    continent: string | null;

    /**
     * A two-character [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) country code for the country associated with an IP address.
     *
     * The US country code is returned for IP addresses associated with overseas United States military bases.
     *
     * These values include subdivisions that are assigned their own country codes in ISO 3166-1. For example, subdivisions NO-21 and NO-22 are presented with the country code SJ for Svalbard and the Jan Mayen Islands.
     */
    country_code: string | null;

    /**
     * A three-character [ISO 3166-1 alpha-3](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-3) country code for the country associated with the IP address.
     *
     * The USA country code is returned for IP addresses associated with overseas United States military bases.
     */
    country_code3: string | null;

    /**
     * Country name.
     *
     * This field is the [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) English short name for a country.
     */
    country_name: string | null;

    /**
     * Time zone offset from Greenwich Mean Time (GMT) for `city`.
     */
    gmt_offset: string | null;

    /**
     * Latitude, in units of degrees from the equator.
     *
     * Values range from -90.0 to +90.0 inclusive, and are based on the [WGS 84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
     */
    latitude: number | null;

    /**
     * Longitude, in units of degrees from the [IERS Reference Meridian](https://en.wikipedia.org/wiki/IERS_Reference_Meridian).
     *
     * Values range from -180.0 to +180.0 inclusive, and are based on the [WGS 84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
     */
    longitude: number | null;

    /**
     * Metro code, representing designated market areas (DMAs) in the United States.
     */
    metro_code: number | null;

    /**
     * The postal code associated with the IP address.
     *
     * These are available for some IP addresses in Australia, Canada, France, Germany, Italy, Spain, Switzerland, the United Kingdom, and the United States.
     *
     * For Canadian postal codes, this is the first 3 characters. For the United Kingdom, this is the first 2-4 characters (outward code). For countries with alphanumeric postal codes, this field is a lowercase transliteration.
     */
    postal_code: string | null;

    /**
     * Client proxy description.
     */
    proxy_description: string | null;

    /**
     * Client proxy type.
     */
    proxy_type: string | null;

    /**
     * [ISO 3166-2](https://en.wikipedia.org/wiki/ISO_3166-2) country subdivision code.
     *
     * For countries with multiple levels of subdivision (for example, nations within the United Kingdom), this variable gives the more specific subdivision.
     *
     * This field can be None for countries that do not have ISO country subdivision codes. For example, None is given for IP addresses assigned to the Åland Islands (country code AX, illustrated below).
     */
    region: string | null;

    /**
     * Time zone offset from coordinated universal time (UTC) for `city`.
     */
    utc_offset: number | null;
  }
}
