from distutils.core import setup, Extension
setup(name='quickft', version='1.0',  \
      ext_modules=[Extension('libQuickFTpy', 
      ['py.c'] )])
