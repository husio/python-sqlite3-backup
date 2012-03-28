#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from distutils.core import setup, Extension

module = Extension('sqlitebck',
        sources=['src/sqlitebck.c', ],
        libraries=['sqlite3', ])

def long_description():
    with open(os.path.join(os.path.dirname(__file__), 'README.rst')) as fd:
        return fd.read()

setup(name='sqlitebck',
    version='1.0',
    include_dirs=['/usr/local/include', ],
    description='Sqlite3 online backup API implementation.',
    long_description=long_description(),
    url='https://github.com/husio/python-sqlite3-backup',
    download_url='https://github.com/husio/python-sqlite3-backup/tarball/master#egg=sqlitebck-1.0',
    ext_modules=[module, ],
    author='Piotr Husiatyński',
    author_email='phusiatynski@gmail.com',
    license='MIT',
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Intended Audience :: Developers',
        'Operating System :: OS Independent',
        'Topic :: Database',
        'Programming Language :: C',
        'Programming Language :: Python :: 2 :: Only',
        'Programming Language :: Python :: Implementation :: CPython',
    ])
