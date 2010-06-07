#include "Python.h"
/* header from Python-X.X/Modules/_sqlite */
#include "connection.h"
#include "sqlite3.h"


/* 
 * docs for sqlite3 backup api: 
 * from http://www.sqlite.org/backup.html
 *
 * Important: used interface is experimental and is known to be changed
 * without notice.
 */


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
    /* TOOD: type checking? */
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
    rc = copy_database(db_dest, db_source);
    if (rc != SQLITE_OK) {
        PyErr_Format(PyExc_Exception, 
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

void
initsqlitebck(void)
{
    Py_InitModule("sqlitebck", sqlitebck_methods);
}
