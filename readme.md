# Activitym

A real-time system monitor built from scratch.
C++ reads the hardware, C# serves it, React displays it.

## Stack

| Layer    | Tech                  | What it does                        |
|----------|-----------------------|-------------------------------------|
| sampler  | C++20 + macOS APIs    | Reads CPU + RAM, prints JSON        |
| api      | C# ASP.NET Core       | Spawns sampler, streams over SSE    |
| frontend | React + TypeScript    | Live dashboard UI                   |

## Run the sampler
```bash
cmake -B build
cmake --build build
./build/sampler
```

## Progress

- [x] C++ sampler — real CPU + RAM on macOS
- [ ] C# API — SSE endpoint
- [ ] React frontend — live dashboard
- [/] Windows port — swap macOS APIs for PDH + GlobalMemoryStatusEx