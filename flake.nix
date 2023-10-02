{
  description = "EE500 Assignment Flake";
  nixConfig.bash-prompt = "\\n\\[\\033[1;32m\\][\\[\\e]0;\\u@\\h: \\w\\a\\]\\u(EE500):\\w]\$\\[\\033[0m\\] ";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    ns3pkgs.url = "github:nixos/nixpkgs/4ce6364e291d6853cee1bf8383e1e1e8b25abbcb";
  };

  outputs = {
    self,
    nixpkgs,
    ns3pkgs,
  }: let
    system = "x86_64-linux";
    upkgs = nixpkgs.legacyPackages.${system}.pkgs;
    pkgs = ns3pkgs.legacyPackages.${system}.pkgs;
  in {
    formatter.${system} = upkgs.alejandra;
    packages.${system} = rec {
      ns3 =
        (pkgs.ns-3.override {
          withExamples = true;
          modules = [];
        })
        .overrideAttrs (o: rec {
          version = "30.1";
          nativeBuildInputs = o.nativeBuildInputs ++ [pkgs.pkg-config];
          buildInputs = o.buildInputs ++ [pkgs.sqlite];
          src = pkgs.fetchFromGitLab {
            owner = "nsnam";
            repo = "ns-3-dev";
            rev = "ns-3.${version}";
            sha256 = "sha256-lD2xDZbt7ny6XN4sUkO9Z++LJspfIWOhS+15zN59080=";
          };
        });
      WiFi = pkgs.stdenv.mkDerivation {
        pname = "network-simulation";
        version = "0.0.1";
        src = ./WiFi;
        nativeBuildInputs = [pkgs.cmake pkgs.pkg-config];
        buildInputs = [ns3 pkgs.sqlite];
      };
      default = WiFi;
    };
    devShells.${system} = {
      default = pkgs.mkShell {
        nativeBuildInputs = [self.packages.${system}.WiFi pkgs.gnuplot pkgs.sqlite];
      };
    };
  };
}
