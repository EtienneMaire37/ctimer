# ctimer

ctimer is a speedrun timer for the terminal written in C.

<p align="center">
<img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/EtienneMaire37/ctimer/cmake-multi-platform.yml">
<img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/EtienneMaire37/ctimer">
</p>

## Cross platform support

ctimer currently only supports linux.
If you want support for any other OS, feel free to create an [issue](https://github.com/EtienneMaire37/ctimer/issues).

## Screenshots

![INFRA - Beat Part 1](/screenshots/img1.png)
![Rain World - Peebles%](/screenshots/img2.png)

## Building

Clone the repo:

```bash
git clone https://github.com/EtienneMaire37/ctimer
cd ctimer
```

Then simply build using CMake:

```bash
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Release
cmake --build build/
```

The binaries will be at `/build/bin/`

## Usage

```bash
sudo ./build/bin/ctimer <path-to-splits>.lss
```

## Keybinds

| Action | Key |
|--------|-----------|
| Start/Split | **F1** |
| Reset | **F2** |
| Pause | **F3** |

## Contributing

For bug reports and suggestions, please create an [issue](https://github.com/EtienneMaire37/ctimer/issues).

## License

[MIT](/LICENSE)
