// Unless explicitly stated otherwise all files in this repository are licensed
// under the Apache License Version 2.0.
// This product includes software developed at Datadog (https://www.datadoghq.com/).
// Copyright 2019 Datadog, Inc.
#include "two.h"
#include "constants.h"

#include <iostream>


Two::~Two()
{
    Py_Finalize();
}

void Two::init(const char* pythonHome) {
    if (pythonHome != NULL) {
        _pythonHome = pythonHome;
    }

    // TODO: is this a good idea? Py_NoSiteFlag = 1;
    Py_SetPythonHome(const_cast<char *>(_pythonHome));
    Py_Initialize();

    PyModules::iterator it;
    for (it = _modules.begin(); it != _modules.end(); ++it) {
        Py_InitModule(it->first.c_str(), &_modules[it->first][0]);
    }
}

bool Two::isInitialized() const
{
    return Py_IsInitialized();
}

const char* Two::getPyVersion() const
{
    return Py_GetVersion();
}

int Two::runSimpleFile(const char* path) const
{
    FILE* fp = fopen(path, "r");
    if (!fp) {
        std::cerr << "error opening file: " << path << std::endl;
        return -1;
    }

    return PyRun_SimpleFileEx(fp, path, 1);  // automatically closes the file
}

void Two::addModuleFunction(const char* module, const char* funcName,
                            void* func, MethType t)
{
    int ml_flags;

    switch (t) {
        case Six::NOARGS:
            ml_flags = METH_NOARGS;
            break;
        case Six::ARGS:
            ml_flags = METH_VARARGS;
            break;
        case Six::KEYWORDS:
            ml_flags = METH_VARARGS | METH_KEYWORDS;
            break;
    }

    PyMethodDef def = {
        funcName,
        (PyCFunction)func,
        ml_flags,
        ""
    };

    PyMethodDef guard = {NULL, NULL};

    if (_modules.find(module) == _modules.end()) {
        _modules[module] = PyMethods();
        // add the guard
        _modules[module].push_back(guard);
    }

    // insert at beginning so we keep guard at the end
    _modules[module].insert(_modules[module].begin(), def);
}