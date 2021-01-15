/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextDocument>

#include "ArenaWidgetFactory.h"
#include "EmoticonFactory.h"
#include "HubFrame.h"
#include "SearchFrame.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "Secretary.h"

Secretary::Secretary(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SecretaryPrivate())
{
    setupUi(this);
    Q_D(Secretary);

    toolButton_HIDE->setIcon(WICON(WulforUtil::eiEDITDELETE));
    searchFrame->hide();

    installEventFilter(this);
    lineEdit_FIND->installEventFilter(this);

    d->maxLines = spinBoxLines->value();
    textEdit_MESSAGES->document()->setMaximumBlockCount(d->maxLines);
    textEdit_MESSAGES->setContextMenuPolicy(Qt::CustomContextMenu);
    textEdit_MESSAGES->setReadOnly(true);
    textEdit_MESSAGES->setAutoFormatting(QTextEdit::AutoNone);
    textEdit_MESSAGES->viewport()->installEventFilter(this); // QTextEdit don't receive all mouse events
    textEdit_MESSAGES->setMouseTracking(true);

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_MESSAGES->document());


    connect(this, SIGNAL(coreStatusMsg(const QString, const QString, const QString, const QString)),
            this, SLOT(addStatus(const QString, const QString, const QString, const QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreChatMessage(const QString, const QString, const QString, const QString)),
            this, SLOT(newChatMsg(const QString, const QString, const QString, const QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(corePrivateMsg(const QString, const QString, const QString, const QString)),
            this, SLOT(newPrivMsg(const QString, const QString, const QString, const QString)), Qt::QueuedConnection);

    connect(textEdit_MESSAGES, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));
    connect(checkBox_MAGNETS, SIGNAL(toggled(bool)), this, SLOT(searchMagnetLinks(bool)));
    connect(spinBoxLines, SIGNAL(valueChanged(int)), this, SLOT(maxLinesChanged(int)));
    connect(pushButton_ClearLog, SIGNAL(clicked(bool)), this, SLOT(clearNotes()));
    connect(toolButton_BACK, SIGNAL(clicked()), this, SLOT(slotFindBackward()));
    connect(toolButton_FORWARD, SIGNAL(clicked()), this, SLOT(slotFindForward()));
    connect(toolButton_HIDE, SIGNAL(clicked()), this, SLOT(slotHideSearchBar()));
    connect(lineEdit_FIND, SIGNAL(textEdited(QString)), this, SLOT(slotFindTextEdited(QString)));
    connect(toolButton_ALL, SIGNAL(clicked()), this, SLOT(slotFindAll()));

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
    updateStyles();
}

Secretary::~Secretary()
{
    Q_D(Secretary);
    delete d;
}

QWidget *Secretary::getWidget() {
    return this;
}

QString Secretary::getArenaTitle() {
    return tr("Secretary");
}

QString Secretary::getArenaShortTitle() {
    return getArenaTitle();
}

QMenu *Secretary::getMenu() {
    return nullptr;
}

bool Secretary::eventFilter(QObject *obj, QEvent *e){
    Q_D(Secretary);

    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        const bool keyEnter = (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return);
        const bool shiftModifier = (k_e->modifiers() == Qt::ShiftModifier);

        if (static_cast<QLineEdit*>(obj) == lineEdit_FIND) {
            const bool ret = QWidget::eventFilter(obj, e);

            if (keyEnter) {
                slotFindForward();
            }

            return ret;
        }

    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        const bool controlModifier = (k_e->modifiers() == Qt::ControlModifier);

        if (qobject_cast<LineEdit*>(obj) == lineEdit_FIND && k_e->key() == Qt::Key_Escape){
            lineEdit_FIND->clear();
            slotHideSearchBar();
            return true;
        }

#if QT_VERSION >= 0x050000
        if (controlModifier) {
            if (k_e->key() == Qt::Key_Equal || k_e->key() == Qt::Key_Plus){
                textEdit_MESSAGES->zoomIn();

                return true;
            }
            else if (k_e->key() == Qt::Key_Minus){
                textEdit_MESSAGES->zoomOut();

                return true;
            }
        }
#endif
    }
    else if (e->type() == QEvent::MouseButtonPress){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        const bool isChat = (static_cast<QWidget*>(obj) == textEdit_MESSAGES->viewport());

        if (isChat)
            textEdit_MESSAGES->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        if (isChat && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_MESSAGES->anchorAt(textEdit_MESSAGES->mapFromGlobal(QCursor::pos()));

            if (!WulforUtil::getInstance()->openUrl(pressedParagraph)){
                /**
                  Do nothing
                */
            }
        }
        else if (isChat && m_e->button() == Qt::MidButton)
        {
            QString nick;
            bool cursoratnick = false;

            if (isChat){
                QTextCursor cursor = textEdit_MESSAGES->textCursor();
                const QString pressedParagraph = cursor.block().text();
                const int positionCursor = cursor.columnNumber();

                int l = pressedParagraph.indexOf(" <");
                int r = pressedParagraph.indexOf("> ");

                if (l < r){
                    nick = pressedParagraph.mid(l+2, r-l-2);
                }
                if ((positionCursor < r) && (positionCursor > l))
                    cursoratnick = true;
            }
        }
    }
    else if (e->type() == QEvent::MouseButtonDblClick){
        const bool isChat = (static_cast<QWidget*>(obj) == textEdit_MESSAGES->viewport());

        if (isChat){
            QString nick;
            bool cursoratnick = false;
            QTextCursor cursor = textEdit_MESSAGES->textCursor();

            QString nickstatus="",nickmessage="";
            QString pressedParagraph = cursor.block().text();
            //qDebug() << pressedParagraph;
            int positionCursor = cursor.columnNumber();
            int l = pressedParagraph.indexOf(" <");
            int r = pressedParagraph.indexOf("> ");
            if (l < r)
                nickmessage = pressedParagraph.mid(l+2, r-l-2);
            else {
                int l1 = pressedParagraph.indexOf(" * ");
                //qDebug() << positionCursor << " " << l1 << " " << l << " " << r;
                if (l1 > -1 ) {
                    QString pressedParagraphstatus = pressedParagraph.remove(0,l1+3).simplified();
                    //qDebug() << pressedParagraphstatus;
                    int r1 = pressedParagraphstatus.indexOf(" ");
                    //qDebug() << r1;
                    nickstatus = pressedParagraphstatus.mid(0, r1);
                    //qDebug() << nickstatus;
                }
            }
            if (!nickmessage.isEmpty() || !nickstatus.isEmpty()) {
                //qDebug() << nickstatus;
                //qDebug() << nickmessage;
                nick = nickmessage + nickstatus;
                //qDebug() << nick;
            }
            if (((positionCursor < r) && (positionCursor > l))/* || positionCursor > l1*/)
                cursoratnick = true;
        }
    }
    else if (e->type() == QEvent::MouseMove && (static_cast<QWidget*>(obj) == textEdit_MESSAGES->viewport())){
        if (!textEdit_MESSAGES->anchorAt(textEdit_MESSAGES->mapFromGlobal(QCursor::pos())).isEmpty())
            textEdit_MESSAGES->viewport()->setCursor(Qt::PointingHandCursor);
        else
            textEdit_MESSAGES->viewport()->setCursor(Qt::IBeamCursor);
    }

    return QWidget::eventFilter(obj, e);
}

void Secretary::updateStyles() {
    QString custom_font_desc = WSGET(WS_CHAT_FONT);
    QFont custom_font;

    if (!custom_font_desc.isEmpty() && custom_font.fromString(custom_font_desc)){
        textEdit_MESSAGES->document()->setDefaultStyleSheet(
                    QString("pre { margin:0px; white-space:pre-wrap; font-family:'%1'; font-size: %2pt; }")
                    .arg(custom_font.family()).arg(custom_font.pointSize())
                    );
    }
    else {
        textEdit_MESSAGES->document()->setDefaultStyleSheet(
                    QString("pre { margin:0px; white-space:pre-wrap; font-family:'%1' }")
                    .arg(QApplication::font().family())
                    );
    }
}

void Secretary::clearNotes(){
    Q_D(Secretary);
    d->origMessages.clear();
    textEdit_MESSAGES->setHtml("");

    updateStyles();

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_MESSAGES->document());
}

void Secretary::searchMagnetLinks(bool value){
    checkBox_BT_LINKS->setEnabled(value);
    checkBox_SEARCH_LINKS->setEnabled(value);
}

void Secretary::maxLinesChanged(int value){
    Q_D(Secretary);
    d->maxLines = value;
    textEdit_MESSAGES->document()->setMaximumBlockCount(value);
}

void Secretary::slotChatMenu(const QPoint &){
    QTextEdit *editor = qobject_cast<QTextEdit*>(sender());
    if (!editor)
        return;

    QTextCursor cursor = editor->cursorForPosition(editor->mapFromGlobal(QCursor::pos()));
    QString nick;
    if(cursor.block().userData())
        nick = static_cast<UserListUserData*>(cursor.block().userData())->data;

    Q_D(Secretary);

    if (nick.isEmpty()){
        QMenu *m = editor->createStandardContextMenu(QCursor::pos());
        m->exec(QCursor::pos());
        delete m;
        return;
    }

    QPoint p = QCursor::pos();

    enum Action {
        CopyText = 0,
        SearchText,
        CopyNick,
        ClearNotes,
        FindInNotes,
        SelectAllNotes,
        ZoomInNotes,
        ZoomOutNotes,
        None
    };

    QMenu *menu = new QMenu(nullptr);

    QAction *title = new QAction(nick, menu);
    QFont f;
    f.setBold(true);
    title->setFont(f);
    title->setEnabled(false);

    WulforUtil *WU = WulforUtil::getInstance();
    QAction *copy_text    = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy"), menu);
    QAction *search_text  = new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Search text"), menu);
    QAction *copy_nick    = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy nick"), menu);
    QAction *sep1         = new QAction(menu);
    QAction *clear_notes  = new QAction(WU->getPixmap(WulforUtil::eiCLEAR), tr("Clear notes"), menu);
    QAction *find_in_notes= new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Find in notes"), menu);
    QAction *sep2         = new QAction(menu);
    QAction *select_all   = new QAction(tr("Select all"), menu);
    QAction *sep3         = new QAction(menu);
    QAction *zoom_in      = new QAction(WU->getPixmap(WulforUtil::eiZOOM_IN), tr("Zoom In"), menu);
    QAction *zoom_out     = new QAction(WU->getPixmap(WulforUtil::eiZOOM_OUT), tr("Zoom Out"), menu);

    copy_text->setData(static_cast<int>(CopyText));
    search_text->setData(static_cast<int>(SearchText));
    copy_nick->setData(static_cast<int>(CopyNick));
    clear_notes->setData(static_cast<int>(ClearNotes));
    find_in_notes->setData(static_cast<int>(FindInNotes));
    select_all->setData(static_cast<int>(SelectAllNotes));
    zoom_in->setData(static_cast<int>(ZoomInNotes));
    zoom_out->setData(static_cast<int>(ZoomOutNotes));

    sep1->setSeparator(true);
    sep2->setSeparator(true);
    sep3->setSeparator(true);

    menu->addAction(title);
    menu->addAction(copy_text);
    menu->addAction(search_text);
    menu->addAction(copy_nick);
    menu->addAction(sep1);
    menu->addAction(clear_notes);
    menu->addAction(find_in_notes);
    menu->addAction(sep2);
    menu->addAction(select_all);
    menu->addAction(sep3);
    menu->addAction(zoom_in);
    menu->addAction(zoom_out);

    QAction *result = menu->exec(QCursor::pos());
    if (!result || !result->data().isValid()){
        menu->deleteLater();
        return;
    }

    switch (result->data().toInt()){
        case CopyText:
        {
            QString ret = editor->textCursor().selectedText();

            if (ret.isEmpty())
                ret = editor->anchorAt(textEdit_MESSAGES->mapFromGlobal(p));

            if (ret.startsWith("user://")){
                ret.remove(0, 7);

                ret = ret.trimmed();

                if (ret.startsWith("<") && ret.endsWith(">")){
                    ret.remove(0, 1); //remove <
                    ret = ret.left(ret.lastIndexOf(">")); //remove >
                }
            }

            if (ret.isEmpty())
                ret = editor->textCursor().block().text();

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case SearchText:
        {
            QString ret = editor->textCursor().selectedText();

            if (ret.isEmpty())
                ret = editor->anchorAt(textEdit_MESSAGES->mapFromGlobal(p));

            if (ret.startsWith("user://")){
                ret.remove(0, 7);

                ret = ret.trimmed();

                if (ret.startsWith("<") && ret.endsWith(">")){
                    ret.remove(0, 1);//remove <
                    ret = ret.left(ret.lastIndexOf(">"));//remove >
                }
            } else if (ret.startsWith("magnet:?")) {
                QString name, tth;
                int64_t size;

                WulforUtil::splitMagnet(ret, size, tth, name);
                ret = name;
            }

            if (ret.isEmpty())
                break;

            SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame, QWidget*>(this);
            sf->fastSearch(ret, false);

            break;
        }
        case CopyNick:
        {
            qApp->clipboard()->setText(nick, QClipboard::Clipboard);

            break;
        }
        case ClearNotes:
        {
            clearNotes();

            break;
        }
        case FindInNotes:
        {
            slotShowSearchBar();

            break;
        }
        case SelectAllNotes:
        {
            editor->selectAll();

            break;
        }
        case ZoomInNotes:
        {
            editor->zoomIn();

            break;
        }
        case ZoomOutNotes:
        {
            editor->zoomOut();

            break;
        }
        default:
        {
            break;
        }
    }

    menu->deleteLater();
}

void Secretary::slotFindTextEdited(const QString &text){
    if (text.isEmpty()){
        textEdit_MESSAGES->verticalScrollBar()->setValue(textEdit_MESSAGES->verticalScrollBar()->maximum());
        textEdit_MESSAGES->textCursor().movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);

        return;
    }

    QTextCursor c = textEdit_MESSAGES->textCursor();

    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    c = textEdit_MESSAGES->document()->find(lineEdit_FIND->text(), c, nullptr);
    if (!c.isNull()) {
        textEdit_MESSAGES->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        textEdit_MESSAGES->setTextCursor(c);
        slotFindAll();
    }
}

void Secretary::slotFindAll(){
    if (!toolButton_ALL->isChecked()){
        textEdit_MESSAGES->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!lineEdit_FIND->text().isEmpty()) {
        QTextEdit::ExtraSelection selection;

        QColor color;
        color.setNamedColor(WSGET(WS_CHAT_FIND_COLOR));
        color.setAlpha(WIGET(WI_CHAT_FIND_COLOR_ALPHA));

        selection.format.setBackground(color);

        QTextCursor c = textEdit_MESSAGES->document()->find(lineEdit_FIND->text(), 0, nullptr);

        while (!c.isNull()) {
            selection.cursor = c;
            extraSelections.append(selection);

            c = textEdit_MESSAGES->document()->find(lineEdit_FIND->text(), c, nullptr);
        }
    }
    textEdit_MESSAGES->setExtraSelections(extraSelections);
}

void Secretary::slotShowSearchBar(){
    searchFrame->setVisible(true);
    QString stext = textEdit_MESSAGES->textCursor().selectedText();
    if (!stext.isEmpty())
        lineEdit_FIND->setText(stext);
    lineEdit_FIND->selectAll();
    lineEdit_FIND->setFocus();
}

void Secretary::slotHideSearchBar(){
    searchFrame->setVisible(false);
    QTextCursor c = textEdit_MESSAGES->textCursor();
    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    textEdit_MESSAGES->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    textEdit_MESSAGES->setTextCursor(c);
}

void Secretary::findText(QTextDocument::FindFlags flag){
    textEdit_MESSAGES->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    if (lineEdit_FIND->text().isEmpty())
        return;

    QTextCursor c = textEdit_MESSAGES->textCursor();

    const bool ok = textEdit_MESSAGES->find(lineEdit_FIND->text(), flag);

    if (flag == QTextDocument::FindBackward && !ok)
        c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor,1);
    else if (!flag && !ok)
        c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor,1);

    c = textEdit_MESSAGES->document()->find(lineEdit_FIND->text(), c, flag);
    if (!c.isNull()) {
        textEdit_MESSAGES->setTextCursor(c);
        slotFindAll();
    }
}

void Secretary::slotSettingsChanged(const QString &key, const QString&){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void Secretary::addStatus(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url){
    if (htmlMsg.isEmpty() || origMsg.isEmpty() || url.isEmpty())
        return;

    if (!checkBox_STATUS_MSGS->isChecked())
        return;

    addOutput(htmlMsg, origMsg, url);
}

void Secretary::newChatMsg(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url){
    if (htmlMsg.isEmpty() || origMsg.isEmpty() || url.isEmpty())
        return;

    if (!checkBox_CHAT_MSGS->isChecked())
        return;

    addOutput(htmlMsg, origMsg, url);

    Q_D(Secretary);
    if (!d->origMessages.isEmpty())
        addUserData(nick);
}

void Secretary::newPrivMsg(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url){
    if (htmlMsg.isEmpty() || origMsg.isEmpty() || url.isEmpty())
        return;

    if (!checkBox_PRIV_MSGS->isChecked())
        return;

    addOutput("<b>PM: </b>" + htmlMsg, origMsg, url);

    Q_D(Secretary);
    if (!d->origMessages.isEmpty())
        addUserData(nick);
}

void Secretary::addOutput(const QString& htmlMsg, const QString& origMsg, const QString &url) {
    Q_D(Secretary);

    if (checkBox_HUBS_FILTER->isChecked()) {
        const QStringList &&urlParts = url.split(":");
        const QStringList &&addresses = lineEdit_HUBS_FILTER->text().split(",", QString::SkipEmptyParts);
        if (urlParts.isEmpty() || addresses.isEmpty())
            return;

        const QString &ipAddress = urlParts.at(0);
        bool foundAddress = false;
        for (const auto &a : addresses) {
            if (a.trimmed() == ipAddress) {
                foundAddress = true;
                break;
            }
        }

        if (!foundAddress)
            return;
    }

    if (checkBox_FILTER_DUPS->isChecked()) {
        if (d->origMessages.contains(origMsg))
            return;
    }

    bool storeMessage = false;

    const bool foundMagnetLink = origMsg.contains("magnet:?");
    const bool foundBitTorrentLink = foundMagnetLink && (origMsg.contains("urn:btih:") || origMsg.contains("urn:btmh:"));
    const bool foundSearchLink = foundMagnetLink && (origMsg.contains("?kt=") || origMsg.contains("&kt="));

    if (checkBox_MAGNETS->isChecked() && foundMagnetLink) {
        storeMessage = true;

        if (!checkBox_BT_LINKS->isChecked() && foundBitTorrentLink)
            storeMessage = false;
        else if (!checkBox_SEARCH_LINKS->isChecked() && foundSearchLink)
            storeMessage = false;
    }

    if (checkBox_KEYWORDS->isChecked()) {
        const QStringList &&keywords = lineEdit_KEYWORDS->text().split(",", QString::SkipEmptyParts);
        for (const auto &k : keywords) {
            if (origMsg.contains(k, Qt::CaseInsensitive)) {
                storeMessage = true;
                break;
            }
        }
    }

    if (!storeMessage)
        return;

    d->origMessages.append(origMsg);
    while (d->origMessages.size() > spinBoxLines->value()) {
        d->origMessages.removeFirst();
    }

    QString tmp = "[" + url + "] "+ htmlMsg;
    tmp.replace("\r", "");
    tmp = "<pre>" + tmp + "</pre>";
    textEdit_MESSAGES->append(tmp);
}

void Secretary::addUserData(const QString &nick){
    QTextDocument *chatDoc = textEdit_MESSAGES->document();
    for (QTextBlock itu = chatDoc->lastBlock(); itu.isValid(); itu = itu.previous()){
        if (!itu.userData())
            itu.setUserData(new UserListUserData(nick));
        else
            break;
    }
}

