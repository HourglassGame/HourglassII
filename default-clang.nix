with import <nixpkgs> {};
let
boostClang = boost163.override { stdenv = llvmPackages_4.libcxxStdenv; };
tbbClang = tbb.override { stdenv = llvmPackages_4.libcxxStdenv; };
sfmlClang = sfml.override { stdenv = llvmPackages_4.libcxxStdenv; };
in
llvmPackages_4.libcxxStdenv.mkDerivation rec {
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

