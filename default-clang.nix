with import <nixpkgs> {};
let
libcxxStdenv = llvmPackages_5.libcxxStdenv;
boostClang = boost165.override { stdenv = libcxxStdenv; };
tbbClang = tbb.override { stdenv = libcxxStdenv; stdver="c++17"; };
sfmlClang = sfml.override { stdenv = libcxxStdenv; };
in
libcxxStdenv.mkDerivation rec {
  name = "env";
  env = buildEnv { name = name; paths = buildInputs; };
  buildInputs = [
    #libcxxabi
    #libcxx
    #clang_39
    boostClang
    tbbClang
    sfmlClang
  ];
}

