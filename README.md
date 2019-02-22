# ZZC

ZZC is a pack of VCV Rack plugins developed by [Sergey Ukolov (zezic)](https://github.com/zezic).
The purpose of this pack is to make operations from the world of classic DAW easier to reproduce in VCV patches and not loose the advantages of CV control.

## Why it's not listed in VCV Plugin Manager?

At the moment this pack is in early development phase and I need some time to ensure that all the ideas are playing well with each other. I don't want to break compatiblity with user patches during active development.
When this pack will gain critical mass of modules and become an obvious candidate for installation by most of Rack users I will publish it in Plugin Manager for sure.

## Contributing

I welcome Issues, Pull Requests to this repository if you have suggestions for improvement.
Also, new module ideas are welcome. If module idea is really in demand, feels original and not too complicated with high probability I will develop it shortly if I have enough spare time.

## Building on Linux

```bash
export RACK_DIR=/path/to/Rack-SDK
make -j7
```

## Helpers for IDE

```bash
pip install compiledb
compiledb make
```
