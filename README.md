# VL53L1X ChibiOS Library

A VL53L1X Library for ChibiOS, adapted from the [VL53L1X Ultra Lite Driver API](https://www.st.com/en/embedded-software/stsw-img009.html).


## HOW-TO Use

- In your existing ChibiOS project, make a folder called `libs` in the root folder.
- Move the `vl53l1x_uld`-folder and the `libs.mk`-file to the new `libs`-folder.
- Update the `Makefile` in your ChibiOS project.
    - First, point to this new libraries folder: `LIBS     := ./libs`
    - Second, include the `libs.mk`-file under `# Other files (optional).` as such: `include $(LIBS)/libs.mk`


## Example project(s)

There is an example project in the `example`-folder showing you an almost working project.

To use the example code move the `libs.mk`-file and the `vl53l1x_uld`-folder to the `libs`-folder. The `Makefile` is already predefined. An additional `README.md`-file is added explaining about how to wire up the VL53L1X module with a Nucleo-F432KC board.


## Useful Links

- ChibiOS on Github: https://github.com/ChibiOS/ChibiOS
- VL53L1X Datasheet: https://www.st.com/resource/en/datasheet/vl53l1x.pdf
- VL53L1X Ultra Lite Driver (ULD) API User Manual: https://www.st.com/resource/en/user_manual/um2510-a-guide-to-using-the-vl53l1x-ultra-lite-driver-stmicroelectronics.pdf
- VL53L1X Ultra Lite Driver (ULD) API (written in C): https://www.st.com/en/embedded-software/stsw-img009.html
