use base64;
use clap::{arg, Command};
use fastly_api::apis::configuration::{ApiKey, Configuration};
use fastly_api::apis::version_api::{
    activate_service_version, clone_service_version, list_service_versions,
    ActivateServiceVersionParams, CloneServiceVersionParams, ListServiceVersionsParams,
};
use futures::{stream, StreamExt};
use httpdate::fmt_http_date;
use reqwest::Client;
use sha2::{Digest, Sha256};
use simple_error::bail;
use std::error::Error;
use std::path::PathBuf;
use tokio;
use tokio::fs::File;
use toml_edit;
use walkdir::WalkDir;

const PARALLEL_REQUESTS: usize = 10;
const RETRY_REQUESTS: usize = 5;

use phf::phf_map;

static MIMES: phf::Map<&'static str, &'static str> = phf_map! {
  "ez"=> "application/andrew-inset",
  "aw"=> "application/applixware",
  "atom"=> "application/atom+xml",
  "atomcat"=> "application/atomcat+xml",
  "atomdeleted"=> "application/atomdeleted+xml",
  "atomsvc"=> "application/atomsvc+xml",
  "dwd"=> "application/atsc-dwd+xml",
  "held"=> "application/atsc-held+xml",
  "rsat"=> "application/atsc-rsat+xml",
  "bdoc"=> "application/bdoc",
  "xcs"=> "application/calendar+xml",
  "ccxml"=> "application/ccxml+xml",
  "cdfx"=> "application/cdfx+xml",
  "cdmia"=> "application/cdmi-capability",
  "cdmic"=> "application/cdmi-container",
  "cdmid"=> "application/cdmi-domain",
  "cdmio"=> "application/cdmi-object",
  "cdmiq"=> "application/cdmi-queue",
  "cu"=> "application/cu-seeme",
  "mpd"=> "application/dash+xml",
  "davmount"=> "application/davmount+xml",
  "dbk"=> "application/docbook+xml",
  "dssc"=> "application/dssc+der",
  "xdssc"=> "application/dssc+xml",
  "es"=> "application/ecmascript",
  "ecma"=> "application/ecmascript",
  "emma"=> "application/emma+xml",
  "emotionml"=> "application/emotionml+xml",
  "epub"=> "application/epub+zip",
  "exi"=> "application/exi",
  "fdt"=> "application/fdt+xml",
  "pfr"=> "application/font-tdpfr",
  "geojson"=> "application/geo+json",
  "gml"=> "application/gml+xml",
  "gpx"=> "application/gpx+xml",
  "gxf"=> "application/gxf",
  "gz"=> "application/gzip",
  "hjson"=> "application/hjson",
  "stk"=> "application/hyperstudio",
  "ink"=> "application/inkml+xml",
  "inkml"=> "application/inkml+xml",
  "ipfix"=> "application/ipfix",
  "its"=> "application/its+xml",
  "jar"=> "application/java-archive",
  "war"=> "application/java-archive",
  "ear"=> "application/java-archive",
  "ser"=> "application/java-serialized-object",
  "class"=> "application/java-vm",
  "js"=> "application/javascript",
  "mjs"=> "application/javascript",
  "json"=> "application/json",
  "map"=> "application/json",
  "json5"=> "application/json5",
  "jsonml"=> "application/jsonml+json",
  "jsonld"=> "application/ld+json",
  "lgr"=> "application/lgr+xml",
  "lostxml"=> "application/lost+xml",
  "hqx"=> "application/mac-binhex40",
  "cpt"=> "application/mac-compactpro",
  "mads"=> "application/mads+xml",
  "webmanifest"=> "application/manifest+json",
  "mrc"=> "application/marc",
  "mrcx"=> "application/marcxml+xml",
  "ma"=> "application/mathematica",
  "nb"=> "application/mathematica",
  "mb"=> "application/mathematica",
  "mathml"=> "application/mathml+xml",
  "mbox"=> "application/mbox",
  "mscml"=> "application/mediaservercontrol+xml",
  "metalink"=> "application/metalink+xml",
  "meta4"=> "application/metalink4+xml",
  "mets"=> "application/mets+xml",
  "maei"=> "application/mmt-aei+xml",
  "musd"=> "application/mmt-usd+xml",
  "mods"=> "application/mods+xml",
  "m21"=> "application/mp21",
  "mp21"=> "application/mp21",
  "mp4s"=> "application/mp4",
  "m4p"=> "application/mp4",
  "doc"=> "application/msword",
  "dot"=> "application/msword",
  "mxf"=> "application/mxf",
  "nq"=> "application/n-quads",
  "nt"=> "application/n-triples",
  "cjs"=> "application/node",
  "bin"=> "application/octet-stream",
  "dms"=> "application/octet-stream",
  "lrf"=> "application/octet-stream",
  "mar"=> "application/octet-stream",
  "so"=> "application/octet-stream",
  "dist"=> "application/octet-stream",
  "distz"=> "application/octet-stream",
  "pkg"=> "application/octet-stream",
  "bpk"=> "application/octet-stream",
  "dump"=> "application/octet-stream",
  "elc"=> "application/octet-stream",
  "deploy"=> "application/octet-stream",
  "exe"=> "application/octet-stream",
  "dll"=> "application/octet-stream",
  "deb"=> "application/octet-stream",
  "dmg"=> "application/octet-stream",
  "iso"=> "application/octet-stream",
  "img"=> "application/octet-stream",
  "msi"=> "application/octet-stream",
  "msp"=> "application/octet-stream",
  "msm"=> "application/octet-stream",
  "buffer"=> "application/octet-stream",
  "oda"=> "application/oda",
  "opf"=> "application/oebps-package+xml",
  "ogx"=> "application/ogg",
  "omdoc"=> "application/omdoc+xml",
  "onetoc"=> "application/onenote",
  "onetoc2"=> "application/onenote",
  "onetmp"=> "application/onenote",
  "onepkg"=> "application/onenote",
  "oxps"=> "application/oxps",
  "relo"=> "application/p2p-overlay+xml",
  "xer"=> "application/patch-ops-error+xml",
  "pdf"=> "application/pdf",
  "pgp"=> "application/pgp-encrypted",
  "asc"=> "application/pgp-signature",
  "sig"=> "application/pgp-signature",
  "prf"=> "application/pics-rules",
  "p10"=> "application/pkcs10",
  "p7m"=> "application/pkcs7-mime",
  "p7c"=> "application/pkcs7-mime",
  "p7s"=> "application/pkcs7-signature",
  "p8"=> "application/pkcs8",
  "ac"=> "application/pkix-attr-cert",
  "cer"=> "application/pkix-cert",
  "crl"=> "application/pkix-crl",
  "pkipath"=> "application/pkix-pkipath",
  "pki"=> "application/pkixcmp",
  "pls"=> "application/pls+xml",
  "ai"=> "application/postscript",
  "eps"=> "application/postscript",
  "ps"=> "application/postscript",
  "provx"=> "application/provenance+xml",
  "cww"=> "application/prs.cww",
  "pskcxml"=> "application/pskc+xml",
  "raml"=> "application/raml+yaml",
  "rdf"=> "application/rdf+xml",
  "owl"=> "application/rdf+xml",
  "rif"=> "application/reginfo+xml",
  "rnc"=> "application/relax-ng-compact-syntax",
  "rl"=> "application/resource-lists+xml",
  "rld"=> "application/resource-lists-diff+xml",
  "rs"=> "application/rls-services+xml",
  "rapd"=> "application/route-apd+xml",
  "sls"=> "application/route-s-tsid+xml",
  "rusd"=> "application/route-usd+xml",
  "gbr"=> "application/rpki-ghostbusters",
  "mft"=> "application/rpki-manifest",
  "roa"=> "application/rpki-roa",
  "rsd"=> "application/rsd+xml",
  "rss"=> "application/rss+xml",
  "rtf"=> "application/rtf",
  "sbml"=> "application/sbml+xml",
  "scq"=> "application/scvp-cv-request",
  "scs"=> "application/scvp-cv-response",
  "spq"=> "application/scvp-vp-request",
  "spp"=> "application/scvp-vp-response",
  "sdp"=> "application/sdp",
  "senmlx"=> "application/senml+xml",
  "sensmlx"=> "application/sensml+xml",
  "setpay"=> "application/set-payment-initiation",
  "setreg"=> "application/set-registration-initiation",
  "shf"=> "application/shf+xml",
  "siv"=> "application/sieve",
  "sieve"=> "application/sieve",
  "smi"=> "application/smil+xml",
  "smil"=> "application/smil+xml",
  "rq"=> "application/sparql-query",
  "srx"=> "application/sparql-results+xml",
  "gram"=> "application/srgs",
  "grxml"=> "application/srgs+xml",
  "sru"=> "application/sru+xml",
  "ssdl"=> "application/ssdl+xml",
  "ssml"=> "application/ssml+xml",
  "swidtag"=> "application/swid+xml",
  "tei"=> "application/tei+xml",
  "teicorpus"=> "application/tei+xml",
  "tfi"=> "application/thraud+xml",
  "tsd"=> "application/timestamped-data",
  "toml"=> "application/toml",
  "trig"=> "application/trig",
  "ttml"=> "application/ttml+xml",
  "ubj"=> "application/ubjson",
  "rsheet"=> "application/urc-ressheet+xml",
  "td"=> "application/urc-targetdesc+xml",
  "vxml"=> "application/voicexml+xml",
  "wasm"=> "application/wasm",
  "wgt"=> "application/widget",
  "hlp"=> "application/winhlp",
  "wsdl"=> "application/wsdl+xml",
  "wspolicy"=> "application/wspolicy+xml",
  "xaml"=> "application/xaml+xml",
  "xav"=> "application/xcap-att+xml",
  "xca"=> "application/xcap-caps+xml",
  "xdf"=> "application/xcap-diff+xml",
  "xel"=> "application/xcap-el+xml",
  "xns"=> "application/xcap-ns+xml",
  "xenc"=> "application/xenc+xml",
  "xhtml"=> "application/xhtml+xml",
  "xht"=> "application/xhtml+xml",
  "xlf"=> "application/xliff+xml",
  "xml"=> "application/xml",
  "xsl"=> "application/xml",
  "xsd"=> "application/xml",
  "rng"=> "application/xml",
  "dtd"=> "application/xml-dtd",
  "xop"=> "application/xop+xml",
  "xpl"=> "application/xproc+xml",
  "xslt"=> "application/xml",
  "xspf"=> "application/xspf+xml",
  "mxml"=> "application/xv+xml",
  "xhvml"=> "application/xv+xml",
  "xvml"=> "application/xv+xml",
  "xvm"=> "application/xv+xml",
  "yang"=> "application/yang",
  "yin"=> "application/yin+xml",
  "zip"=> "application/zip",
  "3gpp"=> "video/3gpp",
  "adp"=> "audio/adpcm",
  "amr"=> "audio/amr",
  "au"=> "audio/basic",
  "snd"=> "audio/basic",
  "mid"=> "audio/midi",
  "midi"=> "audio/midi",
  "kar"=> "audio/midi",
  "rmi"=> "audio/midi",
  "mxmf"=> "audio/mobile-xmf",
  "mp3"=> "audio/mpeg",
  "m4a"=> "audio/mp4",
  "mp4a"=> "audio/mp4",
  "mpga"=> "audio/mpeg",
  "mp2"=> "audio/mpeg",
  "mp2a"=> "audio/mpeg",
  "m2a"=> "audio/mpeg",
  "m3a"=> "audio/mpeg",
  "oga"=> "audio/ogg",
  "ogg"=> "audio/ogg",
  "spx"=> "audio/ogg",
  "opus"=> "audio/ogg",
  "s3m"=> "audio/s3m",
  "sil"=> "audio/silk",
  "wav"=> "audio/wav",
  "weba"=> "audio/webm",
  "xm"=> "audio/xm",
  "ttc"=> "font/collection",
  "otf"=> "font/otf",
  "ttf"=> "font/ttf",
  "woff"=> "font/woff",
  "woff2"=> "font/woff2",
  "exr"=> "image/aces",
  "apng"=> "image/apng",
  "avif"=> "image/avif",
  "bmp"=> "image/bmp",
  "cgm"=> "image/cgm",
  "drle"=> "image/dicom-rle",
  "emf"=> "image/emf",
  "fits"=> "image/fits",
  "g3"=> "image/g3fax",
  "gif"=> "image/gif",
  "heic"=> "image/heic",
  "heics"=> "image/heic-sequence",
  "heif"=> "image/heif",
  "heifs"=> "image/heif-sequence",
  "hej2"=> "image/hej2k",
  "hsj2"=> "image/hsj2",
  "ief"=> "image/ief",
  "jls"=> "image/jls",
  "jp2"=> "image/jp2",
  "jpg2"=> "image/jp2",
  "jpeg"=> "image/jpeg",
  "jpg"=> "image/jpeg",
  "jpe"=> "image/jpeg",
  "jph"=> "image/jph",
  "jhc"=> "image/jphc",
  "jpm"=> "image/jpm",
  "jpx"=> "image/jpx",
  "jpf"=> "image/jpx",
  "jxr"=> "image/jxr",
  "jxra"=> "image/jxra",
  "jxrs"=> "image/jxrs",
  "jxs"=> "image/jxs",
  "jxsc"=> "image/jxsc",
  "jxsi"=> "image/jxsi",
  "jxss"=> "image/jxss",
  "ktx"=> "image/ktx",
  "ktx2"=> "image/ktx2",
  "png"=> "image/png",
  "btif"=> "image/prs.btif",
  "pti"=> "image/prs.pti",
  "sgi"=> "image/sgi",
  "svg"=> "image/svg+xml",
  "svgz"=> "image/svg+xml",
  "t38"=> "image/t38",
  "tif"=> "image/tiff",
  "tiff"=> "image/tiff",
  "tfx"=> "image/tiff-fx",
  "webp"=> "image/webp",
  "wmf"=> "image/wmf",
  "disposition-notification"=> "message/disposition-notification",
  "u8msg"=> "message/global",
  "u8dsn"=> "message/global-delivery-status",
  "u8mdn"=> "message/global-disposition-notification",
  "u8hdr"=> "message/global-headers",
  "eml"=> "message/rfc822",
  "mime"=> "message/rfc822",
  "3mf"=> "model/3mf",
  "gltf"=> "model/gltf+json",
  "glb"=> "model/gltf-binary",
  "igs"=> "model/iges",
  "iges"=> "model/iges",
  "msh"=> "model/mesh",
  "mesh"=> "model/mesh",
  "silo"=> "model/mesh",
  "mtl"=> "model/mtl",
  "obj"=> "model/obj",
  "stpz"=> "model/step+zip",
  "stpxz"=> "model/step-xml+zip",
  "stl"=> "model/stl",
  "wrl"=> "model/vrml",
  "vrml"=> "model/vrml",
  "x3db"=> "model/x3d+fastinfoset",
  "x3dbz"=> "model/x3d+binary",
  "x3dv"=> "model/x3d-vrml",
  "x3dvz"=> "model/x3d+vrml",
  "x3d"=> "model/x3d+xml",
  "x3dz"=> "model/x3d+xml",
  "appcache"=> "text/cache-manifest",
  "manifest"=> "text/cache-manifest",
  "ics"=> "text/calendar",
  "ifb"=> "text/calendar",
  "coffee"=> "text/coffeescript",
  "litcoffee"=> "text/coffeescript",
  "css"=> "text/css",
  "csv"=> "text/csv",
  "html"=> "text/html",
  "htm"=> "text/html",
  "shtml"=> "text/html",
  "jade"=> "text/jade",
  "jsx"=> "text/jsx",
  "less"=> "text/less",
  "markdown"=> "text/markdown",
  "md"=> "text/markdown",
  "mml"=> "text/mathml",
  "mdx"=> "text/mdx",
  "n3"=> "text/n3",
  "txt"=> "text/plain",
  "text"=> "text/plain",
  "conf"=> "text/plain",
  "def"=> "text/plain",
  "list"=> "text/plain",
  "log"=> "text/plain",
  "in"=> "text/plain",
  "ini"=> "text/plain",
  "dsc"=> "text/prs.lines.tag",
  "rtx"=> "text/richtext",
  "sgml"=> "text/sgml",
  "sgm"=> "text/sgml",
  "shex"=> "text/shex",
  "slim"=> "text/slim",
  "slm"=> "text/slim",
  "spdx"=> "text/spdx",
  "stylus"=> "text/stylus",
  "styl"=> "text/stylus",
  "tsv"=> "text/tab-separated-values",
  "t"=> "text/troff",
  "tr"=> "text/troff",
  "roff"=> "text/troff",
  "man"=> "text/troff",
  "me"=> "text/troff",
  "ms"=> "text/troff",
  "ttl"=> "text/turtle",
  "uri"=> "text/uri-list",
  "uris"=> "text/uri-list",
  "urls"=> "text/uri-list",
  "vcard"=> "text/vcard",
  "vtt"=> "text/vtt",
  "yaml"=> "text/yaml",
  "yml"=> "text/yaml",
  "3gp"=> "video/3gpp",
  "3g2"=> "video/3gpp2",
  "h261"=> "video/h261",
  "h263"=> "video/h263",
  "h264"=> "video/h264",
  "m4s"=> "video/iso.segment",
  "jpgv"=> "video/jpeg",
  "jpgm"=> "image/jpm",
  "mj2"=> "video/mj2",
  "mjp2"=> "video/mj2",
  "ts"=> "video/mp2t",
  "mp4"=> "video/mp4",
  "mp4v"=> "video/mp4",
  "mpg4"=> "video/mp4",
  "mpeg"=> "video/mpeg",
  "mpg"=> "video/mpeg",
  "mpe"=> "video/mpeg",
  "m1v"=> "video/mpeg",
  "m2v"=> "video/mpeg",
  "ogv"=> "video/ogg",
  "qt"=> "video/quicktime",
  "mov"=> "video/quicktime",
  "webm"=> "video/webm"
};

fn lookup(extn: &str) -> Option<&&str> {
    let extn = extn.trim().to_lowercase();
    MIMES.get(&extn)
}

use serde_derive::Deserialize;
use serde_derive::Serialize;

#[derive(Default, Debug, Clone, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Metadata {
    #[serde(rename = "ETag")]
    etag: String,
    #[serde(rename = "Last-Modified")]
    last_modified: String,
    #[serde(rename = "Content-Type")]
    content_type: Option<String>,
}

#[derive(Default, Debug, Clone, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct KVStores {
    data: Vec<KVStore>,
    meta: Meta,
}

#[derive(Default, Debug, Clone, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct KVStore {
    id: String,
    name: String,
    #[serde(rename = "created_at")]
    created_at: String,
    #[serde(rename = "updated_at")]
    updated_at: String,
}

#[derive(Default, Debug, Clone, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Meta {
    limit: i64,
    total: i64,
}

async fn create_store(name: &str, token: &str) -> Result<String, Box<dyn std::error::Error>> {
    let client = reqwest::Client::new();
    let res = client
        .post("https://api.fastly.com/resources/stores/kv")
        .header("Content-Type", "application/json")
        .header("Accept", "application/json")
        .header("Fastly-Key", token)
        .body(format!("{{\"name\":\"{}\"}}", name))
        .send()
        .await?;
    if res.status() == 201 {
        Ok(res.json::<KVStore>().await?.id)
    } else {
        bail!(format!(
            "Failed to create Object Store named `{}`. Response body contained `{}`",
            name,
            res.text().await?
        ))
    }
}

async fn get_or_create_store(
    name: &str,
    token: &str,
) -> Result<String, Box<dyn std::error::Error>> {
    // get all stores
    let client = reqwest::Client::new();
    let res = client
        .get("https://api.fastly.com/resources/stores/kv")
        .header("Content-Type", "application/json")
        .header("Accept", "application/json")
        .header("Fastly-Key", token)
        .send()
        .await?;
    // if no stores at all, create store
    if res.status() == 404 {
        create_store(name, token).await
    } else {
        // check if store already exists
        let res = res
            .json::<KVStores>()
            .await?
            .data
            .into_iter()
            .find_map(|store| {
                if store.name == name {
                    Some(store.id)
                } else {
                    None
                }
            });
        // if store does not exist, create store
        if res.is_none() {
            create_store(name, token).await
        } else {
            Ok(res.unwrap())
        }
    }
}

async fn get_active_version_of_service(
    service_id: &str,
    token: &str,
) -> Result<i32, Box<dyn std::error::Error>> {
    let cfg = &Configuration {
        api_key: Some(ApiKey {
            prefix: None,
            key: token.to_owned(),
        }),
        ..Default::default()
    };

    let params = ListServiceVersionsParams {
        service_id: service_id.to_owned(),
        ..Default::default()
    };

    let result = list_service_versions(cfg, params)
        .await?
        .into_iter()
        .find_map(|v| {
            if v.active.unwrap() {
                Some(v.number.unwrap())
            } else {
                None
            }
        });

    return Ok(result.expect("Service should have an active version to clone"));
}

async fn clone_version_of_service(
    service_id: &str,
    token: &str,
    version: i32,
) -> Result<i32, Box<dyn std::error::Error>> {
    let cfg = &Configuration {
        api_key: Some(ApiKey {
            prefix: None,
            key: token.to_owned(),
        }),
        ..Default::default()
    };

    let params = CloneServiceVersionParams {
        service_id: service_id.to_owned(),
        version_id: version,
        ..Default::default()
    };

    Ok(clone_service_version(cfg, params).await?.number.unwrap())
}

async fn activate_version_of_service(
    service_id: &str,
    token: &str,
    version: i32,
) -> Result<i32, Box<dyn std::error::Error>> {
    let cfg = &Configuration {
        api_key: Some(ApiKey {
            prefix: None,
            key: token.to_owned(),
        }),
        ..Default::default()
    };

    let params = ActivateServiceVersionParams {
        service_id: service_id.to_string(),
        version_id: version,
        ..Default::default()
    };

    Ok(activate_service_version(cfg, params).await?.number.unwrap())
}

fn cli() -> Command {
    Command::new("fastly-file-server")
        .about("Fastly File Server uploads files to Fastly for serving directly from within Fastly Compute applications. Upload any type of file: images, text, video etc and serve directly from Fastly. It is ideal for serving files built from a static site generator such as 11ty.")
        .subcommand_required(true)
        .arg_required_else_help(true)
        .subcommand(
            Command::new("upload")
                .about("Upload files")
                .arg(
                    arg!(path: [PATH])
                        .last(true)
                        .required(true)
                        .value_parser(clap::value_parser!(PathBuf)),
                )
                .arg_required_else_help(true)
                .arg(arg!(--name <NAME>).required(true))
                .arg(arg!(--token <TOKEN>)),
        )
        .subcommand(
            Command::new("local")
                .about("Setup files")
                .arg(
                    arg!(path: [PATH])
                        .last(true)
                        .required(true)
                        .value_parser(clap::value_parser!(PathBuf)),
                )
                .arg_required_else_help(true)
                .arg(arg!(--toml <TOML>).required(true).value_parser(clap::value_parser!(PathBuf)))
                .arg(arg!(--name <NAME>).required(true)),
        )
        .subcommand(
            Command::new("link")
                .about("link store to service")
                .arg(arg!(--name <NAME>).required(true))
                .arg(arg!(--token <TOKEN>))
                .arg(arg!(--"link-name" <LINK_NAME>).required(true))
                .arg(arg!(--"service-id" <SERVICE_ID>).required(true)),
        )
        .subcommand(
            Command::new("unlink")
                .about("unlink store to service")
                .arg(arg!(--name <NAME>).required(true))
                .arg(arg!(--token <TOKEN>))
                .arg(arg!(--"link-name" <LINK_NAME>).required(true))
                .arg(arg!(--"service-id" <SERVICE_ID>).required(true)),
        )
}

async fn link(sub_matches: &clap::ArgMatches) -> Result<(), Box<dyn std::error::Error>> {
    let service_id = sub_matches
        .get_one::<String>("service-id")
        .map(|s| s.as_str())
        .expect("required in clap");

    let link_name = sub_matches
        .get_one::<String>("link-name")
        .map(|s| s.as_str())
        .expect("required in clap");

    let name = sub_matches
        .get_one::<String>("name")
        .map(|s| s.as_str())
        .expect("required in clap");

    let token = sub_matches
        .get_one::<String>("token")
        .map(|s| s.to_owned())
        .or_else(|| match std::env::var("FASTLY_API_TOKEN") {
            Ok(x) => Some(x),
            Err(_) => None,
        });
    if token.is_none() {
        bail!("Missing Fastly API token. Please provide an API token via the --token argument or the FASTLY_API_TOKEN environment variable.")
    }
    let token = token.unwrap();

    let store_id = get_or_create_store(name, &token).await?;

    let version = get_active_version_of_service(service_id, &token).await?;
    let version = clone_version_of_service(service_id, &token, version).await?;

    // link
    let client = reqwest::Client::new();
    let _res = client
        .post(format!(
            "https://api.fastly.com/service/{}/version/{}/resource",
            service_id, version
        ))
        .header("Content-Type", "application/x-www-form-urlencoded")
        .header("Accept", "application/json")
        .header("Fastly-Key", &token)
        .body(format!("name={}&resource_id={}", link_name, store_id))
        .send()
        .await?;

    // activate
    activate_version_of_service(service_id, &token, version).await?;

    Ok(())
}

async fn upload(sub_matches: &clap::ArgMatches) -> Result<(), Box<dyn std::error::Error>> {
    let name = sub_matches
        .get_one::<String>("name")
        .map(|s| s.as_str())
        .expect("required in clap");

    let token = sub_matches
        .get_one::<String>("token")
        .map(|s| s.to_owned())
        .or_else(|| match std::env::var("FASTLY_API_TOKEN") {
            Ok(x) => Some(x),
            Err(_) => None,
        });
    if token.is_none() {
        bail!("Missing Fastly API token. Please provide an API token via the --token argument or the FASTLY_API_TOKEN environment variable.")
    }
    let token = token.unwrap();
    let store_id = get_or_create_store(name, &token).await?;

    let path = sub_matches
        .get_one::<PathBuf>("path")
        .expect("required in clap");

    let entries = WalkDir::new(path).follow_links(true)
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| !e.file_type().is_dir())
        .collect::<Vec<walkdir::DirEntry>>();

    let pb = indicatif::ProgressBar::new(entries.len().try_into().unwrap());
    let client = Client::new();

    let bodies = stream::iter(entries)
        .map(|entry| -> tokio::task::JoinHandle<Result<String, Box<dyn Error + Send + Sync>>> {
            let path = path.clone();
            let store_id = store_id.clone();
            let token = token.clone();
            let client = client.clone();
            tokio::spawn(async move {
                let extension = entry.path().extension().map(|e| e.to_string_lossy().to_string()).unwrap_or("".to_string());
                let normalised_entry = entry.path().strip_prefix(path).unwrap();
                let normalised_path = "/".to_owned() + &normalised_entry.to_string_lossy();
                let key = percent_encoding::utf8_percent_encode(
                    &normalised_path,
                    percent_encoding::NON_ALPHANUMERIC,
                );
                let metadata_key = normalised_path.to_owned() + "__metadata__";
                let metadata_key = percent_encoding::utf8_percent_encode(
                    &metadata_key,
                    percent_encoding::NON_ALPHANUMERIC,
                );
                let file_contents = tokio::fs::read(entry.path()).await?;
                let file = File::open(entry.path()).await?;
                let file_metadata = file.metadata().await?;
                let length = file.metadata().await?.len();
                let mut counter = 0;
                let sha = Sha256::digest(file_contents);
                let sha = base64::encode(&sha);
                let metadata = serde_json::to_string(&Metadata {
                    etag: format!("W/\"{}\"", sha),
                    last_modified: fmt_http_date(file_metadata.modified()?),
                    content_type: lookup(&extension).map(|content_type| content_type.to_string())
                })?;

                loop {
                    let res = client
                        .put(format!(
                            "https://api.fastly.com/resources/stores/kv/{}/keys/{}",
                            store_id, metadata_key
                        ))
                        .header("Content-Type", "application/json")
                        .header("Content-Length", metadata.len().to_string())
                        .header("Accept", "application/json")
                        .header("Fastly-Key", &token)
                        .body(metadata.clone())
                        .send()
                        .await?;
                    if res.status() != 200 {
                        counter = counter + 1;
                        if counter > RETRY_REQUESTS {
                            bail!(
                                "Error uploading metadata for file named `{}`: Response Status: {} Response Body: {}",
                                normalised_path,
                                res.status(),
                                res.text().await?
                            );
                        }
                    } else {
                        break;
                    }
                }
                let mut counter = 0;
                loop {
                    let res = client
                        .put(format!(
                            "https://api.fastly.com/resources/stores/kv/{}/keys/{}",
                            store_id, key
                        ))
                        .header("Content-Type", "application/json")
                        .header("Content-Length", length)
                        .header("Accept", "application/json")
                        .header("Fastly-Key", &token)
                        .body(file.try_clone().await?)
                        .send()
                        .await?;
                    if res.status() != 200 {
                        counter = counter + 1;
                        if counter > RETRY_REQUESTS {
                            bail!(
                                "Error uploading file named `{}`: Response Status: {} Response Body: {}",
                                normalised_path,
                                res.status(),
                                res.text().await?
                            );
                        }
                    } else {
                        return Ok::<String, Box<dyn std::error::Error + Send + Sync>>(
                            normalised_path,
                        );
                    }
                }
            })
        })
        .buffer_unordered(PARALLEL_REQUESTS);

    bodies
        .for_each(|b| async {
            match b {
                Ok(Ok(normalised_entry)) => {
                    pb.println(format!("[+] uploaded {}", normalised_entry));
                    pb.inc(1);
                }
                Ok(Err(e)) => eprintln!("Got a reqwest::Error: {}", e),
                Err(e) => eprintln!("Got a tokio::JoinError: {}", e),
            }
        })
        .await;

    pb.finish_with_message("done");
    Ok(())
}

async fn local(sub_matches: &clap::ArgMatches) -> Result<(), Box<dyn std::error::Error>> {
    let name = sub_matches
        .get_one::<String>("name")
        .map(|s| s.as_str())
        .expect("required in clap");

    let path = sub_matches
        .get_one::<PathBuf>("path")
        .expect("required in clap");

    let toml_path = sub_matches
        .get_one::<PathBuf>("toml")
        .expect("required in clap");

    let entries = WalkDir::new(path).follow_links(true)
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| !e.file_type().is_dir())
        .collect::<Vec<walkdir::DirEntry>>();

    let mut toml = std::fs::read_to_string(toml_path)?.parse::<toml_edit::Document>()?;
    let mut local_server = toml
        .get_key_value("local_server")
        .map(|a| a.1.to_owned())
        .unwrap_or_else(|| toml_edit::table());
    let mut object_store = local_server
        .as_table_mut()
        .unwrap()
        .get_key_value(&format!("object_store"))
        .map(|a| a.1.to_owned())
        .unwrap_or_else(|| toml_edit::table());

    let mut site = toml_edit::array();
    for entry in entries {
        let path = path.clone();
        let entry_path = entry.path().to_string_lossy().to_string();
        let extension = entry
            .path()
            .extension()
            .map(|e| e.to_string_lossy().to_string())
            .unwrap_or("".to_string());
        let normalised_entry = entry.path().strip_prefix(path).unwrap();
        let normalised_path = "/".to_owned() + &normalised_entry.to_string_lossy();
        let key = &normalised_path;
        let metadata_key = normalised_path.to_owned() + "__metadata__";
        let file_contents = tokio::fs::read(entry.path()).await?;
        let file = File::open(entry.path()).await?;
        let file_metadata = file.metadata().await?;
        let sha = Sha256::digest(file_contents);
        let sha = base64::encode(&sha);
        let metadata = serde_json::to_string(&Metadata {
            etag: format!("W/\"{}\"", sha),
            last_modified: fmt_http_date(file_metadata.modified()?),
            content_type: lookup(&extension).map(|content_type| content_type.to_string()),
        })?;
        let mut entry = toml_edit::table();
        entry
            .as_table_mut()
            .unwrap()
            .insert("key", toml_edit::value(metadata_key));
        entry
            .as_table_mut()
            .unwrap()
            .insert("data", toml_edit::value(metadata.clone()));
        site.as_array_of_tables_mut()
            .unwrap()
            .push(entry.as_table().unwrap().to_owned());
        let mut entry = toml_edit::table();
        entry
            .as_table_mut()
            .unwrap()
            .insert("key", toml_edit::value(key));
        entry
            .as_table_mut()
            .unwrap()
            .insert("path", toml_edit::value(entry_path));
        site.as_array_of_tables_mut()
            .unwrap()
            .push(entry.as_table().unwrap().to_owned());
    }
    object_store.as_table_mut().unwrap().insert(name, site);
    local_server
        .as_table_mut()
        .unwrap()
        .insert(&"object_store", object_store);
    toml.as_table_mut().insert("local_server", local_server);
    std::fs::write(toml_path, toml.to_string())?;

    Ok(())
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("link", sub_matches)) => link(sub_matches).await,
        Some(("local", sub_matches)) => local(sub_matches).await,
        Some(("upload", sub_matches)) => upload(sub_matches).await,
        _ => unreachable!(),
    }
}
