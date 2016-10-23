with import <nixpkgs> { };

stdenv.mkDerivation {
  name = "vedve-0.0.1";
  src = ./.;

  buildInputs = [ libevdev ];

  meta =  {
    description = "Keyboard remapper via evdev/uinput";
    license = "GPLv2";
  };
}