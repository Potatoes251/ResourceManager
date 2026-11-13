# Multithreaded Resource Manager & .obj Parser

A C++ resource manager that loads assets (`.obj` models and textures) **asynchronously on a thread pool**, caches them so they are never loaded twice, and works with any resource type through a small template-based API. A GLFW/OpenGL demo lets you spawn models at runtime and **switch between single-threaded and multithreaded loading** to measure the difference.

---

## Features

- **Async loading** on a custom `ThreadPool` (`std::thread`, mutex + condition variable), so the render loop never blocks on disk or parsing work.
- **Multithreaded `.obj` parsing**: in multithread mode the model receives the thread pool and can split its parsing across workers.
- **Mono / multi-thread switch** at runtime (press `R`), with the load time of each resource printed to the console.
- **Thread-safe resource cache** keyed by file name: `Get<T>()` returns an already-loaded asset instead of reloading it.
- **Generic, extensible API**: any class deriving from `IResource` that is constructible from `(std::string)` or `(std::string, ThreadPool&)` can be managed with no change to the manager (Open/Closed Principle).
- **Completion callbacks** with arbitrary arguments (`createAsync<T>(id, callback, args...)`).
- **OpenGL stays on the main thread**: workers only parse/decode on the CPU; GPU uploads are posted back to the main thread (`Application::postToMain`) and executed in `runMainThreadTasks()`.
- Demo scene with a scene graph, directional Blinn-Phong lighting, textures and a mouse-look camera.

## Architecture

```
 Main thread (owns OpenGL context)                 Worker threads
┌────────────────────────────────┐   createAsync   ┌────────────────────────┐
│ Application / Graph / Camera   │ ──────────────► │       ThreadPool       │
│                                │                 │  job queue (mutex+CV)  │
│  runMainThreadTasks()          │                 └───────────┬────────────┘
│   └─ model->uploadToGPU()      │                             │ runs
│   └─ scene graph updates       │ ◄── postToMain ─────────────┤
└──────────────┬─────────────────┘   (GL work only here)       ▼
               │ Get<T>(id)                       ┌────────────────────────┐
               ▼                                  │ ResourceManager::      │
┌────────────────────────────────┐                │   Create<T>(id)        │
│        ResourceManager         │ ◄───────────── │ (parse file, store in  │
│ unordered_map<string,          │   stores       │  cache under a mutex)  │
│     unique_ptr<IResource>>     │                └────────────────────────┘
└────────────────────────────────┘
```

| Component | Role |
|---|---|
| `IResource` | Empty polymorphic base class for every resource type (`Model`, `Texture`, …). |
| `ResourceManager` | Owns the cache (mutex-protected `unordered_map`), creates resources, exposes `Create`, `createAsync`, `Get`, `Delete`, `clearAll`. Constructed with a `bool multithread` flag. |
| `ThreadPool` | Fixed pool (`hardware_concurrency() - 1` workers by default) with a shared job queue. Accepts free functions, lambdas and member functions. |
| Construction dispatch | `if constexpr` + `std::is_constructible_v` picks `T(id, pool)` in multithread mode (so `T` can parallelize internally) or `T(id)` otherwise. |
| Main-thread task queue | `Application::postToMain` / `runMainThreadTasks` hand finished work back to the GL thread. |

### How the demo reuses resources

Press `1`–`4` once: the model is loaded asynchronously, uploaded to the GPU on the main thread when ready, then added to the scene. Press the same key again: the model is fetched from the cache (`Get<Model>`) and a **new instance is added to the scene without reloading**.

## Requirements

- Windows
- CMake **≥ [3.24]**
- A compiler supporting **C++[20]** (MSVC recommended)
- Dependencies *([vendored in `dependencies/`])*:
  - [GLFW](https://www.glfw.org/) – window and input
  - [GLAD](https://github.com/Dav1dde/glad) – OpenGL loader
  - [stb_image](https://github.com/nothings/stb) – texture decoding
  - [LibMath](https://github.com/Potatoes251/LibMath) – custom math library (`Vector3`, `Matrix4`, `Radian`, …)

## Build

```bash
git clone https://github.com/Potatoes251/ResourceManager.git
cd ResourceManager
cmake -S . -B build
cmake --build build --config Release
```

Run the executable from the build folder. Make sure the `assets/` folder (models, textures, shaders) is reachable from the working directory.

## Controls

| Input | Action |
|---|---|
| `1` | Add the **Diablo** model (`diablo3_pose.obj`); pressing again adds another instance |
| `2` | Add **Tyra** (`tyra.obj`) |
| `3` | Add the **Dragon** (`dragon.obj`) |
| `4` | Add the **Armadillo** (`armadillo.obj`) |
| `R` | **Wipe the scene and restart**, toggling between multithread and single-thread loading |
| Mouse | Look around |
| `[WASDQE]` | Move the camera |
| Close window | Quit |

The app **starts in multithread mode**; each press of `R` flips the mode. The console prints the mode and load time for every resource:

```
Parsed dragon.obj in 0.412300s (multithread)
```

## Measuring the impact of multithreading

1. Press `1`–`4` to load the models and note the times printed in the console.
2. Press `R` to wipe the scene; the mode flips (check the console output for `multithread` / `singlethread`).
3. Load the same models again and compare.

**Caveats**

- **Hot cache**: the OS caches recently read files, so the second run can be faster regardless of threading. Alternate the order or restart the app between tests.
- **Model size**: with small models, thread overhead can make multithreading *slower*. Use large `.obj` files for meaningful results.

## Usage example

```cpp
ResourceManager rm(true /* multithread */);

// Synchronous load on the calling thread
rm.Create<Texture>("container.jpg");

// Asynchronous load, callback runs on the worker once the model is parsed
rm.createAsync<Model>("dragon.obj",
    [](Model* model, Application* app) {
        app->postToMain([=] { model->uploadToGPU(); });   // GL calls on main thread only
    },
    &app);

// Later: fetch from the cache, no reload
Model* dragon = rm.Get<Model>("dragon.obj");
```

## Adding a new resource type

1. Create a class deriving from `IResource`.
2. Give it a constructor `T(std::string path)` or `T(std::string path, ThreadPool& pool)` if it can parallelize its own loading.
3. Use `rm.Create<T>(path)`, `rm.createAsync<T>(path, ...)` and `rm.Get<T>(path)`.

The manager itself needs no modification.

## Thread-safety notes

- Worker threads never call OpenGL; GPU uploads go through the main-thread task queue.
- The resource map and the pool's job queue are each protected by a `std::mutex`.
- Workers are signalled with a `std::condition_variable`; shutdown uses an atomic stop flag.

## Possible improvements

- Lock-free job queue with atomics
- Per-thread work queues and work stealing
- Compile-time string-ID hashing for cache keys instead of `std::string`
- Resource dependency tracking (model → textures)
- Prioritization of threads by task type
- CPU affinity experiments (`SetThreadIdealProcessor`)
- More resource types (shaders, sounds, scripts)
