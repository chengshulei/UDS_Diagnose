/****************************************************************************
** Meta object code from reading C++ file 'ReadRealTimeData.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ReadRealTimeData.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ReadRealTimeData.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ReadRealTimeData_t {
    QByteArrayData data[15];
    char stringdata0[251];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ReadRealTimeData_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ReadRealTimeData_t qt_meta_stringdata_ReadRealTimeData = {
    {
QT_MOC_LITERAL(0, 0, 16), // "ReadRealTimeData"
QT_MOC_LITERAL(1, 17, 31), // "send_realtimedata_result_Signal"
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 8), // "TreeData"
QT_MOC_LITERAL(4, 59, 21), // "display_messageSignal"
QT_MOC_LITERAL(5, 81, 19), // "stop_refresh_Signal"
QT_MOC_LITERAL(6, 101, 29), // "send_realtimedata_result_Slot"
QT_MOC_LITERAL(7, 131, 14), // "temp_tree_data"
QT_MOC_LITERAL(8, 146, 17), // "Check_Signal_Slot"
QT_MOC_LITERAL(9, 164, 18), // "Start_Refresh_Slot"
QT_MOC_LITERAL(10, 183, 17), // "stop_refresh_Slot"
QT_MOC_LITERAL(11, 201, 15), // "treeItemChanged"
QT_MOC_LITERAL(12, 217, 14), // "QStandardItem*"
QT_MOC_LITERAL(13, 232, 4), // "item"
QT_MOC_LITERAL(14, 237, 13) // "ReadData_Slot"

    },
    "ReadRealTimeData\0send_realtimedata_result_Signal\0"
    "\0TreeData\0display_messageSignal\0"
    "stop_refresh_Signal\0send_realtimedata_result_Slot\0"
    "temp_tree_data\0Check_Signal_Slot\0"
    "Start_Refresh_Slot\0stop_refresh_Slot\0"
    "treeItemChanged\0QStandardItem*\0item\0"
    "ReadData_Slot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ReadRealTimeData[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    2,   62,    2, 0x06 /* Public */,
       5,    0,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   68,    2, 0x0a /* Public */,
       8,    0,   71,    2, 0x0a /* Public */,
       9,    0,   72,    2, 0x0a /* Public */,
      10,    0,   73,    2, 0x0a /* Public */,
      11,    1,   74,    2, 0x0a /* Public */,
      14,    0,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void,

       0        // eod
};

void ReadRealTimeData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ReadRealTimeData *_t = static_cast<ReadRealTimeData *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->send_realtimedata_result_Signal((*reinterpret_cast< TreeData(*)>(_a[1]))); break;
        case 1: _t->display_messageSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->stop_refresh_Signal(); break;
        case 3: _t->send_realtimedata_result_Slot((*reinterpret_cast< TreeData(*)>(_a[1]))); break;
        case 4: _t->Check_Signal_Slot(); break;
        case 5: _t->Start_Refresh_Slot(); break;
        case 6: _t->stop_refresh_Slot(); break;
        case 7: _t->treeItemChanged((*reinterpret_cast< QStandardItem*(*)>(_a[1]))); break;
        case 8: _t->ReadData_Slot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< TreeData >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< TreeData >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ReadRealTimeData::*_t)(TreeData );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadRealTimeData::send_realtimedata_result_Signal)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ReadRealTimeData::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadRealTimeData::display_messageSignal)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ReadRealTimeData::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ReadRealTimeData::stop_refresh_Signal)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject ReadRealTimeData::staticMetaObject = {
    { &PageInit::staticMetaObject, qt_meta_stringdata_ReadRealTimeData.data,
      qt_meta_data_ReadRealTimeData,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ReadRealTimeData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReadRealTimeData::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ReadRealTimeData.stringdata0))
        return static_cast<void*>(const_cast< ReadRealTimeData*>(this));
    return PageInit::qt_metacast(_clname);
}

int ReadRealTimeData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PageInit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void ReadRealTimeData::send_realtimedata_result_Signal(TreeData _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ReadRealTimeData::display_messageSignal(QString _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ReadRealTimeData::stop_refresh_Signal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
