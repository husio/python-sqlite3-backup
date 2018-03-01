import os
import sqlite3
import sys
import time
import unittest


def extend_importpath():
    # extend import paths so that sqlitebck shared object file will be visible
    # for import script
    import sys
    import fnmatch

    libbck_path = None
    basedir = os.path.abspath(os.path.dirname(__file__))
    for root, dirs, files in os.walk(basedir):
        if libbck_path:
            break
        for fname in files:
            if fnmatch.fnmatch(fname, 'sqlitebck*.so'):
                libbck_path = os.path.join(basedir, root)
                break
    if not libbck_path:
        raise Exception("Cannot find \"sqlitebck\" library file")
    print(libbck_path)
    sys.path.insert(0, libbck_path)


extend_importpath()

import sqlitebck  # noqa


class CopyInheritedTest(unittest.TestCase):
    def setUp(self):
        class Connection(sqlite3.Connection):
            pass

        self.inherited = Connection(':memory:')
        self.native = sqlite3.connect(':memory:')

    def tearDown(self):
        self.inherited.close()
        self.native.close()

    def create_test_db(self, conn):
        curr = conn.cursor()
        curr.execute('CREATE TABLE t(col VARCHAR(20))')
        curr.execute('INSERT INTO t(col) VALUES(\'test\')')
        conn.commit()
        curr.close()

    def test_inerited_source(self):
        self.create_test_db(self.inherited)
        sqlitebck.copy(self.inherited, self.native)
        curr = self.native.cursor()
        curr.execute('SELECT col FROM t')
        self.assertEqual(curr.fetchone(), ('test', ))

    def test_inerited_destination(self):
        self.create_test_db(self.native)
        sqlitebck.copy(self.native, self.inherited)
        curr = self.inherited.cursor()
        curr.execute('SELECT col FROM t')
        self.assertEqual(curr.fetchone(), ('test', ))


class SqliteBackpTest(unittest.TestCase):
    def setUp(self):
        self.db_filename = '/tmp/dest_db.sqlite3.db'
        self.open_connections = []

    def tearDown(self):
        if os.path.isfile(self.db_filename):
            os.unlink(self.db_filename)
        # cleanup databases connections...
        for db_conn in self.open_connections:
            db_conn.close()

    def test_copy_from_not_sqlite_db(self):
        source_non_db = object()
        dest_db = sqlite3.connect(':memory:')
        self.assertRaises(TypeError, sqlitebck.copy, source_non_db, dest_db)

    def test_copy_to_not_sqlite_db(self):
        source_db = sqlite3.connect(':memory:')
        dest_non_db = object()
        self.assertRaises(TypeError, sqlitebck.copy, source_db, dest_non_db)

    def test_copy_from_memory_database(self):
        source_db = sqlite3.connect(':memory:')
        self.open_connections.append(source_db)
        s_curr = source_db.cursor()
        s_curr.execute('CREATE TABLE foo(bar VARCHAR(20))')
        for value in ["foo", "bar", "baz"]:
            s_curr.execute('INSERT INTO foo VALUES (?)', (value, ))
        source_db.commit()
        s_curr.execute('SELECT * FROM foo ORDER BY bar DESC')
        source_data = s_curr.fetchall()
        s_curr.close()
        # save database to file
        db_dest = sqlite3.connect(self.db_filename)
        sqlitebck.copy(source=source_db, dest=db_dest)
        # not try to fetch this data from file database
        self.open_connections.append(db_dest)
        d_curr = db_dest.cursor()
        d_curr.execute('SELECT * FROM foo ORDER BY bar DESC')
        dest_data = d_curr.fetchall()
        d_curr.close()
        self.assertEqual(source_data, dest_data)

    def test_load_from_file_into_memory(self):
        source_db = sqlite3.connect(self.db_filename)
        self.open_connections.append(source_db)
        s_curr = source_db.cursor()
        s_curr.execute('CREATE TABLE baz(foo VARCHAR(20))')
        for value in ["abc", "x-x-x", "123"]:
            s_curr.execute('INSERT INTO baz VALUES (?)', (value, ))
        source_db.commit()
        s_curr.execute('SELECT * FROM baz ORDER BY foo DESC')
        source_data = s_curr.fetchall()
        s_curr.close()
        source_db.rollback()

        # save database to file
        db_dest = sqlite3.connect(':memory:')
        sqlitebck.copy(source_db, db_dest)
        # remove file database
        os.unlink(self.db_filename)
        # not try to fetch this data from file database
        self.open_connections.append(db_dest)
        d_curr = db_dest.cursor()
        d_curr.execute('SELECT * FROM baz ORDER BY foo DESC')
        dest_data = d_curr.fetchall()
        d_curr.close()
        self.assertEqual(source_data, dest_data)

    def test_database_locked(self):
        db = sqlite3.connect(':memory:')
        db2 = sqlite3.connect(':memory:')
        curr = db.cursor()
        curr.execute('CREATE TABLE foo(bar INTEGER)')
        curr.execute('INSERT INTO foo VALUES(2)')
        curr.close()
        self.assertRaises(sqlite3.DatabaseError, sqlitebck.copy, db, db2)

    def test_copy_multiple_pages(self):
        db = sqlite3.connect(':memory:')
        curr = db.cursor()
        curr.execute('CREATE TABLE foo(bar TEXT)')
        args = (
            'a' * (2 * SQLITE3_PAGE_SIZE),
            'b' * SQLITE3_PAGE_SIZE,
        )
        curr.execute('INSERT INTO foo VALUES (?), (?)', args)
        db.commit()
        curr.close()

        db2 = sqlite3.connect(':memory:')
        sqlitebck.copy(db, db2, pages=1, sleep=10)



# https://www.sqlite.org/pgszchng2016.html
SQLITE3_PAGE_SIZE = 4096


if __name__ == '__main__':
    sys.stderr.write("""
SQLite module version: {vsqlite}
Python version: {vpython}

""".format(vpython=sys.version, vsqlite=sqlite3.version))
    unittest.main()
