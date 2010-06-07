# -*- coding: utf-8 -*-

import os
import unittest
import sqlite3

import sqlitebck


class SqliteBackpTest(unittest.TestCase):

    def setUp(self):
        self.dest_filename = '/tmp/dest_db.sqlite3.db'
        self.open_connections = []
    
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
        sqlitebck.save(db=source_db, dest=self.dest_filename)
        db_dest = sqlite3.connect(self.dest_filename)
        # not try to fetch this data from file database
        self.open_connections.append(db_dest)
        d_curr = db_dest.cursor()
        d_curr.execute('SELECT * FROM foo ORDER BY bar DESC')
        dest_data = d_curr.fetchall()
        d_curr.close()
        self.assertEqual(source_data, dest_data)

    def tearDown(self):
        if os.path.isfile(self.dest_filename):
            os.unlink(self.dest_filename)
        # cleanup databases connections...
        for db_conn in self.open_connections:
            db_conn.close()




if __name__ == '__main__':
    unittest.main()
