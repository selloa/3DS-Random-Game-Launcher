# Third-party licenses

This project is licensed under the [MIT License](../LICENSE). The components below are used at build time or runtime and remain under their own terms.

## libctru

Homebrew library for Nintendo 3DS (Horizon OS user mode). Linked when building this application.

- Project: https://github.com/devkitPro/libctru
- Documentation: https://libctru.devkitpro.org

```
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source
   distribution.
```

## devkitPro / devkitARM

Cross-compilation toolchain and 3DS development libraries used to build this project. Install via [devkitPro](https://devkitpro.org/).

Toolchain and bundled libraries are subject to devkitPro’s licensing and distribution terms. See the devkitPro repositories and installer documentation for current license text.

## devkitPro 3ds-examples (`app_launch`)

Early launch logic was informed by the devkitPro example template:

- https://github.com/devkitPro/3ds-examples/blob/master/app_launch/source/main.c

No separate license file is published at the repository root; treat as devkitPro sample code and retain attribution when distributing derivatives.

## Prior art: einso/3ds-randomtitlepicker

Original concept and proof-of-concept by [einso](https://github.com/einso/3ds-randomtitlepicker). That repository does not declare a license. This fork is a substantial rewrite with additional features; einso is credited in the [README](../README.md).

## Title database sources

Offline title metadata is compiled from public 3DS title listings (e.g. community databases). Game names and title IDs are factual references; trademarks belong to their respective owners.
