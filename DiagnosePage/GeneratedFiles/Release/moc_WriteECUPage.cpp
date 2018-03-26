/****************************************************************************
** Meta object code from reading C++ file 'WriteECUPage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../WriteECUPage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WriteECUPage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_WriteECUPage_t {
    QByteArrayData data[6];
    char stringdata0[78];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WriteECUPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WriteECUPage_t qt_meta_stringdata_WriteECUPage = {
    {
QT_MOC_LITERAL(0, 0, 12), // "WriteECUPage"
QT_MOC_LITERAL(1, 13, 18), // "input_resultSignal"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 17), // "InputWriteECUData"
QT_MOC_LITERAL(4, 51, 9), // "Sure_Slot"
QT_MOC_LITERAL(5, 61, 16) // "close_windowSlot"

    },
    "WriteECUPage\0input_resultSignal\0\0"
    "InputWriteECUData\0Sure_Slot\0"
    "close_windowSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WriteECUPage[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   32,    2, 0x0a /* Public */,
       5,    0,   33,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void WriteECUPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        WriteECUPage *_t = static_cast<WriteECUPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->input_resultSignal((*reinterpret_cast< InputWriteECUData(*)>(_a[1]))); break;
        case 1: _t->Sure_Slot(); break;
        case 2: _t->close_windowSlot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< InputWriteECUData >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (WriteECUPage::*_t)(InputWriteECUData );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&WriteECUPage::input_resultSignal)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject WriteECUPage::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_WriteECUPage.data,
      qt_meta_data_WriteECUPage,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *WriteECUPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WriteECUPage::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_WriteECUPage.stringdata0))
        return static_cast<void*>(const_cast< WriteECUPage*>(this));
    return QDialog::qt_metacast(_clname);
}

int WriteECUPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void WriteECUPage::input_resultSignal(InputWriteECUData _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
