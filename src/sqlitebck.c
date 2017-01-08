/*
 * Copyright (c) 2010-2012 Piotr Husiatyński <phusiatynski@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * docs for sqlite3 backup api:
 * from http://www.sqlite.org/backup.html
 */

#include <string.h>

#include "Python.h"
/* header from Python-X.X/Modules/_sqlite */
#include "_sqlite/connection.h"
#include "sqlite3.h"


struct module_state {
    PyObject *error;
    PyObject *connection_type;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else /* PY_MAJOR_VERSION < 3 */
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

/*
 * Copy database from one to another
 *
 * Returns SQLITE_OK or any SQLITE error code on fail.
 */
static int
copy_database(sqlite3 *db_to, sqlite3 *db_from) {
    sqlite3_backup * db_bck;

    db_bck = sqlite3_backup_init(db_to, "main", db_from, "main");
    if (db_bck == NULL) {
        return sqlite3_errcode(db_to);
    }
    sqlite3_backup_step(db_bck, -1);
    sqlite3_backup_finish(db_bck);
    return sqlite3_errcode(db_to);
}

/*
 * Check if a an object is an instance of sqlite3.Connection.
 *
 * Returns 1 if the object is a sqlite3.Connection, 0 if it is not a
 * connection, and -1 if an error occured.
 */
static int
is_sqlite_object(PyObject *m, PyObject *o)
{
    PyObject *connection_type;

    if (!(connection_type = GETSTATE(m)->connection_type)) {
        PyErr_SetString(
                   PyExc_AssertionError,
                   "sqlite3.Connection reference was NULL");
        return -1;
    }
    return PyObject_IsInstance(o, connection_type);
}

/*
 * copy one python database into another
 */
static PyObject*
py_copy(PyObject *self, PyObject *args, PyObject *kwds)
{
    int rc;
    static char *kw_list[] = {"source", "dest", NULL};
    PyObject *db_source_conn, *db_dest_conn;
    sqlite3 *db_source, *db_dest;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kw_list,
                &db_source_conn, &db_dest_conn)) {
        PyErr_BadArgument();
        return NULL;
    }

    if (!is_sqlite_object(self, db_source_conn)) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(
                       PyExc_TypeError,
                       "Given source object is not a sqlite database");
        }
        return NULL;
    }
    if (!is_sqlite_object(self, db_dest_conn)) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(
                       PyExc_TypeError,
                       "Given destination object is not a sqlite database");
        }
        return NULL;
    }


    db_source = ((pysqlite_Connection *)db_source_conn)->db;
    db_dest = ((pysqlite_Connection *)db_dest_conn)->db;
    if (db_source == NULL) {
        PyErr_SetString(PyExc_Exception, "Cannot set source database");
        return NULL;
    }
    if (db_dest == NULL) {
        PyErr_SetString(PyExc_Exception, "Cannot set destination database");
        return NULL;
    }
    if (((pysqlite_Connection *)db_source_conn)->inTransaction) {
        PyErr_Format(
                ((pysqlite_Connection *)db_source_conn)->DatabaseError,
                "Database in transaction");
        return NULL;
    }
    rc = copy_database(db_dest, db_source);
    if (rc != SQLITE_OK) {
        PyErr_Format(
                ((pysqlite_Connection *)db_dest_conn)->DatabaseError,
                "Database copy fail: %s (%d)", sqlite3_errmsg(db_dest), rc);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef sqlitebck_methods[] = {
    {"copy", (PyCFunction)py_copy, METH_VARARGS|METH_KEYWORDS,
        "Copy any sqlite3 database into given destination"},

    {NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION >= 3

static int sqlitebck_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    Py_VISIT(GETSTATE(m)->connection_type);
    return 0;
}

static int sqlitebck_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    Py_CLEAR(GETSTATE(m)->connection_type);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "sqlitebck",
        NULL,
        sizeof(struct module_state),
        sqlitebck_methods,
        NULL,
        sqlitebck_traverse,
        sqlitebck_clear,
        NULL
};
#define INITERROR return NULL
#else /* PY_MAJOR_VERSION < 3 */
#define INITERROR return
#endif


#if PY_MAJOR_VERSION >= 3
PyObject *
PyInit_sqlitebck(void)
#else /* PY_MAJOR_VERSION < 3 */
void
initsqlitebck(void)
#endif
{
    PyObject *module;
    PyObject *sqlite3_module;
    struct module_state *st;

#if PY_MAJOR_VERSION >= 3
    module = PyModule_Create(&moduledef);
#else /* PY_MAJOR_VERSION < 3 */
    module = Py_InitModule("sqlitebck", sqlitebck_methods);
#endif

    if (module == NULL) {
        INITERROR;
    }
    if (!(sqlite3_module = PyImport_ImportModule("sqlite3"))) {
        Py_DECREF(module);
        INITERROR;
    }

    st = GETSTATE(module);

    st->connection_type = PyObject_GetAttrString(sqlite3_module, "Connection");
    Py_DECREF(sqlite3_module);
    if (!st->connection_type) {
        Py_DECREF(module);
        INITERROR;
    }

    st->error = PyErr_NewException("sqlitebck.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
