# Built by .github/workflows/deploy.yml (context ., file Dockerfile) and pushed
# to Artifact Registry.
#
# A job image, not a server: the default command processes the bundled sample
# image and exits 0 on success. It will never satisfy a $PORT health check.
# OpenCV 4.10 from Debian trixie. Multi-stage: the runtime stage carries only
# the three OpenCV shared libraries the binary links (core, imgproc,
# imgcodecs), not the full libopencv-dev tree, and runs as a non-root user.
FROM debian:trixie AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake ninja-build libopencv-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY src ./src
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build \
 && install -D build/app /out/app

FROM debian:trixie-slim AS runtime
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      libopencv-core410 libopencv-imgproc410 libopencv-imgcodecs410 \
 && rm -rf /var/lib/apt/lists/* \
 && useradd -r -u 10001 app
WORKDIR /app
ARG BUILD_ID=""
ENV BUILD_ID=$BUILD_ID
COPY --from=build /out/app /app/app
COPY data ./data
RUN mkdir -p /app/out && chown app:app /app/out
USER app
CMD ["/app/app"]
