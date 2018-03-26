/****************************************************************************
** Meta object code from reading C++ file 'ReadECU.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ReadECU.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ReadECU.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ReadECU_t {
    QByteArrayData data[16];
    char stringdata0[273];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ReadECU_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ReadECU_t qt_meta_stringdata_ReadECU = {
    {
QT_MOC_LITERAL(0, 0, 7), // "ReadECU"
QT_MOC_LITERAL(1, 8, 22), // "send_ECU_result_Signal"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 12), // "ECUTableData"
QT_MOC_LITERAL(4, 45, 27), // "send_writeECU_result_Signal"
QT_MOC_LITERAL(5, 73, 21), // "display_messageSignal"
QT_MOC_LITERAL(6, 95, 20), // "send_ECU_result_Slot"
QT_MOC_LITERAL(7, 116, 18), // "temp_ecutable_data"
QT_MOC_LITERAL(8, 135, 26), // "edit_pushbutton_click_Slot"
QT_MOC_LITERAL(9, 162, 16), // "input_resultSlot"
QT_MOC_LITERAL(10, 179, 17), // "InputWriteECUData"
QT_MOC_LITERAL(11, 197, 18), // "input_writeecudata"
QT_MOC_LITERAL(12, 216, 25), // "send_writeECU_result_Slot"
QT_MOC_LITERAL(13, 242, 10), // "write_flag"
QT_MOC_LITERAL(14, 253, 14), // "writeitem_name"
QT_MOC_LITERAL(15, 268, 4) // "Init"

    },
    "ReadECU\0send_ECU_result_Signal\0\0"
    "ECUTableData\0send_writeECU_result_Signal\0"
    "display_messageSignal\0send_ECU_result_Slot\0"
    "temp_ecutable_data\0edit_pushbutton_click_Slot\0"
    "input_resultSlot\0InputWriteECUData\0"
    "input_writeecudata\0send_writeECU_result_Slot\0"
    "write_flag\0writeitem_name\0Init"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ReadECU[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    2,   57,    2, 0x06 /* Public */,
       5,    2,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   67,    2, 0x0a /* Public */,
       8,    0,   70,    2, 0x0a /* Public */,
       9,    1,   71,    2, 0x0a /* Public */,
      12,    2,   74,    2, 0x0a /* Public */,
      15,    0,   79,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    7,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   13,   14,
    QMetaType::Void,

       0        // eod
};

void ReadECU::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ReadECU *_t = static_cast<ReadECU *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->send_ECU_result_Signal((*reinterpret_cast< ECUTableData(*)>(_a[1]))); break;
        case 1: _t->send_writeECU_result_Signal((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->display_messageSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->send_ECU_result_Slot((*reinterpret_cast< ECUTableData(*)>(_a[1]))); break;
        case 4: _t->edit_pushbutton_click_Slot(); break;
        case 5: _t->input_resultSlot((*reinterpret_cast< InputWriteECUData(*)>(_a[1]))); break;
        case 6: _t->send_writeECU_result_Slot((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 7: _t->Init(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ECUTableData >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ECUTableData >(); break;
            }
            break;
        case 5:
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
            typedef void (ReadECU::*_t)(ECUTableData );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadECU::send_ECU_result_Signal)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ReadECU::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadECU::send_writeECU_result_Signal)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ReadECU::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadECU::display_messageSignal)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject ReadECU::staticMetaObject = {
    { &PageInit::staticMetaObject, qt_meta_stringdata_ReadECU.data,
      qt_meta_data_ReadECU,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ReadECU::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReadECU::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ReadECU.stringdata0))
        return static_cast<void*>(const_cast< ReadECU*>(this));
    return PageInit::qt_metacast(_clname);
}

int ReadECU::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PageInit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ReadECU::send_ECU_result_Signal(ECUTableData _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ReadECU::send_writeECU_result_Signal(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ReadECU::display_messageSignal(QString _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
