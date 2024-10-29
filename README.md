# WebGPU Native

## Build for Web

```bash
emcmake cmake -B build-web
cmake --build build-web
```

Host on :8080 with:

```bash
python3 -m http.server -d build-web
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


# TODO, mesh system like square
# TODO, vertex input assembly? Need to make meshes with different inputs possible, pos3, pos3norm3, pos3norm3tex2, vertex colors, etc.
# pairity with SQAURE
# imlement PBR materials
# normal matrix bug? lighting is glitchy when rotating object



