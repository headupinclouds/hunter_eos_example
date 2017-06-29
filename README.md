# hunter_eos_example
Simple EOS example (deformable face mesh fitting) using dlib detection + landmarks with the hunter package manager.

* https://github.com/patrikhuber/eos
* https://github.com/davisking/dlib
* https://github.com/ruslo/hunter

Quick start:

```
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Release
```
 
Or using polly toolchains:

See: https://github.com/ruslo/polly.git (toolchains)

```
polly.py --toolchain libcxx --verbose --open --reconfig 
```
```
polly.py --toolchain xcode --verbose --open --reconfig 
```
...
