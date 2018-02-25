# `sqlitebck`

[![Build Status](https://travis-ci.org/husio/python-sqlite3-backup.svg?branch=master)](https://travis-ci.org/husio/python-sqlite3-backup)
[![PyPI version](https://badge.fury.io/py/sqlitebck.svg)](https://badge.fury.io/py/sqlitebck)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


## SQLite3 backup function implementation for Python sqlite3 module

Single function that allows to copy content of one sqlite3 database to another one. You can use this for example for loading and dumping in memory database (`:memory:`) into a file (alternative to the [iter dump](https://docs.python.org/2/library/sqlite3.html#sqlite3.Connection.iterdump) functionality).

See the [Sqlite3 C API docs](http://www.sqlite.org/c3ref/backup_finish.html) for more information.

The same functionality is being provided by the [apsw backup](https://rogerbinns.github.io/apsw/backup.html) API, which provides more information about the copy process.


## Build and installation

You can build or install `sqlitebck` using distutils:

    $ python setup.py build
    $ python setup.py install

You can also install it, using the `pip` command:

    $ pip install sqlitebck



## Tests

Test basic functionality (make sure you have build the `sqlitebck` module):

    $ python tests.py


## Usage example

Basic usage example - a memory database saved into a file:


```python
>>> import sqlite3
>>> conn = sqlite3.connect(':memory:')
>>> curr = conn.cursor()
```

Create a table and put there some data:

```python
>>> curr.execute('CREATE TABLE foo (bar INTEGER)')
<sqlite3.Cursor object at 0xb73b2800>
>>> curr.execute('INSERT INTO foo VALUES (123)')
<sqlite3.Cursor object at 0xb73b2800>
>>> curr.close()
>>> conn.commit()
>>> import sqlitebck
```

Save a memory database (conn) into a file:

```python
>>> conn2 = sqlite3.connect('/tmp/in_memory_sqlite_db_save.db')
>>> sqlitebck.copy(conn, conn2)
>>> conn.close()
>>> curr2 = conn2.cursor()
```

Check if data is in a file database:

```python
>>> curr2.execute('SELECT * FROM foo');
<sqlite3.Cursor object at 0xb73b2860>
>>> curr2.fetchall()
[(123,)]
```

If you want to load a file database into a memory:

```python
>>> sqlitebck.copy(conn2, conn)
```

If you want to copy a (large) online database, you can release the read-lock on the source database for a specified time between [copying each batch of pages](https://sqlite.org/c3ref/backup_finish.html#sqlite3backupstep):

```python
>>> sqlitebck.copy(conn2, conn, pages, sleep)
```

By default, `pages=0` and `sleep=0`, which copies all pages with no interruption.

The online copying process handles situations with a write-lock on the source database and copying of changes written to the source database while copying batches of pages.
