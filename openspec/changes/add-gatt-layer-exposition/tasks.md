## 1. Implementation
- [x] 1.1 Define the GATT service/characteristic UUIDs and data format for layer number exposure
- [x] 1.2 Package the feature as an external ZMK/Zephyr module rather than a direct core-ZMK patch
- [x] 1.3 Add a read characteristic in the module that returns the current active layer number
- [x] 1.4 Add notify support in the module and emit updates on layer change when clients subscribe
- [x] 1.5 Document and validate the build/config integration required to enable the module for Corney builds
- [x] 1.6 Add tests or validation steps for read/notify behavior
