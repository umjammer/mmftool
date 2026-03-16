# mmftool

mmfplayer powered by M5_Emu*.dll for cross compiling on macos.

## Install

### prepare

```
DefMA3_16.vm3
M5_EmuHw.dll
M5_EmuSmw5.dll
```

### build

```shell
$ brew intall mingw-w64 wine-crossover
$ git clone ...
$ make
```

## Usage

```shell
$ MMFTOOL_MASTER_VOLUME=20 wine mmftool.exe
```

### environment variable

- `MMFTOOL_MASTER_VOLUME` ... 0 to 127

## References

- [original](https://murachue.sytes.net/web/softlist.cgi?mode=desc&title=mmftool)

## TODO

- make `M5_Emu*.dll` architecture independent like [this](https://github.com/M-HT/websynth_d-77)
