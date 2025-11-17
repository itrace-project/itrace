#!/bin/bash

set -e

find src \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 -n1 echo
find itrace-cli \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 -n1 echo
find include \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 -n1 echo
find examples \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 -n1 echo
find itrace-tui \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 -n1 echo

find src \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i
find itrace-cli \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i
find include \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i
find examples \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i
find itrace-tui \( -iname '*.cpp' -o -iname '*.h' -o -iname '*.hpp' \) -print0 | xargs -0 clang-format -i
