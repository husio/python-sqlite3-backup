#!/usr/bin/env python
# -*- coding: utf-8 -*-


from distutils.core import setup, Extension

module = Extension('sqlitebck',
        sources=['src/sqlitebck.c', ],
        libraries=['sqlite3', ])

setup(name='sqlitebck',
    version='1.0',
    include_dirs=['/usr/local/include', ],
    description='Sqlite3 online backup API implementation.',
    ext_modules=[module, ],
    author='Piotr Husiatyński',
    author_email='phusiatynski@gmail.com',
    license='MIT',
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Intended Audience :: Developers',
        'Operating System :: OS Independent',
        'Programming Language :: SQL',
        'Programming Language :: Python',
    ])
