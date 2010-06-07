#include "Python.h"
/* from python-x.x/Modules/_sqlite */
#include "connection.h"
#include "sqlite3.h"


/* 
 * docs for backup api: 
 * from http://www.sqlite.org/backup.html
 *
 * Important: used interface is experimental and is known to be changed
 * without notice.
 */


static PyObject*
py_save(PyObject *self, PyObject *args, PyObject *kwds)
{
    int ret_code;
    const char *dest_filename;
    PyObject *db_source_conn;
    static char *kw_list[] = {"db", "dest", NULL};
    
    sqlite3 *db_source = NULL;
    sqlite3 *db_dest = NULL;
    sqlite3_backup *db_bck = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os", kw_list,
                &db_source_conn, &dest_filename)) {
        PyErr_BadArgument();
        return NULL;
    }
    /* TOOD: type checking? */
    db_source = ((pysqlite_Connection *)db_source_conn)->db;
    if (db_source == NULL) {
        PyErr_SetString(PyExc_Exception, "Can't find source db");
        return NULL;
    }
    ret_code = sqlite3_open(dest_filename, &db_dest);
    if (ret_code != SQLITE_OK) {
        PyErr_Format(PyExc_Exception, 
                "Database open fail: %s (%d)", 
                sqlite3_errmsg(db_dest), ret_code);
        return NULL;
    }
    db_bck = sqlite3_backup_init(db_dest, "main", db_source, "main");
    if(db_bck == NULL){
        PyErr_Format(PyExc_Exception, 
                "Backup initialization fail: %s (%d)", 
                sqlite3_errmsg(db_dest), sqlite3_errcode(db_dest));
        return NULL;
    }
    ret_code = sqlite3_backup_step(db_bck, -1);
    if (ret_code != SQLITE_OK) {
        PyErr_Format(PyExc_Exception, 
                "Database copy error: %s (%d)", 
                sqlite3_errmsg(db_dest), ret_code);
        return NULL;
    }
    ret_code = sqlite3_backup_finish(db_bck);
    /* ret_code = sqlite3_errcode(db_dest); */
    if (ret_code != SQLITE_OK) {
        PyErr_Format(PyExc_Exception, 
                "Destination database close error: %s (%d)", 
                sqlite3_errmsg(db_dest), ret_code);
        return NULL;
    }
    ret_code = sqlite3_close(db_dest);
    if (ret_code != SQLITE_OK) {
        PyErr_Format(PyExc_Exception, 
                "Database close error: %s (%d)", 
                sqlite3_errmsg(db_dest), ret_code);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef sqlitebck_methods[] = {
    {"save", (PyCFunction)py_save, METH_VARARGS|METH_KEYWORDS,
         "Backup any sqlite3 database"},

    {NULL, NULL, 0, NULL}
};

void
initsqlitebck(void)
{
  Py_InitModule("sqlitebck", sqlitebck_methods);
}

