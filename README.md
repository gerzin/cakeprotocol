# Cake Protocol

> WIP. Detection still missing

At work there's this thing where if someone leaves the laptop unlocked and unattended, passerby colleagues will
open teams and post **cake** on a random group on teams. Others will pile on with a wave of __cake__ replies.

This small app called cakeprotocol puts an end to this. It uses uses the webcam and [Haar cascades](https://docs.opencv.org/3.4/db/d28/tutorial_cascade_classifier.html) to detect if the user is in front of the laptop. It has a configurable poll interval and miss treshold to decide when to lock the screen.

Developed and tested on Linux. Should work on Mac and Windows too. If it doesn't feel free to open an issue or even better, a PR.

> Among the dependencies there are some that are heavy to compile, like protobuf, that might get you an OOMKilled when trying to compile. For this reason I limited the parallelism in the [user.bazelrc](./user.bazelrc). Usually this file should not be committed. I added it anyway, just tweak it with a value that fits your machine.

## Build
The app uses [Bazel](https://bazel.build/install/bazelisk) and C++23.

### Release build
```sh
bazel build -c opt //src/main:main
```
Note: There's a `:cakeprotocol` alias that points to the `//src/main:main`.

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
