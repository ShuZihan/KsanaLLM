# Copyright 2024 Tencent Inc.  All rights reserved.

if(NOT WITH_TESTING)
  return()
endif()

enable_testing()
include(external/gtest)

set(PYTHON_PATH "python" CACHE STRING "Python path")
execute_process(COMMAND ${PYTHON_PATH} "-c" "from __future__ import print_function; import torch; print(torch.__version__,end='');"
  RESULT_VARIABLE _PYTHON_SUCCESS
  OUTPUT_VARIABLE TORCH_VERSION)

if(TORCH_VERSION VERSION_LESS "1.5.0")
  message(FATAL_ERROR "PyTorch >= 1.5.0 is needed for TorchScript mode.")
endif()

execute_process(COMMAND ${PYTHON_PATH} "-c" "from __future__ import print_function; import os; import torch;print(os.path.dirname(torch.__file__),end='');"
  RESULT_VARIABLE _PYTHON_SUCCESS
  OUTPUT_VARIABLE TORCH_DIR)

if(NOT _PYTHON_SUCCESS MATCHES 0)
  message(FATAL_ERROR "Torch config Error.")
endif()

list(APPEND CMAKE_PREFIX_PATH ${TORCH_DIR})
find_package(Torch REQUIRED)
execute_process(COMMAND ${PYTHON_PATH} "-c" "from __future__ import print_function; from distutils import sysconfig;print(sysconfig.get_python_inc());"
  RESULT_VARIABLE _PYTHON_SUCCESS
  OUTPUT_VARIABLE PY_INCLUDE_DIR)

if(NOT _PYTHON_SUCCESS MATCHES 0)
  message(FATAL_ERROR "Python config Error.")
endif()

execute_process(COMMAND ${PYTHON_PATH} "-c" "from __future__ import print_function; import torch;print(torch._C._GLIBCXX_USE_CXX11_ABI,end='');"
  RESULT_VARIABLE _PYTHON_SUCCESS
  OUTPUT_VARIABLE USE_CXX11_ABI)
include_directories(${PY_INCLUDE_DIR})
include_directories(${TORCH_DIR}/include/torch/csrc/api/include/)
include_directories(${TORCH_DIR}/include/)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
