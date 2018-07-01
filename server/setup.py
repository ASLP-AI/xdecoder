import os
from distutils.core import setup, Extension

os.environ["CC"] = "g++"

module = Extension('_xdecoder', 
                   language='c++',
                   sources=['resource-manager_wrap.cxx',
                            '../src/resource-manager.cc',
                            '../src/decodable.cc',
                            '../src/decode-task.cc',
                            '../src/faster-decoder.cc',
                            '../src/feature-pipeline.cc',
                            '../src/fft.cc',
                            '../src/fst.cc',
                            '../src/net.cc',
                            '../src/utils.cc',
                            '../src/vad.cc'],
                   include_dirs=['../src', '../'],
                   libraries=['openblas'],
                   library_dirs=['usr/lib', 'usr/local/lib'],
                   define_macros=[('USE_VARINT', None),
                                  ('USE_BLAS', None)],
                   extra_compile_args=['-g', '-std=c++11', '-msse4.1'])

setup(name='xdecoder',
      version='0.1',
      author='Binbin Zhang',
      description='xdecoder python interface by swig',
      ext_modules=[module],
      py_modules=['xdecoder'])
