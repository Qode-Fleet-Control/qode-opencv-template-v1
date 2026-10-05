# OpenCV template

Provisioned from [`Qode-Fleet-Control/fleet-template-v1`](https://github.com/Qode-Fleet-Control/fleet-template-v1) — the fleet
lifecycle contract (`bin/`, `fleet.conf`, `compose.yaml`, deploy workflows) with a OpenCV (C++) starter laid on top.

A computer-vision batch job on OpenCV 4.10 (Debian trixie), built with CMake. It loads the bundled sample image `data/fruits.jpg`, runs grayscale → Gaussian blur → Canny edges → external contours, writes `out/edges.png` and `out/contours.png`, and prints a summary. It exits 0 on success and non-zero when the image cannot be read, the results cannot be written, or the pipeline finds no edges — so the container's exit code is the job's result. `INPUT_IMAGE` / `OUTPUT_DIR` (or argv[1] / argv[2]) point it elsewhere.

## Origin

    hand-written (OpenCV ships no project generator) — CMakeLists.txt as in OpenCV's "Using OpenCV with gcc and CMake" tutorial (find_package(OpenCV) + ${OpenCV_LIBS}); data/fruits.jpg is samples/data/fruits.jpg from the opencv/opencv repository at tag 4.10.0


## Run it

### On the fleet

The fleet runs it as containers (the docker runtime): `bin/run` builds the image with
`docker compose build` and then stops — this is a job, so `DOCKER_START_CMD` is empty and nothing listens on `$PORT`.

### With docker

```sh
docker compose build
docker compose run --rm app            # runs the job; exit code = result
```

### Without docker

```sh
# Debian/Ubuntu: sudo apt install build-essential cmake libopencv-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/app                                  # data/fruits.jpg -> out/
./build/app path/to/image.png results/       # any other image
```

`fleet.conf` drives every script in `bin/`:

| step | docker runtime (fleet) | `FLEET_RUNTIME=process` |
|---|---|---|
| install | — | `(none)` |
| build | `docker compose build` | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j` |
| start | `(none — a job)` | `(none — a job)` |

## Layout

- `CMakeLists.txt` — one executable target `app`, linked to the `core`, `imgproc` and `imgcodecs` modules only.
- `src/main.cpp` — the pipeline and its success checks.
- `data/fruits.jpg` — the bundled sample image (from OpenCV's own samples).
- `Dockerfile` — `debian:trixie` build stage with `libopencv-dev`; `debian:trixie-slim` runtime with only `libopencv-core410`, `libopencv-imgproc410`, `libopencv-imgcodecs410`; non-root user `app`; `CMD ["/app/app"]`.
- `compose.yaml` — service `app`, no ports (a job), fleet variables plus `INPUT_IMAGE` / `OUTPUT_DIR` passed through by name. Mount a volume on `/app/out` to keep the results: `docker compose run --rm -v "$PWD/out:/app/out" app`.

## Deviations from stock, and why

- The tutorial's example displays the image in a window (`imshow`); a container has no display, so this job writes its results as files and reports on stdout instead.
- The tutorial links every module (`find_package(OpenCV REQUIRED)`); this names only the three it uses, so the runtime image needs only those shared libraries.
- `PORT`, `HEALTH_PATH`, `START_CMD` and `DOCKER_START_CMD` are empty by design: nothing listens on a port, and `bin/run` builds the image and stops there.

## Verified

2026-10-05, Docker 29.8 on linux/amd64, from the scaffold directory:

- `docker compose build` → built.
- `docker compose run --rm app` → exit 0, printing `OpenCV 4.10.0`, `input: data/fruits.jpg (512x480, 3 channels)`, `edges: 7029 pixels (2.86011%)`, `contours: 146`, `wrote: out/edges.png, out/contours.png`.
- `docker compose down --rmi local -v` → clean.

The no-docker path (`FLEET_RUNTIME=process`) was not run on a host toolchain; it is the same CMake build the image runs.

See `docs/fleet-lifecycle.md` for the lifecycle contract.
