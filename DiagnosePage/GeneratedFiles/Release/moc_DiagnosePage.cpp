/****************************************************************************
** Meta object code from reading C++ file 'DiagnosePage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../DiagnosePage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DiagnosePage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DiagnosePage_t {
    QByteArrayData data[20];
    char stringdata0[322];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DiagnosePage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DiagnosePage_t qt_meta_stringdata_DiagnosePage = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DiagnosePage"
QT_MOC_LITERAL(1, 13, 17), // "receivedataSignal"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 19), // "DiagnoseEnterSiganl"
QT_MOC_LITERAL(4, 52, 21), // "display_messageSignal"
QT_MOC_LITERAL(5, 74, 24), // "stop_realtimedata_Siganl"
QT_MOC_LITERAL(6, 99, 22), // "ReceiveSelectModelSlot"
QT_MOC_LITERAL(7, 122, 9), // "ModelInfo"
QT_MOC_LITERAL(8, 132, 18), // "selected_modelinfo"
QT_MOC_LITERAL(9, 151, 17), // "DiagnoseEnterSlot"
QT_MOC_LITERAL(10, 169, 14), // "ChangeModeSlot"
QT_MOC_LITERAL(11, 184, 8), // "temp_row"
QT_MOC_LITERAL(12, 193, 10), // "TableWrite"
QT_MOC_LITERAL(13, 204, 15), // "message_content"
QT_MOC_LITERAL(14, 220, 12), // "message_type"
QT_MOC_LITERAL(15, 233, 24), // "communication_resultSlot"
QT_MOC_LITERAL(16, 258, 12), // "Exist_Device"
QT_MOC_LITERAL(17, 271, 18), // "temp_commu_setting"
QT_MOC_LITERAL(18, 290, 16), // "close_windowSlot"
QT_MOC_LITERAL(19, 307, 14) // "min_windowSlot"

    },
    "DiagnosePage\0receivedataSignal\0\0"
    "DiagnoseEnterSiganl\0display_messageSignal\0"
    "stop_realtimedata_Siganl\0"
    "ReceiveSelectModelSlot\0ModelInfo\0"
    "selected_modelinfo\0DiagnoseEnterSlot\0"
    "ChangeModeSlot\0temp_row\0TableWrite\0"
    "message_content\0message_type\0"
    "communication_resultSlot\0Exist_Device\0"
    "temp_commu_setting\0close_windowSlot\0"
    "min_windowSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DiagnosePage[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,
       3,    0,   70,    2, 0x06 /* Public */,
       4,    2,   71,    2, 0x06 /* Public */,
       5,    0,   76,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   77,    2, 0x0a /* Public */,
       9,    0,   80,    2, 0x0a /* Public */,
      10,    1,   81,    2, 0x0a /* Public */,
      12,    2,   84,    2, 0x0a /* Public */,
      15,    1,   89,    2, 0x0a /* Public */,
      18,    0,   92,    2, 0x0a /* Public */,
      19,    0,   93,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,   13,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DiagnosePage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DiagnosePage *_t = static_cast<DiagnosePage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->receivedataSignal(); break;
        case 1: _t->DiagnoseEnterSiganl(); break;
        case 2: _t->display_messageSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->stop_realtimedata_Siganl(); break;
        case 4: _t->ReceiveSelectModelSlot((*reinterpret_cast< ModelInfo(*)>(_a[1]))); break;
        case 5: _t->DiagnoseEnterSlot(); break;
        case 6: _t->ChangeModeSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->TableWrite((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->communication_resultSlot((*reinterpret_cast< Exist_Device(*)>(_a[1]))); break;
        case 9: _t->close_windowSlot(); break;
        case 10: _t->min_windowSlot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Exist_Device >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DiagnosePage::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DiagnosePage::receivedataSignal)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DiagnosePage::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DiagnosePage::DiagnoseEnterSiganl)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DiagnosePage::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DiagnosePage::display_messageSignal)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DiagnosePage::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DiagnosePage::stop_realtimedata_Siganl)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject DiagnosePage::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_DiagnosePage.data,
      qt_meta_data_DiagnosePage,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DiagnosePage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DiagnosePage::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DiagnosePage.stringdata0))
        return static_cast<void*>(const_cast< DiagnosePage*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int DiagnosePage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void DiagnosePage::receivedataSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void DiagnosePage::DiagnoseEnterSiganl()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void DiagnosePage::display_messageSignal(QString _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DiagnosePage::stop_realtimedata_Siganl()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
