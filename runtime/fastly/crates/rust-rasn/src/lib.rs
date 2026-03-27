use std::marker::PhantomData;

mod generated;

use generated::x509::*;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn decode_spki(
    data: &RSlice,
    out: &mut *mut SubjectPublicKeyInfo,
    err: *mut SpecString,
) -> bool {
    match rasn::der::decode(data.into()) {
        Ok(spki) => {
            *out = Box::into_raw(Box::new(spki));
            true
        }
        Err(e) => {
            unsafe { *err = format!("{e}").into() };
            false
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn free_spki(spki: *mut SubjectPublicKeyInfo) {
    let _box = unsafe { Box::from_raw(spki) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn decode_pkcs8(
    data: &RSlice,
    out: &mut *mut PrivateKeyInfo,
    err: *mut SpecString,
) -> bool {
    match rasn::der::decode(data.into()) {
        Ok(pkcs8) => {
            *out = Box::into_raw(Box::new(pkcs8));
            true
        }
        Err(e) => {
            unsafe { *err = format!("{e}").into() };
            false
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn free_pkcs8(pkcs8: *mut PrivateKeyInfo) {
    let _box = unsafe { Box::from_raw(pkcs8) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn spki_is_rsa(spki: &SubjectPublicKeyInfo) -> bool {
    spki.algorithm.algorithm == *RSA_ENCRYPTION
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn decode_public_key(
    spki: &SubjectPublicKeyInfo,
    out: &mut *mut RSAPublicKey,
    err: *mut SpecString,
) -> bool {
    match rasn::der::decode(spki.subject_public_key.as_raw_slice()) {
        Ok(pubkey) => {
            *out = Box::into_raw(Box::new(pubkey));
            true
        }
        Err(e) => {
            unsafe { *err = format!("{e}").into() };
            false
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn free_pubkey(pubkey: *mut RSAPublicKey) {
    let _box = unsafe { Box::from_raw(pubkey) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn pkcs8_is_rsa(pkcs8: &PrivateKeyInfo) -> bool {
    pkcs8.private_key_algorithm.0.algorithm == *RSA_ENCRYPTION
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn decode_private_key(
    pkcs8: &PrivateKeyInfo,
    out: &mut *mut RSAPrivateKey,
    err: *mut SpecString,
) -> bool {
    match rasn::der::decode(pkcs8.private_key.0.as_ref()) {
        Ok(privkey) => {
            *out = Box::into_raw(Box::new(privkey));
            true
        }
        Err(e) => {
            unsafe { *err = format!("{e}").into() };
            false
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn free_privkey(privkey: *mut RSAPrivateKey) {
    let _box = unsafe { Box::from_raw(privkey) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_pubkey_modulus(pubkey: &RSAPublicKey) -> SpecString {
    format!("{}", pubkey.modulus).into()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_pubkey_exponent(pubkey: &RSAPublicKey) -> SpecString {
    format!("{}", pubkey.public_exponent).into()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_privkey_modulus(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.modulus).into()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_privkey_exponent(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.public_exponent).into()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_privkey_private_exponent(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.private_exponent).into()
}

/// This type exists to transfer String-likes over FFI.
#[repr(C)]
pub struct SpecString {
    data: *mut u8,
    len: usize,
    cap: usize,
}

impl From<String> for SpecString {
    fn from(mut s: String) -> SpecString {
        let data = s.as_mut_ptr();
        let len = s.len();
        let cap = s.capacity();
        std::mem::forget(s);
        Self { data, len, cap }
    }
}

impl From<SpecString> for String {
    fn from(spec: SpecString) -> String {
        let spec = unsafe { std::slice::from_raw_parts(spec.data, spec.len) };
        String::from_utf8(spec.to_owned()).unwrap()
    }
}

impl<'a> From<&'a SpecString> for &'a str {
    fn from(spec: &SpecString) -> &str {
        let spec = unsafe { std::slice::from_raw_parts(spec.data, spec.len) };
        std::str::from_utf8(spec).unwrap()
    }
}

impl Drop for SpecString {
    fn drop(&mut self) {
        unsafe {
            self.data.drop_in_place();
        }
    }
}

/// This type exists to transfer slices over FFI.
#[repr(C)]
pub struct RSlice<'a> {
    data: *const u8,
    len: usize,
    _marker: PhantomData<&'a [u8]>,
}

impl RSlice<'_> {
    fn new(data: *const u8, len: usize) -> Self {
        RSlice {
            data,
            len,
            _marker: PhantomData,
        }
    }
}

impl<'a> From<&RSlice<'a>> for &'a [u8] {
    fn from(spec: &RSlice<'a>) -> &'a [u8] {
        unsafe { std::slice::from_raw_parts(spec.data, spec.len) }
    }
}

impl<'a> From<&'a str> for RSlice<'a> {
    fn from(s: &'a str) -> RSlice<'a> {
        let ptr = if !s.is_empty() {
            s.as_ptr()
        } else {
            std::ptr::null()
        };
        RSlice::new(ptr, s.len())
    }
}

impl<'a> From<&RSlice<'a>> for &'a str {
    fn from(spec: &RSlice<'a>) -> &'a str {
        let spec = unsafe { std::slice::from_raw_parts(spec.data, spec.len) };
        std::str::from_utf8(spec).unwrap()
    }
}
