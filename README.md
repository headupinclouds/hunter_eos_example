# hunter_eos_example
Simple EOS example (deformable face mesh fitting) using dlib detection + landmarks with the hunter package manager.  This avoids the need to install dependencies (the eos and dlib model files still need to be downloaded.)

* https://github.com/patrikhuber/eos
* https://github.com/davisking/dlib
* https://github.com/ruslo/hunter

Quick start:

Choose a working directory:
```
cd /tmp
```

Clone the sample app:
```
git clone https://github.com/headupinclouds/hunter_eos_example.git
cd hunter_eos_exmaple
```

Build via cmake:
```
cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=Release
```
 
Or using polly toolchains (see: https://github.com/ruslo/polly.git)

```
TOOLCHAIN=xcode # or one for iOS, Android, etc
polly.py --toolchain $TOOLCHAIN --verbose --config Release --install --reconfig --open
```

![example](https://user-images.githubusercontent.com/554720/27667305-edd3b0c4-5c47-11e7-94be-67ff6423dcdf.jpg)

Download models:

* https://github.com/patrikhuber/eos/tree/master/share
* https://github.com/davisking/dlib-models/blob/master/shape_predictor_68_face_landmarks.dat.bz2
