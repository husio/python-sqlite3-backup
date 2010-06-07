Sqlite3 backup function implementation for Python sqlite3 module
================================================================

Single function that allows to save any sqlite3 database into file. See the [C
API docs](http://www.sqlite.org/c3ref/backup_finish.html) for more info.

Build and installation
----------------------

First, link CPython sqlite3 module sources::

    $ cd <sqlitebck>
    $ ls
    README  setup.py  src  tests.py
    $ ln <python-code>/Modules/_sqlite .
    $ ls
    README  setup.py  _sqlite  src  tests.py
    $ python setup.py install


Usage example
-------------

Basic usage examply - in memory dumped into file::
    
    >>> import sqlite3
    >>> conn = sqlite3.connect(':memory:')
    >>> curr = conn.cursor()
    # create table and put there some data
    >>> curr.execute('CREATE TABLE foo (bar INTEGER)')
    <sqlite3.Cursor object at 0xb73b2800>
    >>> curr.execute('INSERT INTO foo VALUES (123)')
    <sqlite3.Cursor object at 0xb73b2800>
    >>> curr.close()
    >>> conn.commit()
    >>> import sqlitebck
    # save in memory database (conn) into file
    >>> sqlitebck.save(conn, '/tmp/in_memory_sqlite_db_save.db')
    >>> conn.close()
    >>> conn2 = sqlite3.connect('/tmp/in_memory_sqlite_db_save.db')
    >>> curr2 = conn2.cursor()
    # check if data is in file database ;)
    >>> curr2.execute('SELECT * FROM foo');
    <sqlite3.Cursor object at 0xb73b2860>
    >>> curr2.fetchall()
    [(123,)]
