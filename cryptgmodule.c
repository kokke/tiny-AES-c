#include <Python.h>
#include <stdint.h>
#include "aes.h"

static PyObject *
encrypt_ige(PyObject *self, PyObject *args)
{
    struct AES_ctx ctx;
    const uint8_t *plain, *key, *iv;
    const int plain_len;
    int dummy;

    if (!PyArg_ParseTuple(args, "y#y#y#",
                          &plain, &plain_len, &key, &dummy, &iv, &dummy))
        return NULL;

    AES_init_ctx_iv32(&ctx, key, iv);
    AES_IGE_encrypt_buffer(&ctx, plain, plain_len);

    return Py_BuildValue("y#", plain, plain_len);
}

static PyObject *
decrypt_ige(PyObject *self, PyObject *args)
{
    struct AES_ctx ctx;
    const uint8_t *cipher, *key, *iv;
    const int cipher_len;
    int dummy;

    if (!PyArg_ParseTuple(args, "y#y#y#",
                          &cipher, &cipher_len, &key, &dummy, &iv, &dummy))
        return NULL;

    AES_init_ctx_iv32(&ctx, key, iv);
    AES_IGE_decrypt_buffer(&ctx, cipher, cipher_len);

    return Py_BuildValue("y#", cipher, cipher_len);
}

static PyMethodDef CryptgMethods[] = {
    {"encrypt_ige", encrypt_ige, METH_VARARGS,
     "Encrypts the input plain text with the 32 bytes key and IV."},

    {"decrypt_ige", decrypt_ige, METH_VARARGS,
     "Decrypts the input cipher text with the 32 bytes key and IV."},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cryptgmodule = {
    PyModuleDef_HEAD_INIT,
    "cryptg",
    "Cryptographic utilities for Telegram",
    -1,
    CryptgMethods
};

PyMODINIT_FUNC
PyInit_cryptg(void)
{
    PyObject *m;

    m = PyModule_Create(&cryptgmodule);
    if (m == NULL)
        return NULL;

    return m;
}
