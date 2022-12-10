import ctypes
lib = ctypes.cdll.LoadLibrary('/src/nona/nona3.so')

lib.testFunc()
