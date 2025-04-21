## Gupo

- Gabriel Campelo Formiga
- Gabriel Sherterton Araújo de Freitas
 -Guilherme Dantas Pinto

### Prerequisites

- [Meson Build System](https://mesonbuild.com/)
- C++ 17

### How to build the project

1. Setup release and debug builds:

```
meson setup build --buildtype=release
```

2. Compile build:

```
meson compile -C <build>
```

3. Run:

From the root directory:

```
./<build or build_debug>/src/asp
```
