/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>

#include <QSettings>
#include <QtNetwork/QTcpSocket>

#include <bb/cascades/TouchEnterEvent>
#include <bb/cascades/Button>
#include <bb/cascades/ImageButton>
#include <bb/cascades/Container>
#include <bb/cascades/StackLayout>
#include <bb/cascades/DockLayout>
#include <bb/cascades/Color>
#include <bb/cascades/Page>
#include <bb/cascades/Menu>
#include <bb/cascades/SettingsActionItem>
#include <bb/cascades/ActionItem>
#include <bb/cascades/TitleBar>
#include <bb/cascades/TitleBarKind>
#include <bb/cascades/Label>
#include <bb/cascades/TextField>
#include <bb/cascades/NavigationFocusPolicy>
#include <bb/cascades/Slider>
#include <bb/cascades/KeyEvent>
#include <bb/cascades/KeyListener>

#include <bb/system/SystemPrompt>
#include <bb/system/SystemUiResult>
#include <bb/system/SystemToast>
#include <bb/system/SystemUiPosition>

namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
    }
}

class QTranslator;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
public:
    ApplicationUI();
    virtual ~ApplicationUI() {}
private slots:
    void onSystemLanguageChanged();

    void ShowMessage(QString message);
    void onMsgFinished(bb::system::SystemUiResult::Type value);
    //Main interface
    void loadInterface();
    void onbuttonConnectClicked();
    void onSettingsTriggered();

    void onbuttonUpClicked(bb::cascades::TouchEvent *event);
    void onbuttonDownClicked(bb::cascades::TouchEvent *event);
    void onbuttonLeftClicked(bb::cascades::TouchEvent *event);
    void onbuttonRightClicked(bb::cascades::TouchEvent *event);
    void onSliderValueChanged(float value);

    void onKeyPressedHandler(bb::cascades::KeyEvent *event);
    void onKeyReleasedHandler(bb::cascades::KeyEvent *event);

    void onDisconnectAction();
    void onMessageAction();
    void onAutoAction();
    void onShutdownAction();
    void onStopAction();

    //Settings
    void onipTextFieldtextChanged (QString text);
    void onportTextFieldtextChanged (QString text);
    //TCPClient
    void slotWrite(QString message);
    void slotConnected();
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError error);
private:
    QTranslator* m_pTranslator;
    QTcpSocket* m_pTcpSocket;

    bb::cascades::LocaleHandler* m_pLocaleHandler;

    bb::cascades::Page *page;
    bb::cascades::ImageButton *buttonUp;
    bb::cascades::ImageButton *buttonDown;
    bb::cascades::ImageButton *buttonLeft;
    bb::cascades::ImageButton *buttonRight;
    bb::cascades::Slider *slider;
    bb::cascades::Label *LabelSpeed;
    bb::cascades::Button *buttonConnect;

    bb::system::SystemToast* m_toast;
    bb::system::SystemPrompt* m_prompt;

    bb::cascades::Menu *menu;
    //Menu bar
    bb::cascades::ActionItem* disconnectAction;
    bb::cascades::ActionItem* messageAction;
    bb::cascades::ActionItem* autoAction;
    bb::cascades::ActionItem* shutdownAction;
    bb::cascades::ActionItem* stopAction;

    QString ip;
    int port;

    int currentSpeed;

    bool isTracking;
    bool isConnected;

    void goStraight(bool go);
    void goBack(bool go);
    void goLeft(bool go);
    void goRight(bool go);
    void setControlsEnabled(bool set);
};

#endif /* ApplicationUI_HPP_ */
