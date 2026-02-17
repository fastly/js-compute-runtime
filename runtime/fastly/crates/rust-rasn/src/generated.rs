#[allow(
    non_camel_case_types,
    non_snake_case,
    non_upper_case_globals,
    unused,
    clippy::too_many_arguments
)]
pub mod x509 {
    extern crate alloc;
    use core::borrow::Borrow;
    use rasn::prelude::*;
    use std::sync::LazyLock;
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct AlgorithmIdentifier {
        pub algorithm: ObjectIdentifier,
        pub parameters: Option<Any>,
    }
    impl AlgorithmIdentifier {
        pub fn new(algorithm: ObjectIdentifier, parameters: Option<Any>) -> Self {
            Self {
                algorithm,
                parameters,
            }
        }
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct Attribute {
        #[rasn(identifier = "type")]
        pub r_type: AttributeType,
        pub values: SetOf<AttributeValue>,
    }
    impl Attribute {
        pub fn new(r_type: AttributeType, values: SetOf<AttributeValue>) -> Self {
            Self { r_type, values }
        }
    }
    #[doc = " at least one value is required"]
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct AttributeType(pub ObjectIdentifier);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct AttributeValue(pub Any);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct Attributes(pub SetOf<Attribute>);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct Extension {
        #[rasn(identifier = "extnID")]
        pub extn_id: ObjectIdentifier,
        #[rasn(default = "extension_critical_default")]
        pub critical: bool,
        #[rasn(identifier = "extnValue")]
        pub extn_value: OctetString,
    }
    impl Extension {
        pub fn new(extn_id: ObjectIdentifier, critical: bool, extn_value: OctetString) -> Self {
            Self {
                extn_id,
                critical,
                extn_value,
            }
        }
    }
    fn extension_critical_default() -> bool {
        false
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate, size("1.."))]
    pub struct Extensions(pub SequenceOf<Extension>);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct OtherPrimeInfo {
        pub prime: Integer,
        pub exponent: Integer,
        pub coefficient: Integer,
    }
    impl OtherPrimeInfo {
        pub fn new(prime: Integer, exponent: Integer, coefficient: Integer) -> Self {
            Self {
                prime,
                exponent,
                coefficient,
            }
        }
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate, size("1.."))]
    pub struct OtherPrimeInfos(pub SequenceOf<OtherPrimeInfo>);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct PrivateKey(pub OctetString);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct PrivateKeyAlgorithmIdentifier(pub AlgorithmIdentifier);
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct PrivateKeyInfo {
        pub version: Version,
        #[rasn(identifier = "privateKeyAlgorithm")]
        pub private_key_algorithm: PrivateKeyAlgorithmIdentifier,
        #[rasn(identifier = "privateKey")]
        pub private_key: PrivateKey,
        #[rasn(tag(context, 0))]
        pub attributes: Option<Attributes>,
    }
    impl PrivateKeyInfo {
        pub fn new(
            version: Version,
            private_key_algorithm: PrivateKeyAlgorithmIdentifier,
            private_key: PrivateKey,
            attributes: Option<Attributes>,
        ) -> Self {
            Self {
                version,
                private_key_algorithm,
                private_key,
                attributes,
            }
        }
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct RSAPrivateKey {
        pub version: Version,
        pub modulus: Integer,
        #[rasn(identifier = "publicExponent")]
        pub public_exponent: Integer,
        #[rasn(identifier = "privateExponent")]
        pub private_exponent: Integer,
        pub prime1: Integer,
        pub prime2: Integer,
        pub exponent1: Integer,
        pub exponent2: Integer,
        pub coefficient: Integer,
        #[rasn(identifier = "otherPrimeInfos")]
        pub other_prime_infos: Option<OtherPrimeInfos>,
    }
    impl RSAPrivateKey {
        pub fn new(
            version: Version,
            modulus: Integer,
            public_exponent: Integer,
            private_exponent: Integer,
            prime1: Integer,
            prime2: Integer,
            exponent1: Integer,
            exponent2: Integer,
            coefficient: Integer,
            other_prime_infos: Option<OtherPrimeInfos>,
        ) -> Self {
            Self {
                version,
                modulus,
                public_exponent,
                private_exponent,
                prime1,
                prime2,
                exponent1,
                exponent2,
                coefficient,
                other_prime_infos,
            }
        }
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct RSAPublicKey {
        pub modulus: Integer,
        #[rasn(identifier = "publicExponent")]
        pub public_exponent: Integer,
    }
    impl RSAPublicKey {
        pub fn new(modulus: Integer, public_exponent: Integer) -> Self {
            Self {
                modulus,
                public_exponent,
            }
        }
    }
    #[doc = " DEFINED BY AttributeType"]
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    pub struct SubjectPublicKeyInfo {
        pub algorithm: AlgorithmIdentifier,
        #[rasn(identifier = "subjectPublicKey")]
        pub subject_public_key: BitString,
    }
    impl SubjectPublicKeyInfo {
        pub fn new(algorithm: AlgorithmIdentifier, subject_public_key: BitString) -> Self {
            Self {
                algorithm,
                subject_public_key,
            }
        }
    }
    #[derive(AsnType, Debug, Clone, Decode, Encode, PartialEq, Eq, Hash)]
    #[rasn(delegate)]
    pub struct Version(pub Integer);
    pub static PKCS_1: LazyLock<ObjectIdentifier> =
        LazyLock::new(|| Oid::const_new(&[1u32, 2u32, 840u32, 113549u32, 1u32, 1u32]).to_owned());
    pub static RSA_ENCRYPTION: LazyLock<ObjectIdentifier> = LazyLock::new(|| {
        Oid::new(&[&***PKCS_1, &[1u32]].concat())
            .unwrap()
            .to_owned()
    });
}
