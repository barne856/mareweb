# WebGPU Native

## Build for Web

```bash
emcmake cmake -B build-web
cmake --build build-web
```

Host on :8080 with:

```bash
python -m http.server -d build-web
```

## Build for Native

```bash
cd build
cmake ..
cd ..
cmake --build ./build
```

Run samples:

```bash
./build/basic_mesh
./build/basic_texture
./build/basic_renderer
```
