/****************************************************************************
** Meta object code from reading C++ file 'SynthGUI.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SynthGUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SynthGUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SynthGUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   10,    9,    9, 0x0a,
      42,   33,    9,    9, 0x0a,
      69,   61,    9,    9, 0x0a,
      94,   87,    9,    9, 0x0a,
     111,    9,    9,    9, 0x0a,
     125,    9,    9,    9, 0x09,
     144,    9,    9,    9, 0x09,
     165,    9,    9,    9, 0x09,
     185,    9,    9,    9, 0x09,
     204,    9,    9,    9, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_SynthGUI[] = {
    "SynthGUI\0\0cents\0setDetune(float)\0"
    "waveForm\0setWaveForm(float)\0release\0"
    "setRelease(float)\0volume\0setVolume(float)\0"
    "aboutToQuit()\0detuneChanged(int)\0"
    "waveFormChanged(int)\0releaseChanged(int)\0"
    "volumeChanged(int)\0oscRecv()\0"
};

void SynthGUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SynthGUI *_t = static_cast<SynthGUI *>(_o);
        switch (_id) {
        case 0: _t->setDetune((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->setWaveForm((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->setRelease((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->setVolume((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: _t->aboutToQuit(); break;
        case 5: _t->detuneChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->waveFormChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->releaseChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->volumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->oscRecv(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SynthGUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SynthGUI::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_SynthGUI,
      qt_meta_data_SynthGUI, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SynthGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SynthGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SynthGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SynthGUI))
        return static_cast<void*>(const_cast< SynthGUI*>(this));
    return QFrame::qt_metacast(_clname);
}

int SynthGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE