/****************************************************************************
** Meta object code from reading C++ file 'RoutineControl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RoutineControl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RoutineControl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_RoutineControl_t {
    QByteArrayData data[14];
    char stringdata0[265];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RoutineControl_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RoutineControl_t qt_meta_stringdata_RoutineControl = {
    {
QT_MOC_LITERAL(0, 0, 14), // "RoutineControl"
QT_MOC_LITERAL(1, 15, 30), // "SendCommunicationControlSignal"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 34), // "communicationcontrol_result_S..."
QT_MOC_LITERAL(4, 82, 21), // "display_messageSignal"
QT_MOC_LITERAL(5, 104, 31), // "communicationcontrolButtontSlot"
QT_MOC_LITERAL(6, 136, 32), // "communicationcontrol_result_Slot"
QT_MOC_LITERAL(7, 169, 4), // "flag"
QT_MOC_LITERAL(8, 174, 13), // "function_name"
QT_MOC_LITERAL(9, 188, 18), // "start_operate_Slot"
QT_MOC_LITERAL(10, 207, 16), // "end_operate_Slot"
QT_MOC_LITERAL(11, 224, 14), // "ChangeModeSlot"
QT_MOC_LITERAL(12, 239, 8), // "temp_row"
QT_MOC_LITERAL(13, 248, 16) // "start_learn_Slot"

    },
    "RoutineControl\0SendCommunicationControlSignal\0"
    "\0communicationcontrol_result_Signal\0"
    "display_messageSignal\0"
    "communicationcontrolButtontSlot\0"
    "communicationcontrol_result_Slot\0flag\0"
    "function_name\0start_operate_Slot\0"
    "end_operate_Slot\0ChangeModeSlot\0"
    "temp_row\0start_learn_Slot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RoutineControl[] = {

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
       3,    2,   62,    2, 0x06 /* Public */,
       4,    2,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   72,    2, 0x0a /* Public */,
       6,    2,   73,    2, 0x0a /* Public */,
       9,    0,   78,    2, 0x0a /* Public */,
      10,    0,   79,    2, 0x0a /* Public */,
      11,    1,   80,    2, 0x0a /* Public */,
      13,    0,   83,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,

       0        // eod
};

void RoutineControl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RoutineControl *_t = static_cast<RoutineControl *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->SendCommunicationControlSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->communicationcontrol_result_Signal((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->display_messageSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->communicationcontrolButtontSlot(); break;
        case 4: _t->communicationcontrol_result_Slot((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->start_operate_Slot(); break;
        case 6: _t->end_operate_Slot(); break;
        case 7: _t->ChangeModeSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->start_learn_Slot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RoutineControl::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RoutineControl::SendCommunicationControlSignal)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (RoutineControl::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RoutineControl::communicationcontrol_result_Signal)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (RoutineControl::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RoutineControl::display_messageSignal)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject RoutineControl::staticMetaObject = {
    { &PageInit::staticMetaObject, qt_meta_stringdata_RoutineControl.data,
      qt_meta_data_RoutineControl,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *RoutineControl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RoutineControl::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_RoutineControl.stringdata0))
        return static_cast<void*>(const_cast< RoutineControl*>(this));
    return PageInit::qt_metacast(_clname);
}

int RoutineControl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void RoutineControl::SendCommunicationControlSignal(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RoutineControl::communicationcontrol_result_Signal(bool _t1, QString _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RoutineControl::display_messageSignal(QString _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
