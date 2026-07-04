#include <lib/python/connections.h>
#include <lib/python/python.h>
#include <lib/base/nconfig.h>

void __attribute__((visibility("default"))) PSignal::callPython(ePyObject tuple)
{
	if (!Py_IsInitialized())
		return;
	PyGILState_STATE state = PyGILState_Ensure();
	ePyObject list = get();
	if (list && PyList_Check(list))
	{
		int size = PyList_Size(list);
		for (int i = 0; i < size; ++i)
		{
			ePyObject item = PyList_GetItem(list, i);
			if (PyTuple_Check(item))
			{
				ePyObject obj = PyTuple_GetItem(item, 0);
				if (PyCallable_Check(obj))
				{
					ePyObject res = PyObject_CallObject(obj, tuple);
					if (res)
						Org_Py_DECREF(res);
					else
						PyErr_Print();
				}
			}
			else if (PyCallable_Check(item))
			{
				ePyObject res = PyObject_CallObject(item, tuple);
				if (res)
					Org_Py_DECREF(res);
				else
					PyErr_Print();
			}
		}
	}
	PyGILState_Release(state);
}

PSignal::PSignal()
{
	m_list = PyList_New(0);
}

PSignal::~PSignal()
{
	if (m_list)
	{
		ePyObject list = m_list;
		m_list = (PyObject*)NULL;
		if (Py_IsInitialized())
		{
			PyGILState_STATE state = PyGILState_Ensure();
			Org_Py_DECREF(list);
			PyGILState_Release(state);
		}
	}
}

PyObject *PSignal::get()
{
	return m_list;
}

PyObject *PSignal::getSteal(bool clear)
{
	PyObject *res = m_list;
	if (clear)
		m_list = PyList_New(0);
	return res;
}

// eConfigManager implementations to satisfy missing symbols in DreamOS Enigma2
eConfigManager::eConfigManager() {}
eConfigManager::~eConfigManager() {}

std::string eConfigManager::getConfigValue(const char *key)
{
	return "";
}

int eConfigManager::getConfigIntValue(const char *key, int defaultvalue)
{
	return defaultvalue;
}

bool eConfigManager::getConfigBoolValue(const char *key, bool defaultvalue)
{
	return defaultvalue;
}
