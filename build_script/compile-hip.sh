#!/bin/bash
export KMDUMPISA=1

make -j driver
/opt/rocm/hcc/bin/llvm-objdump -mcpu=gfx906 -source -line-numbers driver/dump-gfx906.isabin > driver/dump-gfx906.isabin.isa
