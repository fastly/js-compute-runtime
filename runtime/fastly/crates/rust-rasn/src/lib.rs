use std::pin::Pin;

mod generated;

use generated::x509::*;

#[no_mangle]
pub unsafe extern "C" fn decode_spki(
    data: &SpecSlice,
    out: &mut *mut SubjectPublicKeyInfo,
    err: &mut *mut SpecString,
) -> bool {
    match rasn::der::decode(data.into()) {
        Ok(spki) => {
            *out = Box::into_raw(Box::new(spki));
            true
        }
        Err(e) => {
            *err = format!("{e}").into();
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn decode_pkcs8(
    data: &SpecSlice,
    out: &mut *mut PrivateKeyInfo,
    err: &mut *mut SpecString,
) -> bool {
    match rasn::der::decode(data.into()) {
        Ok(pkcs8) => {
            *out = Box::into_raw(Box::new(pkcs8));
            true
        }
        Err(e) => {
            *err = format!("{e}").into();
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn spki_is_rsa(spki: &SubjectPublicKeyInfo) -> bool {
    self.algorithm.algorithm == *RSA_ENCRYPTION
}

#[no_mangle]
pub unsafe extern "C" fn decode_spki(
    spki: &SubjectPublicKeyInfo,
    out: &mut *mut RSAPublicKey,
    err: &mut *mut SpecString,
) -> bool {
    match rasn::der::decode(spki.subject_public_key.as_raw_slice()) {
        Ok(pubkey) => {
            *out = Box::into_raw(Box::new(pubkey));
            true
        }
        Err(e) => {
            *err = format!("{e}").into();
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn pkcs8_is_rsa(spki: &PrivateKeyInfo) -> bool {
    self.private_key_algorithm.0.algorithm == *RSA_ENCRYPTION
}

#[no_mangle]
pub unsafe extern "C" fn decode_pkcs8(
    spki: &PrivateKeyInfo,
    out: &mut *mut RSAPrivateKey,
    err: &mut *mut SpecString,
) -> bool {
    match rasn::der::decode(self.private_key.0) {
        Ok(privkey) => {
            *out = Box::into_raw(Box::new(privkey));
            true
        }
        Err(e) => {
            *err = format!("{e}").into();
            false
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn rsa_pubkey_modulus(pubkey: &RSAPublicKey) -> SpecString {
    format!("{}", pubkey.modulus).into()
}

#[no_mangle]
pub unsafe extern "C" fn rsa_pubkey_exponent(pubkey: &RSAPublicKey) -> SpecString {
    format!("{}", pubkey.public_exponent).into()
}

#[no_mangle]
pub unsafe extern "C" fn rsa_privkey_modulus(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.modulus).into()
}

#[no_mangle]
pub unsafe extern "C" fn rsa_privkey_exponent(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.public_exponent).into()
}

#[no_mangle]
pub unsafe extern "C" fn rsa_privkey_private_exponent(privkey: &RSAPrivateKey) -> SpecString {
    format!("{}", privkey.private_exponent).into()
}

#[repr(C)]
pub struct CVec<T> {
    ptr: *mut T,
    len: usize,
    cap: usize,
}

impl<T> Drop for CVec<T> {
    fn drop(&mut self) {
        unsafe {
            self.ptr.drop_in_place();
        }
    }
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
        let spec = unsafe { slice::from_raw_parts(spec.data, spec.len) };
        String::from_utf8(spec.to_owned()).unwrap()
    }
}

impl<'a> From<&'a SpecString> for &'a str {
    fn from(spec: &SpecString) -> &str {
        let spec = unsafe { slice::from_raw_parts(spec.data, spec.len) };
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

/// This type exists to transfer &str-likes over FFI.
#[repr(C)]
pub struct SpecSlice<'a> {
    data: *const u8,
    len: usize,
    _marker: PhantomData<&'a [u8]>,
}

impl SpecSlice<'_> {
    fn new(data: *const u8, len: usize) -> Self {
        SpecSlice {
            data,
            len,
            _marker: PhantomData,
        }
    }
}

impl<'a> From<&SpecSlice<'a>> for &'a [u8] {
    fn from(spec: &SpecSlice<'a>) -> &'a [u8] {
        unsafe { slice::from_raw_parts(spec.data, spec.len) }
    }
}

impl<'a> From<&'a str> for SpecSlice<'a> {
    fn from(s: &'a str) -> SpecSlice<'a> {
        let ptr = if !s.is_empty() {
            s.as_ptr()
        } else {
            std::ptr::null()
        };
        SpecSlice::new(ptr, s.len())
    }
}

impl<'a> From<&SpecSlice<'a>> for &'a str {
    fn from(spec: &SpecSlice<'a>) -> &'a str {
        let spec = unsafe { slice::from_raw_parts(spec.data, spec.len) };
        std::str::from_utf8(spec).unwrap()
    }
}
