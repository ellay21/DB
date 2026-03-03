# Docker setup for Revenant

## The most important flags

```bash
docker run --rm -it \
  --shm-size=2g \          # default 64 MB; rings will not fit
  --ulimit memlock=-1:-1 \ # mlock / MAP_LOCKED
  --ulimit nofile=65536:65536 \
  --cap-add=SYS_NICE \     # sched_setaffinity, priorities
  --cap-add=IPC_LOCK \     # hugepages, mlock
  --security-opt seccomp=unconfined \   # ASan/TSan
  -v "$PWD":/work -w /work \
  ghcr.io/you/revenant-dev@sha256:...
```

**`--shm-size=2g` is the one that catches everyone.**
Docker's default `/dev/shm` is 64 MB. A 64 MiB ring plus metadata plus a second channel will fail with a confusing `ENOSPC` from `ftruncate`. The bootstrap script checks and warns loudly.

## What the container pins — and what it does not

The container pins: compilers, analysis tools, dependency versions.

The container does **not** pin: the kernel. `pidfd_open` availability, cgroup v2 layout, hugepage configuration, and `perf_event_paranoid` all come from the host kernel. CI tests kernel-feature axes explicitly.

## `--pid=host`

Used **only** by the chaos test compose service. It is a genuine security relaxation and must never be used by the default dev container.

## Images

| Image | Tag pattern | Purpose |
|-------|-------------|---------|
| `Dockerfile.dev` | `revenant-dev:latest` | Human development |
| `Dockerfile.ci`  | `revenant-ci:gcc-13`, `revenant-ci:clang-18` | GitHub Actions |
