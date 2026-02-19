use rasn::uper::de::DecodeError;

mod generated;

use generated::x509::*;

pub fn decode_spki(
    data: &CxxVector<u8>,
    out: Pin<&mut *mut SubjectPublicKeyInfo>,
    err: Pin<&mut CxxString>,
) -> bool {
    match rasn::der::decode(data.as_slice()) {
        Ok(spki) => {
            out.set(Box::into_raw(Box::new(spki)));
            true
        }
        Err(err) => {
            err.push_bytes(format!("{err}").as_ref());
            false
        }
    }
}

pub fn decode_pkcs8(
    data: &CxxVector<u8>,
    out: Pin<&mut *mut PrivateKeyInfo>,
    err: Pin<&mut CxxString>,
) -> bool {
    match rasn::der::decode(data.as_slice()) {
        Ok(pkcs8) => {
            out.set(Box::into_raw(Box::new(spki)));
            true
        }
        Err(err) => {
            err.push_bytes(format!("{err}").as_ref());
            false
        }
    }
}

impl SubjectPublicKeyInfo {
    pub fn is_rsa(&self) -> bool {
        self.algorithm.algorithm == *RSA_ENCRYPTION
    }
}

impl PrivateKeyInfo {
    pub fn is_rsa(&self) -> bool {
        self.private_key_algorithm.0.algorithm == *RSA_ENCRYPTION
    }
}

impl RSAPublicKey {
    pub fn details(&self, modulus: Pin<&mut CxxString>, exponent: Pin<&mut CxxString>) {
        modulus.push_bytes(format!("{}", self.modulus).as_ref());
        exponent.push_bytes(format!("{}", self.public_exponent).as_ref());
    }
}

#[cxx::bridge]
mod ffi {
    #[namespace = "fastly::sys::asn"]
    extern "Rust" {
        type SubjectPublicKeyInfo;
        fn is_rsa(&self) -> bool;
        fn decode_spki(
            data: &CxxVector<u8>,
            out: Pin<&mut *mut SubjectPublicKeyInfo>,
            err: Pin<&mut CxxString>,
        ) -> bool;
    }

    #[namespace = "fastly::sys::asn"]
    extern "Rust" {
        type PrivateKeyInfo;
        fn is_rsa(&self) -> bool;
        fn decode_pkcs8(
            data: &CxxVector<u8>,
            out: Pin<&mut *mut PrivateKeyInfo>,
            err: Pin<&mut CxxString>,
        ) -> bool;
    }

    #[namespace = "fastly::sys::asn"]
    extern "Rust" {
        type RSAPublicKey;
        fn details(&self, modulus: Pin<&mut CxxString>, exponent: Pin<&mut CxxString>);
    }
}
