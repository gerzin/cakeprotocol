# Cake Protocol

> WIP. Detection still missing

At work there's this thing where if someone leaves the laptop unlocked and unattended, passerby colleagues will
open teams and post **cake** on a random group on teams. Others will pile on with a wave of __cake__ replies.

This small app called cakeprotocol puts an end to this.

Developed and tested on Linux. Should work on Mac and Windows too. If it doesn't feel free to open an issue or even better, a PR.

## Build
The app uses [BAZEL](https://bazel.build/install/bazelisk) and C++23.

### Release build
```sh
bazel build -c opt //src/main:main
```

### Install
Copy the optimized binary somewhere in your PATH:
```sh
cp $(bazel cquery --output=files -c opt //src/main:main 2>/dev/null) <SOMEWHERE IN $PATH>/cakeprotocol
```

### Run
```sh
bazel run //src/main:main
```

### Generate compile_commands.json
```sh
bazel run //:refresh_compile_commands
```

### Update bazelrc presets
```sh
bazel run //tools:bazelrc.update
```
