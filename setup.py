#!/usr/bin/env python
# -*- coding: utf-8 -*-


from distutils.core import setup, Extension

module = Extension('sqlitebck', 
        sources=['src/sqlitebck.c', ],
        libraries=['sqlite3', ])

setup(name='sqlitebck',
   version='1.0',
   include_dirs=['./_sqlite', '/usr/local/include'],
   description='Sqlite3 backup function',
   ext_modules=[module, ])
