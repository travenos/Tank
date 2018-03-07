/****************************************************************************
** Meta object code from reading C++ file 'applicationui.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/applicationui.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'applicationui.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ApplicationUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      49,   41,   14,   14, 0x08,
      76,   70,   14,   14, 0x08,
     124,   14,   14,   14, 0x08,
     140,   14,   14,   14, 0x08,
     165,   14,   14,   14, 0x08,
     193,  187,   14,   14, 0x08,
     238,  187,   14,   14, 0x08,
     285,  187,   14,   14, 0x08,
     332,  187,   14,   14, 0x08,
     380,   70,   14,   14, 0x08,
     408,  187,   14,   14, 0x08,
     453,  187,   14,   14, 0x08,
     499,   14,   14,   14, 0x08,
     520,   14,   14,   14, 0x08,
     538,   14,   14,   14, 0x08,
     553,   14,   14,   14, 0x08,
     577,  572,   14,   14, 0x08,
     611,  572,   14,   14, 0x08,
     647,   41,   14,   14, 0x08,
     666,   14,   14,   14, 0x08,
     682,   14,   14,   14, 0x08,
     704,  698,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ApplicationUI[] = {
    "ApplicationUI\0\0onSystemLanguageChanged()\0"
    "message\0ShowMessage(QString)\0value\0"
    "onMsgFinished(bb::system::SystemUiResult::Type)\0"
    "loadInterface()\0onbuttonConnectClicked()\0"
    "onSettingsTriggered()\0event\0"
    "onbuttonUpClicked(bb::cascades::TouchEvent*)\0"
    "onbuttonDownClicked(bb::cascades::TouchEvent*)\0"
    "onbuttonLeftClicked(bb::cascades::TouchEvent*)\0"
    "onbuttonRightClicked(bb::cascades::TouchEvent*)\0"
    "onSliderValueChanged(float)\0"
    "onKeyPressedHandler(bb::cascades::KeyEvent*)\0"
    "onKeyReleasedHandler(bb::cascades::KeyEvent*)\0"
    "onDisconnectAction()\0onMessageAction()\0"
    "onAutoAction()\0onShutdownAction()\0"
    "text\0onipTextFieldtextChanged(QString)\0"
    "onportTextFieldtextChanged(QString)\0"
    "slotWrite(QString)\0slotConnected()\0"
    "slotReadyRead()\0error\0"
    "slotError(QAbstractSocket::SocketError)\0"
};

void ApplicationUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ApplicationUI *_t = static_cast<ApplicationUI *>(_o);
        switch (_id) {
        case 0: _t->onSystemLanguageChanged(); break;
        case 1: _t->ShowMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->onMsgFinished((*reinterpret_cast< bb::system::SystemUiResult::Type(*)>(_a[1]))); break;
        case 3: _t->loadInterface(); break;
        case 4: _t->onbuttonConnectClicked(); break;
        case 5: _t->onSettingsTriggered(); break;
        case 6: _t->onbuttonUpClicked((*reinterpret_cast< bb::cascades::TouchEvent*(*)>(_a[1]))); break;
        case 7: _t->onbuttonDownClicked((*reinterpret_cast< bb::cascades::TouchEvent*(*)>(_a[1]))); break;
        case 8: _t->onbuttonLeftClicked((*reinterpret_cast< bb::cascades::TouchEvent*(*)>(_a[1]))); break;
        case 9: _t->onbuttonRightClicked((*reinterpret_cast< bb::cascades::TouchEvent*(*)>(_a[1]))); break;
        case 10: _t->onSliderValueChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 11: _t->onKeyPressedHandler((*reinterpret_cast< bb::cascades::KeyEvent*(*)>(_a[1]))); break;
        case 12: _t->onKeyReleasedHandler((*reinterpret_cast< bb::cascades::KeyEvent*(*)>(_a[1]))); break;
        case 13: _t->onDisconnectAction(); break;
        case 14: _t->onMessageAction(); break;
        case 15: _t->onAutoAction(); break;
        case 16: _t->onShutdownAction(); break;
        case 17: _t->onipTextFieldtextChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 18: _t->onportTextFieldtextChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 19: _t->slotWrite((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 20: _t->slotConnected(); break;
        case 21: _t->slotReadyRead(); break;
        case 22: _t->slotError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ApplicationUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ApplicationUI::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ApplicationUI,
      qt_meta_data_ApplicationUI, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ApplicationUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ApplicationUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ApplicationUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ApplicationUI))
        return static_cast<void*>(const_cast< ApplicationUI*>(this));
    return QObject::qt_metacast(_clname);
}

int ApplicationUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
