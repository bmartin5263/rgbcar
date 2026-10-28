# CI image (`cicd/Dockerfile`)

A CI optimization. It bakes PlatformIO, the espressif32 platform and toolchains, and all `lib_deps` into an image so CI builds don't redownload them on every run (which is slow and can hit PlatformIO's download limit).

## Rebuild when

- `platform = espressif32@...` changes
- `lib_deps` change, especially several at once
- `PIO_VERSION` in the Dockerfile changes
- CI logs show `Installing ...` lines again, or the download limit returns

A stale image still works, it will just be slower or can hit the limit again.

## Build and push

Run from the repo root:

```sh
docker build --platform linux/amd64 -f cicd/Dockerfile -t ghcr.io/<user>/rgbcar-ci:latest .
echo $GHCR_TOKEN | docker login ghcr.io -u <user> --password-stdin
docker push ghcr.io/<user>/rgbcar-ci:latest
```

`GHCR_TOKEN` is a classic personal access token with the `write:packages` scope.

Or, build a test image

```sh
docker build --platform linux/arm64 -f cicd/Dockerfile -t rgbcar-ci:test .
```

## Verify before pushing

Commit `platformio.ini` changes first, then check the image builds from a clean clone with no network:

```sh
rm -rf /tmp/rgbcar-clean && git clone . /tmp/rgbcar-clean
docker run --rm --network none -v /tmp/rgbcar-clean:/work -w /work \
  ghcr.io/<user>/rgbcar-ci:latest sh -c "pio run -e ci && pio test -e native"
```
