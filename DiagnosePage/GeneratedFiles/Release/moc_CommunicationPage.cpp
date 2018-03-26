/****************************************************************************
** Meta object code from reading C++ file 'CommunicationPage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../CommunicationPage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CommunicationPage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CommunicationPage_t {
    QByteArrayData data[6];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CommunicationPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CommunicationPage_t qt_meta_stringdata_CommunicationPage = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CommunicationPage"
QT_MOC_LITERAL(1, 18, 26), // "communication_resultSignal"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 12), // "Exist_Device"
QT_MOC_LITERAL(4, 59, 9), // "Sure_Slot"
QT_MOC_LITERAL(5, 69, 16) // "close_windowSlot"

    },
    "CommunicationPage\0communication_resultSignal\0"
    "\0Exist_Device\0Sure_Slot\0close_windowSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CommunicationPage[] = {

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

void CommunicationPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CommunicationPage *_t = static_cast<CommunicationPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->communication_resultSignal((*reinterpret_cast< Exist_Device(*)>(_a[1]))); break;
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
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Exist_Device >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CommunicationPage::*_t)(Exist_Device );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CommunicationPage::communication_resultSignal)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject CommunicationPage::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CommunicationPage.data,
      qt_meta_data_CommunicationPage,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CommunicationPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CommunicationPage::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CommunicationPage.stringdata0))
        return static_cast<void*>(const_cast< CommunicationPage*>(this));
    return QDialog::qt_metacast(_clname);
}

int CommunicationPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CommunicationPage::communication_resultSignal(Exist_Device _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
