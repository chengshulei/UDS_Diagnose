/****************************************************************************
** Meta object code from reading C++ file 'SelectModelPage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SelectModelPage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SelectModelPage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_SelectModelPage_t {
    QByteArrayData data[6];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SelectModelPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SelectModelPage_t qt_meta_stringdata_SelectModelPage = {
    {
QT_MOC_LITERAL(0, 0, 15), // "SelectModelPage"
QT_MOC_LITERAL(1, 16, 17), // "selectmodelsignal"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 9), // "ModelInfo"
QT_MOC_LITERAL(4, 45, 21), // "display_messageSignal"
QT_MOC_LITERAL(5, 67, 15) // "SelectModelSlot"

    },
    "SelectModelPage\0selectmodelsignal\0\0"
    "ModelInfo\0display_messageSignal\0"
    "SelectModelSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SelectModelPage[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    2,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   37,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    2,    2,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void SelectModelPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SelectModelPage *_t = static_cast<SelectModelPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectmodelsignal((*reinterpret_cast< ModelInfo(*)>(_a[1]))); break;
        case 1: _t->display_messageSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->SelectModelSlot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SelectModelPage::*_t)(ModelInfo );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SelectModelPage::selectmodelsignal)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (SelectModelPage::*_t)(QString , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SelectModelPage::display_messageSignal)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject SelectModelPage::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_SelectModelPage.data,
      qt_meta_data_SelectModelPage,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SelectModelPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SelectModelPage::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SelectModelPage.stringdata0))
        return static_cast<void*>(const_cast< SelectModelPage*>(this));
    return QDialog::qt_metacast(_clname);
}

int SelectModelPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void SelectModelPage::selectmodelsignal(ModelInfo _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SelectModelPage::display_messageSignal(QString _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
