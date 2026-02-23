fn main() {
    cxx_build::bridge("src/lib.rs")
        .std("c++20")
        .compile("crypto-rasn-wrapper");
}
