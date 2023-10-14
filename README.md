# SurfaceFix

Surface Fix is a DLL for Game Maker 7 and 8 that "fixes" 3D surface support. The GM runner does not
associate depth buffers with surfaces, making them useless as separate render targets in 3D games.
Surface Fix hooks runner functions to bind a depth buffer to each surface, allowing you to create 3D
effects without shaders and even use room transitions.

I wrote this back in 2009 and posted it under the username amd42. I decided to upload the source
code for archival purposes so that GM projects which use it can continue to work. It is pretty much
untouched from the last release, aside from being packaged into a git repository.

## Download

You can download the original DLL and GEX builds from the
[releases page](https://github.com/adierking/SurfaceFix/releases).

## Compiling

The included solution file is for Visual Studio 2010 but may work with newer versions. You will need
to build for 32-bit and compile against the [DirectX 8 SDK](https://archive.org/details/dx8a_sdk).
