use std::pin::Pin;

use cxx::{CxxString, CxxVector};

mod generated;

use generated::x509::*;

pub fn decode_spki(
    data: &CxxVector<u8>,
    mut out: Pin<&mut *mut SubjectPublicKeyInfo>,
    err: Pin<&mut CxxString>,
) -> bool {
    match rasn::der::decode(data.as_slice()) {
        Ok(spki) => {
            out.set(Box::into_raw(Box::new(spki)));
            true
        }
        Err(e) => {
            err.push_bytes(format!("{e}").as_ref());
            false
        }
    }
}

pub fn decode_pkcs8(
    data: &CxxVector<u8>,
    mut out: Pin<&mut *mut PrivateKeyInfo>,
    err: Pin<&mut CxxString>,
) -> bool {
    match rasn::der::decode(data.as_slice()) {
        Ok(pkcs8) => {
            out.set(Box::into_raw(Box::new(pkcs8)));
            true
        }
        Err(e) => {
            err.push_bytes(format!("{e}").as_ref());
            false
        }
    }
}

impl SubjectPublicKeyInfo {
    pub fn is_rsa(&self) -> bool {
        self.algorithm.algorithm == *RSA_ENCRYPTION
    }

    pub fn decode_public_key(
        &self,
        mut out: Pin<&mut *mut RSAPublicKey>,
        err: Pin<&mut CxxString>,
    ) -> bool {
        match rasn::der::decode(self.subject_public_key.as_raw_slice()) {
            Ok(pubkey) => {
                out.set(Box::into_raw(Box::new(pubkey)));
                true
            }
            Err(e) => {
                err.push_bytes(format!("{e}").as_ref());
                false
            }
        }
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
        fn decode_public_key(
            &self,
            mut out: Pin<&mut *mut RSAPublicKey>,
            err: Pin<&mut CxxString>,
        ) -> bool;
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
