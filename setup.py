#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from distutils.core import setup, Extension

VERSION = '1.4'

def long_description():
    fd_path = os.path.join(os.path.dirname(__file__), 'README')
    if not os.path.isfile(fd_path):
        return ''
    with open(fd_path) as fd:
        return fd.read()

module = Extension('sqlitebck', sources=['src/sqlitebck.c'],
        libraries=['sqlite3'])

setup(name='sqlitebck',
    version=VERSION,
    include_dirs=['/usr/local/include', ],
    description='Sqlite3 online backup API implementation.',
    long_description=long_description(),
    url='https://github.com/husio/python-sqlite3-backup',
    download_url='https://github.com/husio/python-sqlite3-backup/tarball/master#egg=sqlitebck-' + VERSION,
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
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: Implementation :: CPython',
    ])
