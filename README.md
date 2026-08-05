# Kilket

 ```text
 ██╗  ██╗ ██╗ ██╗      ██╗  ██╗ ███████╗ ████████╗
 ██║ ██╔╝ ██║ ██║      ██║ ██╔╝ ██╔════╝ ╚══██╔══╝
 █████╔╝  ██║ ██║      █████╔╝  █████╗      ██║   
 ██╔═██╗  ██║ ██║      ██╔═██╗  ██╔══╝      ██║   
 ██║  ██╗ ██║ ███████╗ ██║  ██╗ ███████╗    ██║   
 ╚═╝  ╚═╝ ╚═╝ ╚══════╝ ╚═╝  ╚═╝ ╚══════╝    ╚═╝   

```




## Table of Contents
1. [Introduction](#introduction)
2. [Why Kilket?](#why-kilket)
3. [Installation](#installation)
4. [Command Reference & Usage](#command-reference--usage)


### Introduction

Kilket is an open-source CLI tool that automates your build process by watching for filesystem changes and running your build commands for you. It's built to be simple, efficient, and easy to use, letting you define one or more build commands through a straightforward CLI interface. You can also hook in additional commands to run automatically after a build succeeds or fails.

Point it at a directory, and Kilket watches for changes and triggers your build commands as soon as they're detected.

### Why Kilket?

Kilket started out of personal necessity and a desire to look under the hood. A few months ago, I began diving deep into C++ and exploring systems programming. While falling in love with low-level control, I quickly hit a massive developer bottleneck: the constant, frustrating friction of having to manually re-compile and execute code every single time I made a minor file modification just to see if my changes worked. 

While heavy, pre-existing file-watching frameworks exist, I decided to build Kilket as a raw learning tool to solve this exact pain point. As the project evolved, my focus shifted toward tailoring the tool to handle the specific real-world workflow issues I face every night, shaping it into something I actually want to use every day. 

What began as a simple automation script grew into a highly optimized, zero-bloat systems utility. Because it operates close to the iron and leverages the Linux kernel’s native `inotify` subsystem, it monitors deep project directories with near-zero CPU overhead and a virtually invisible memory footprint. 

### What Makes it Different?
While there are other file watchers out there, Kilket was designed with a few core strengths that are difficult to find out-of-the-box in popular alternatives:

* **True Persistence:** Kilket configurations don't evaporate. Your active watch environments and tasks are securely serialized to disk. If your system fully shuts down or reboots, Kilket recovers its exact state gracefully on reload without locking up on stale paths. Once you set up your system you only need to run the `kilket run` command whenever you want to start monitoring your project and start a session.
* **Multi-Project Task Orchestration:** It isn't restricted to a single folder. A single instance can keep track of completely separate projects, mapping out individual boundaries and firing off independent hook configurations concurrently.
* **Contextual Branching (`on_success` / `on_failure`):** Instead of blindly executing a single catch-all command string on save, Kilket intrinsically tracks execution exit states. This allows you to uniquely fork separate execution paths—like running an extensive test suite only if the compilation passes, or sending a desktop notification the exact millisecond a build breaks.
* **Language Agnostic:** Though engineered in native C++, Kilket is entirely toolchain-agnostic. Whether you are compiling C++, watching TypeScript builds, restarting a Go backend, or running Python scripts, it hooks into any CLI pipeline seamlessly.
* **Session Tracking:** Kilket keeps track of your active sessions, and logs information about each of your sessions, into a log file called `<YOUR_PROJECT_NAME>-kilket.log` right in your project directory. So you can easily review the history of your sessions and your build history including the terminal output and several other informations, and debug any issues that arise at any time.

### Installation

#### Option 1

This is the easiest way to get started with Kilket.

1. You can grab the latest release from the [GitHub releases page](https://github.com/kefaKa/Kilket/releases/latest). Steps to download the binary release can be found in the release page.

#### Option 2

If you prefer to compile from source or are modifying the codebase, ensure your system has a modern C++ toolchain supporting C++17/20, cmake, and make.

On Ubuntu/Debian or WSL (Ubuntu), install the build requirements:

```Bash
sudo apt update && sudo apt install -y build-essential cmake
```
Then compile and install:
1. Clone and navigate into the project

```Bash
git clone https://github.com/kefaKa/Kilket.git
cd Kilket
```
2. Create and enter a clean build directory
```Bash
mkdir build && cd build
```

3. Generate an optimized Release build configuration
```Bash
cmake -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release ..
```
4. Compile the binary using all available CPU cores(to save some of your time)
```Bash
make -j$(nproc)
```
5. Install system-wide to `/usr/local/bin`
```Bash
sudo make install
```

#### Verifying the Installation

To verify the installation, run `kilket --version` and ensure it outputs the version number.

## Command Reference & Usage

Kilket provides a Git-like subcommand interface to seamlessly configure, track, and monitor your development pipelines.

### Quick Start Workflow

1. Initialize a new Kilket environment in your project root
```bash
kilket init
```
2. Add your execution bounds and build/test targets
```bash
kilket add --command "make" --on-success "./run_tests" --on-failure "echo 'Fix the build!'"
```
3. Prevent heavy transient directories from overloading tracking tables
```bash
kilket add --ignored-path "build/" --ignored-pattern "*.o"
```
4. Spin up the background thread filewatcher engine
```bash
kilket run
```

### Global Flags
These flags can be appended to the base kilket command from anywhere:

`-h`, `--help` — Displays structural help layouts and options for any command or subcommand.

`--version` — Prints the active engine version layout (0.0.0) and immediately exits.

`--verbose` — Enables verbose logging output.

### Subcommands

*Tip: Use `kilket --help <subcommand>` to see detailed usage and options for each subcommand.

#### 1. init
Initializes a kilket instance in your current directory and registers it inside a config.json file under `$HOME/.config/kilket/config.json`.

Usage: `kilket init`

Options: 
  `--task TEXT`[OPTIONAL] — enables you to name your kilket instance a unique name. If not provided the filename of your current directory will be taken as name. 

#### 2. add

Usage: `kilket add [options]`

Options:

`--command "<cmd>"` — The base target compilation or execution string to trigger on file change.

`--on-success "<cmd>"` — Hook execution path to fork immediately if the base target command returns exit code 0.

`--on-failure "<cmd>"` — Hook execution path to fork immediately if the base target command returns a non-zero exit code.

`--ignored-path "<path>"` — Tells the internal filewatcher module to explicitly skip registering inotify bounds on this directory.

`--ignored-pattern "<glob>"` — Skips filing tracking updates for files matching specific extensions (e.g., *.tmp, *.o).

#### 3. list
Dumps an immediate structural breakdown of the currently registered configuration data into your terminal.

Usage: `kilket list [options]`

Flags:

  `--tasks` — lists all registered tasks and their associated paths in your machine.
  
  `--paths` — lists all registered paths and watched paths in the working directory.

  `--commands` — lists all build commands registered in the current directory.

  `--ignored` — lists all ignored paths and patterns in the current directory.

  `--on-success` — lists all on success commands to run in the current directory.

  `--on-failure` — lists all on failure commands to run in the current directory.

#### 4. remove
Drops a specified path boundary, target match pattern, or hook configuration from the active registry.

Usage: `kilket remove [options]`

Options:

`-f` — Bypasses structural verification checks to force immediate state eviction.

#### 5. set
Tweaks global engine performance constants and environmental execution behaviors.

Usage: `kilket set [options]`

Options:

`--depth <int>` — Adjusts recursive folder watch limits. Accepts integers mapping from 0 (shallow directory tracking) up to 10.

Flag:

 `--active` — sets the the initialized task in the current directory as active. This enables you to watch for only tasks labeled as active later using the command `kilket run --active`

#### 6. check
It checks for the status set by the `set` command.

Usage: `kilket check`
Options: 
  `--depth`  - Adjusts recursive folder watch limits.
  `--active`  - checks if current kilket instance in the current working directory is labeled active.
  `--deactive`  - checks if current kilket instance in the current working directory is labeled deactive.

#### 7. run
Spins up the underlying filewatcher background-thread, mounts native kernel inotify tracking hooks across the working directory, and starts orchestrating loops.

Usage: `kilket run`

Flags:

 `--quiet` — Silences non-essential lifecycle output streams, funneling thread-safe logs strictly to background buffers instead of flooding stdout.

 `--all` - Runs all registered tasks across the entire machine.
 `--active` - Runs all tasks that are labeled as active.

Your complete layout is officially complete.

## Author

Kefa Kassahun - aspiring systems engineer

## Used 

- [nlohmann/json library](https://github.com/nlohmann/json)
- [CLI11 library](https://github.com/CLIUtils/CLI11)

```
