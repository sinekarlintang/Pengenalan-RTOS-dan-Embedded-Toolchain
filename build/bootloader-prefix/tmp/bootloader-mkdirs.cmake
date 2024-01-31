# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/HP/esp/v5.1.2/esp-idf/components/bootloader/subproject"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/tmp"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/src"
  "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Mentoring/Pengenalan-RTOS-dan-Embedded-Toolchain/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
