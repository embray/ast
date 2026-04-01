# AST -- World Coordinate Systems Library

The Starlink AST library provides a comprehensive range of facilities for
attaching world coordinate systems to astronomical data, for
retrieving and interpreting that information in a variety of formats,
including FITS-WCS, and for generating graphical output based on it.

This library should be of interest to anyone writing astronomical
applications which need to manipulate coordinate system data,
especially celestial or spectral coordinate systems.
AST is portable and environment-independent.

Full documentation is available in the Starlink User Notes:

- [SUN/210](https://www.starlink.ac.uk/docs/sun210.htx/sun210.html) -- AST programmer's guide
- [SUN/211](https://www.starlink.ac.uk/docs/sun211.htx/sun211.html) -- AST class reference

## Building with Starlink Autotools

This is the original build system.
It requires the Starlink `starconf` tool and associated build infrastructure
(including `messgen`, `sst`, and `htx`) to be installed and in your `PATH`.
The `buildsupport` tools are expected to be found via the `STARLINK`
environment variable (typically `~/star` or `/star`).

### Prerequisites

- Starlink `starconf` and `buildsupport` tools
- GNU Autotools (autoconf >= 2.69, automake >= 1.8.2, libtool)
- A C compiler
- Optional: a Fortran compiler (for Fortran wrappers and PGPLOT support)
- Optional: libyaml, pthreads

### Build

```shell
./bootstrap
./configure --prefix=/star
make
make check
make install
```

### Autotools configure options

| Option | Description |
|--------|-------------|
| `--without-fortran` | Skip Fortran wrapper generation |
| `--without-pthreads` | Disable POSIX thread support |
| `--without-yaml` | Disable YAML channel support |
| `--with-external_pal` | Use system PAL/ERFA libraries |
| `--with-external_cminpack` | Use system CMINPACK library |
| `--with-starmem` | Use Starlink memory management library |
| `--with-memdebug` | Enable memory leak debugging |
| `--without-topinclude` | Install headers only in `includedir/star` |

## Building with CMake

The CMake build does not require any Starlink infrastructure.

### Prerequisites

- CMake >= 3.14
- A C compiler
- Perl (for generating the `ast.h` public header)
- Optional: libyaml (for YAML channel support)
- Optional: pthreads (for thread safety)
- Optional: a Fortran compiler (for Fortran wrappers, requires `AST_BUILD_FORTRAN=ON`)

### Build

```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
cmake --install build --prefix /usr/local
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `ON` | Build shared libraries |
| `BUILD_TESTING` | `ON` | Build the test program |
| `AST_BUILD_FORTRAN` | `OFF` | Build Fortran wrappers (requires a Fortran compiler) |
| `AST_WITH_PTHREADS` | `ON` | Enable POSIX thread support |
| `AST_WITH_YAML` | `ON` | Enable YAML channel support |
| `AST_WITH_EXTERNAL_PAL` | `OFF` | Use system PAL/ERFA instead of bundled |
| `AST_WITH_EXTERNAL_CMINPACK` | `OFF` | Use system CMINPACK instead of bundled |
| `AST_WITH_MEMDEBUG` | `OFF` | Enable memory leak debugging |

### Using AST from another CMake project

After installation, use `find_package`:

```cmake
find_package(ast REQUIRED)
target_link_libraries(myapp PRIVATE AST::ast AST::ast_err AST::ast_grf_2.0
    AST::ast_grf_3.2 AST::ast_grf_5.6 AST::ast_grf3d)
```

Or use pkg-config:

```shell
cc -o myapp myapp.c $(pkg-config --cflags --libs ast)
```

## License

AST is licensed under the GNU Lesser General Public License, version 3
or later. See `COPYING` for details.
