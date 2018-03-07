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

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>

using namespace bb::cascades;

ApplicationUI::ApplicationUI() :
        QObject()
{
    // prepare the localization
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    // initial load
    onSystemLanguageChanged();

    QSettings settings;
    ip=settings.value("ip","192.168.150.1").toString();
    port=settings.value("port", 8888).toInt();
    currentSpeed=180;
    isTracking=false;
    isConnected=false;

    //Settings menu
    menu= Menu::create()
        .settings(SettingsActionItem::create());
    menu->settingsAction()->setTitle("Settings");
    Application::instance()->setMenu(menu);
    connect(menu->settingsAction(),SIGNAL(triggered()),this,SLOT(onSettingsTriggered()));

    m_toast=NULL;

    m_pTcpSocket = new QTcpSocket(this);
    connect(m_pTcpSocket, SIGNAL(connected()),this, SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()),this, SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));

    loadInterface();
}

void ApplicationUI::loadInterface()
{
        //Button for connection to server
        buttonConnect=Button::create().text("Connect");
        buttonConnect->setParent(this);
        connect(buttonConnect,SIGNAL(clicked()),this,SLOT(onbuttonConnectClicked()));

        //Tank control buttons
        buttonUp=ImageButton::create()
        .defaultImage(QUrl("asset:///icons/up-default.png"))
        .pressedImage(QUrl("asset:///icons/up-pressed.png"))
        .disabledImage(QUrl("asset:///icons/up-disabled.png"));
        buttonUp->setPreferredHeight(150);
        buttonUp->setPreferredWidth(150);
        buttonUp->setParent(this);
        buttonUp->navigation()->setFocusPolicy(NavigationFocusPolicy::NotFocusable);
        connect(buttonUp,SIGNAL(touch (bb::cascades::TouchEvent* )),this,SLOT(onbuttonUpClicked(bb::cascades::TouchEvent* )));

        buttonDown=ImageButton::create()
        .defaultImage(QUrl("asset:///icons/down-default.png"))
        .pressedImage(QUrl("asset:///icons/down-pressed.png"))
        .disabledImage(QUrl("asset:///icons/down-disabled.png"));
        buttonDown->setPreferredHeight(150);
        buttonDown->setPreferredWidth(150);
        buttonDown->setParent(this);
        buttonDown->navigation()->setFocusPolicy(NavigationFocusPolicy::NotFocusable);
        connect(buttonDown,SIGNAL(touch (bb::cascades::TouchEvent* )),this,SLOT(onbuttonDownClicked(bb::cascades::TouchEvent* )));

        buttonLeft=ImageButton::create()
        .defaultImage(QUrl("asset:///icons/left-default.png"))
        .pressedImage(QUrl("asset:///icons/left-pressed.png"))
        .disabledImage(QUrl("asset:///icons/left-disabled.png"));
        buttonLeft->setPreferredHeight(150);
        buttonLeft->setPreferredWidth(150);
        buttonLeft->setParent(this);
        buttonLeft->navigation()->setFocusPolicy(NavigationFocusPolicy::NotFocusable);
        connect(buttonLeft,SIGNAL(touch (bb::cascades::TouchEvent* )),this,SLOT(onbuttonLeftClicked(bb::cascades::TouchEvent* )));

        buttonRight=ImageButton::create()
        .defaultImage(QUrl("asset:///icons/right-default.png"))
        .pressedImage(QUrl("asset:///icons/right-pressed.png"))
        .disabledImage(QUrl("asset:///icons/right-disabled.png"));
        buttonRight->setPreferredHeight(150);
        buttonRight->setPreferredWidth(150);
        buttonRight->setParent(this);
        buttonRight->navigation()->setFocusPolicy(NavigationFocusPolicy::NotFocusable);
        connect(buttonRight,SIGNAL(touch (bb::cascades::TouchEvent* )),this,SLOT(onbuttonRightClicked(bb::cascades::TouchEvent* )));

        slider = new Slider();
        slider->setFromValue(40);
        slider->setToValue(255);
        slider->setValue(currentSpeed);
        connect(slider, SIGNAL(immediateValueChanged(float)), this, SLOT(onSliderValueChanged(float)));

        LabelSpeed=Label::create()
            .text("Speed: "+QString::number(currentSpeed));

        //main container
        Container *contentContainer = new Container();

            // Get the UIConfig object to use resolution independent sizes
            UIConfig *ui = contentContainer->ui();
            contentContainer->setTopPadding(ui->du(2.0));
            contentContainer->setLeftPadding(ui->du(2.0));
            contentContainer->setRightPadding(ui->du(2.0));
            contentContainer->setLayout(DockLayout::create());

            //Containers for tank control buttons
            Container* ControlButtons = new Container();
            ControlButtons->setLayout(StackLayout::create().orientation(LayoutOrientation::TopToBottom));

            Container* LeftDownRight = new Container();
            LeftDownRight->setLayout(StackLayout::create().orientation(LayoutOrientation::LeftToRight));

            //Setting possitions of buttons
            ControlButtons->setVerticalAlignment(VerticalAlignment::Center);
            ControlButtons->setHorizontalAlignment(HorizontalAlignment::Center);
            buttonUp->setHorizontalAlignment(HorizontalAlignment::Center);
            LeftDownRight->setHorizontalAlignment(HorizontalAlignment::Center);
            buttonConnect->setVerticalAlignment(VerticalAlignment::Top);
            buttonConnect->setHorizontalAlignment(HorizontalAlignment::Center);
            slider->setHorizontalAlignment(HorizontalAlignment::Center);
            LabelSpeed->setHorizontalAlignment(HorizontalAlignment::Center);

            //Adding buttons to page
            ControlButtons->add(buttonUp);
            LeftDownRight->add(buttonLeft);
            LeftDownRight->add(buttonDown);
            LeftDownRight->add(buttonRight);
            ControlButtons->add(LeftDownRight);
            ControlButtons->add(LabelSpeed);
            ControlButtons->add(slider);
            contentContainer->add(ControlButtons);
            contentContainer->add(buttonConnect);

            //Keyboard control
            KeyListener* myKeyListener = KeyListener::create()
        //       .onKeyEvent(this, SLOT(onKeyEventHandler(bb::cascades::KeyEvent *)))
               .onKeyPressed(this, SLOT(onKeyPressedHandler(bb::cascades::KeyEvent *)))
               .onKeyReleased(this, SLOT(onKeyReleasedHandler(bb::cascades::KeyEvent *)));

            // Create a page with the main UI as the content
            page = new Page();
            page->addKeyListener(myKeyListener);
            page->setContent(contentContainer);

            // Create the application scene
            Application::instance()->setScene(page);

            setControlsEnabled(isConnected);
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("TankController_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void ApplicationUI::goStraight(bool go)
{
    if (go)
        slotWrite("L"+QString::number(currentSpeed)+":R"+QString::number(currentSpeed)+":");//MLgo:MRgo:");
    else
        slotWrite("L0:R0:");
    buttonDown->setEnabled(!go);
    buttonLeft->setEnabled(!go);
    buttonRight->setEnabled(!go);
}
void ApplicationUI::goBack(bool go)
{
    if (go)
        slotWrite("L-"+QString::number(currentSpeed)+":R-"+QString::number(currentSpeed)+":");
    else
        slotWrite("L0:R0:");
    buttonUp->setEnabled(!go);
    buttonLeft->setEnabled(!go);
    buttonRight->setEnabled(!go);
}
void ApplicationUI::goLeft(bool go)
{
    if (go)
        slotWrite("L-"+QString::number(currentSpeed)+":R"+QString::number(currentSpeed)+":");//"MLback:MRgo:");
    else
        slotWrite("L0:R0:");
    buttonDown->setEnabled(!go);
    buttonUp->setEnabled(!go);
    buttonRight->setEnabled(!go);
}
void ApplicationUI::goRight(bool go)
{
    if (go)
        slotWrite("L"+QString::number(currentSpeed)+":R-"+QString::number(currentSpeed)+":");//"MLgo:MRback:");
    else
        slotWrite("L0:R0:");
    buttonDown->setEnabled(!go);
    buttonUp->setEnabled(!go);
    buttonLeft->setEnabled(!go);
}

void ApplicationUI::onbuttonUpClicked(bb::cascades::TouchEvent *event)
{
    if (buttonUp->isEnabled())
    {
        if (event->touchType() == TouchType::Down)
            goStraight(true);
        else if (event->touchType() == TouchType::Up || event->touchType() == TouchType::Cancel)
            goStraight(false);
    }
}

void ApplicationUI::onbuttonDownClicked(bb::cascades::TouchEvent *event)
{
    if (buttonDown->isEnabled())
    {
        if (event->touchType() == TouchType::Down)
            goBack(true);
        else if (event->touchType() == TouchType::Up || event->touchType() == TouchType::Cancel)
            goBack(false);
    }
}

void ApplicationUI::onbuttonLeftClicked(bb::cascades::TouchEvent *event)
{
    if (buttonLeft->isEnabled())
    {
        if (event->touchType() == TouchType::Down)
            goLeft(true);
        else if (event->touchType() == TouchType::Up || event->touchType() == TouchType::Cancel)
            goLeft(false);
    }
}

void ApplicationUI::onbuttonRightClicked(bb::cascades::TouchEvent *event)
{
    if (buttonRight->isEnabled())
    {
        if (event->touchType() == TouchType::Down)
            goRight(true);
        else if (event->touchType() == TouchType::Up || event->touchType() == TouchType::Cancel)
            goRight(false);
    }
}

//Keyboard control
void ApplicationUI::onKeyPressedHandler(bb::cascades::KeyEvent *event)
{
    if (isConnected)
    {
        switch(event->keycap())
        {
            case 'w':
                if (buttonUp->isEnabled())
                    goStraight(true);
                break;
            case 'a':
                if (buttonLeft->isEnabled())
                    goLeft(true);
                break;
            case 's':
                if (buttonDown->isEnabled())
                    goBack(true);
                break;
            case 'd':
                if (buttonRight->isEnabled())
                    goRight(true);
                break;
        }
    }
}
void ApplicationUI::onKeyReleasedHandler(bb::cascades::KeyEvent *event)
{
    if (isConnected)
    {
        switch(event->keycap())
        {
            case 'w':
                if (buttonUp->isEnabled())
                    goStraight(false);
                break;
            case 'a':
                if (buttonLeft->isEnabled())
                    goLeft(false);
                break;
            case 's':
                if (buttonDown->isEnabled())
                    goBack(false);
                break;
            case 'd':
                if (buttonRight->isEnabled())
                    goRight(false);
                break;
        }
    }
}

void ApplicationUI::onSliderValueChanged(float value)
{
    currentSpeed=(int)value;
    QString speed=QString::number(currentSpeed);
    LabelSpeed->setText("Speed: "+speed);
    if (value<10)
    {
        speed.insert(0,"00");
    }
    else    if (value<100)
    {
        speed.insert(0,'0');
    }
    if (isConnected)
    {
        slotWrite("B"+speed+":");
    }
}

void ApplicationUI::onSettingsTriggered()
{
    TitleBar *settingsBar = TitleBar::create(TitleBarKind::Default)
        .title("Settings");
    settingsBar->setVisibility(bb::cascades::ChromeVisibility::Visible);
    ActionItem *closeItem=ActionItem::create()
        .title("Close");
    connect(closeItem,SIGNAL(triggered()),this,SLOT(loadInterface()));
    settingsBar->setDismissAction(closeItem);
    Page *settingPage=Page::create()
        .titleBar(settingsBar);

    Container *setContainer = Container::create();
    UIConfig *ui = setContainer->ui();
    setContainer->setTopPadding(ui->du(2.0));
    setContainer->setLeftPadding(ui->du(2.0));
    setContainer->setRightPadding(ui->du(2.0));

    setContainer->setLayout(StackLayout::create());
    settingPage->setContent(setContainer);

    Label *ipLabel = Label::create()
                .text("IP");
    TextField *ipTextField = TextField::create()
                .text(ip);
    connect(ipTextField,SIGNAL(textChanged (QString )),this,SLOT(onipTextFieldtextChanged (QString )));
    ipTextField->setInputMode(TextFieldInputMode::NumbersAndPunctuation);
    Label *portLabel = Label::create()
                .text("Port");
    TextField *portTextField = TextField::create().text(QString::number(port));
    connect(portTextField,SIGNAL(textChanged (QString )),this,SLOT(onportTextFieldtextChanged (QString )));
    portTextField->setInputMode(TextFieldInputMode::Pin);
    portTextField->setMaximumLength(10);


    setContainer->add(ipLabel);
    setContainer->add(ipTextField);
    setContainer->add(portLabel);
    setContainer->add(portTextField);

    Application::instance()->setScene(settingPage);

}

void ApplicationUI::onipTextFieldtextChanged (QString text)
{
    ip=text;
    QSettings settings;
    settings.setValue("ip", ip);
    settings.sync();
}

void ApplicationUI::onportTextFieldtextChanged (QString text)
{
    port=text.toInt();
    QSettings settings;
    settings.setValue("port", port);
    settings.sync();
}

void ApplicationUI::onbuttonConnectClicked()
{
    if (isConnected)
        m_pTcpSocket->close();
    m_pTcpSocket->connectToHost(ip,port);
    m_pTcpSocket->waitForConnected(5000);
}

void ApplicationUI::slotConnected()
{
    onSliderValueChanged(slider->value());
    setControlsEnabled(true);
    isConnected=true;

}

void ApplicationUI::slotReadyRead()
{
    if (m_pTcpSocket->bytesAvailable()>2)
    {
        QString str=QString(m_pTcpSocket->readAll());
        str=str.trimmed();
        if (str.length()>0)
        {
            str.insert(0,"Message from the tank:\n");
            //ShowMessage("Message from the tank:\n"+QString(m_pTcpSocket->readAll().data()));
            ShowMessage(str);
        }
    }
}

void ApplicationUI::slotError(QAbstractSocket::SocketError error)
{
    m_pTcpSocket->close();
    isTracking=false;
    isConnected=false;
    setControlsEnabled(false);

    ShowMessage("Connection error");
}

void ApplicationUI::setControlsEnabled(bool set)
{
    buttonDown->setEnabled(set && !isTracking);
    buttonUp->setEnabled(set && !isTracking);
    buttonLeft->setEnabled(set && !isTracking);
    buttonRight->setEnabled(set && !isTracking);
    slider->setEnabled(!isTracking);

    buttonConnect->setVisible(!set);
    //Menu bar
    if (!set)
    {
        page->removeAllActions();
    }
    else if (page->actionCount()!=3)
    {
        disconnectAction = ActionItem::create()
                .title("Disconnect");
        disconnectAction->setImageSource(QUrl("asset:///icons/disconnect.png"));
        connect(disconnectAction,SIGNAL(triggered()),this,SLOT(onDisconnectAction()));
        messageAction = ActionItem::create()
                .title("Send a message");
        connect(messageAction,SIGNAL(triggered()),this,SLOT(onMessageAction()));
        messageAction->setImageSource(QUrl("asset:///icons/send.png"));

        autoAction = ActionItem::create()
                .title("Automatic mode");
        autoAction->setImageSource(QUrl("asset:///icons/starred.png"));
        connect(autoAction,SIGNAL(triggered()),this,SLOT(onAutoAction()));

        shutdownAction = ActionItem::create()
                .title("Turn off the tank");
        shutdownAction->setImageSource(QUrl("asset:///icons/shutdown.png"));
        connect(shutdownAction,SIGNAL(triggered()),this,SLOT(onShutdownAction()));

        stopAction = ActionItem::create()
                        .title("STOP");
                stopAction->setImageSource(QUrl("asset:///icons/stop.png"));
                connect(stopAction,SIGNAL(triggered()),this,SLOT(onStopAction()));

        page->addAction(stopAction,ActionBarPlacement::OnBar);
        page->addAction(disconnectAction,ActionBarPlacement::InOverflow);
        page->addAction(messageAction,ActionBarPlacement::Signature);
        page->addAction(autoAction,ActionBarPlacement::OnBar);
        page->addAction(shutdownAction,ActionBarPlacement::InOverflow);
    }

}


void ApplicationUI::onDisconnectAction()
{
    try
    {
        m_pTcpSocket->disconnectFromHost();
        m_pTcpSocket->close();
        isTracking=false;
        setControlsEnabled(false);
    }
    catch(QtConcurrent::Exception *e)
    {
        ShowMessage(e->what());
    }
}

void ApplicationUI::onAutoAction()
{
    isTracking=!isTracking;
    if (isTracking)
        slotWrite("tracking_on");
    else
    {
        slotWrite("tracking_off");
        onSliderValueChanged(slider->value());
    }
    buttonDown->setEnabled(!isTracking);
    buttonUp->setEnabled(!isTracking);
    buttonLeft->setEnabled(!isTracking);
    buttonRight->setEnabled(!isTracking);
    slider->setEnabled(!isTracking);
}

void ApplicationUI::onMessageAction()
{
    m_prompt = new bb::system::SystemPrompt(this);

    m_prompt->setTitle("Enter a command");
    m_prompt->setDismissAutomatically(true);
    m_prompt->inputField()->setEmptyText("Please enter a command ...");
    m_prompt->inputField()->setInputMode(bb::system::SystemUiInputMode::Url);
    if (m_prompt->exec()==bb::system::SystemUiResult::ConfirmButtonSelection)
    {
        slotWrite(m_prompt->inputFieldTextEntry());
    }
    m_prompt->deleteLater();
}

void ApplicationUI::onShutdownAction()
{
    slotWrite("shutdown");
}

void ApplicationUI::onStopAction()
{
    if (isTracking)
        onAutoAction();
    slotWrite("tracking_off");
    slotWrite("L0:R0:SAUoff:");
}

void ApplicationUI::slotWrite(QString message)
{
    message+="\n";
    m_pTcpSocket->write(message.toAscii());
    m_pTcpSocket->flush();
}

void ApplicationUI::ShowMessage(QString message)
{
    if (m_toast==NULL)
    {
        m_toast = new bb::system::SystemToast();
        m_toast->setAutoUpdateEnabled(true);
        m_toast->setPosition(bb::system::SystemUiPosition::MiddleCenter);
        connect(m_toast,SIGNAL(finished(bb::system::SystemUiResult::Type)),this,SLOT(onMsgFinished(bb::system::SystemUiResult::Type)));
        m_toast->setBody(message);
        m_toast->show();
    }
    else
    {
        m_toast->setBody(message);
        m_toast->update();
    }
}

void ApplicationUI::onMsgFinished(bb::system::SystemUiResult::Type value)
{
        m_toast->deleteLater();
        m_toast=NULL;
}
