#include "EmoticonFactory.h"
#include "WulforSettings.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QtDebug>

#ifndef CLIENT_ICONS_DIR
#define CLIENT_ICONS_DIR ""
#endif

static const QString EmoticonSectionName = "emoticons";
static const QString EmoticonSubsectionName = "emoticon";
static const QString EmoticonTextSectionName = "emotext";
static const QString EmotionPath = CLIENT_ICONS_DIR "/emot/";

EmoticonFactory::EmoticonFactory() :
    QObject(NULL), im(NULL)
{

}

EmoticonFactory::~EmoticonFactory(){
    clear();
}

void EmoticonFactory::load(){
    QString emoTheme = WSGET(WS_APP_EMOTICON_THEME);

    if (!QDir(EmotionPath+emoTheme).exists() || emoTheme.isEmpty())
        return;

    QString xmlFile = EmotionPath+emoTheme+QDir::separator()+emoTheme+".xml";
    QString pngFile = EmotionPath+emoTheme+QDir::separator()+emoTheme+".png";

    if (!(QFile::exists(xmlFile) && QFile::exists(pngFile)))
        return;

    QFile f(xmlFile);

    if (!f.open(QIODevice::ReadOnly))
        return;

    clear();

    QDomDocument dom;

    if (dom.setContent(&f))
        createEmoticonMap(dom);

    f.close();

    im = new QImage();
    im->load(pngFile, "PNG");
}

void EmoticonFactory::addEmoticons(QTextDocument *to){
    if (!im || list.isEmpty() || !to)
        return;

    QString emoTheme = WSGET(WS_APP_EMOTICON_THEME);

    foreach(EmoticonObject *i, list){
        to->addResource( QTextDocument::ImageResource,
                         QUrl(emoTheme + "/emoticon" + QString().setNum(i->id)),
                         im->copy(
                                  i->left,
                                  i->top,
                                  i->right - i->left,
                                  i->bottom - i->top
                                 )
                       );
    }
}

QString EmoticonFactory::convertEmoticons(const QString &html){
    if (html.isEmpty() || list.isEmpty() || map.isEmpty())
        return html;

    QString emoTheme = WSGET(WS_APP_EMOTICON_THEME);
    QString out = "";

    QStringList out_list = html.split(' ', QString::SkipEmptyParts);

    foreach (QString s, out_list){
        if (map.contains(s)){
            EmoticonObject *obj = map[s];

            QString img = QString("<img alt=\"%1\" title=\"%1\" align=\"center\" source=\"%2/emoticon%3\" />").arg(s)
                                                                                                              .arg(emoTheme)
                                                                                                              .arg(obj->id);

            out += img + " ";
        }
        else
            out += s + " ";
    }

    out_list = out.split("&nbsp;");

    out = "";

    foreach (QString s, out_list){
        if (map.contains(s)){
            EmoticonObject *obj = map[s];

            QString img = QString("<img alt=\"%1\" title=\"%1\" align=\"center\" source=\"%2/emoticon%3\" />").arg(s)
                                                                                                              .arg(emoTheme)
                                                                                                              .arg(obj->id);

            out += img + "&nbsp;";
        }
        else
            out += s + "&nbsp;";
    }

    return out;
}

QString EmoticonFactory::textForPos(unsigned x, unsigned y){
    QString emote = "";

    if (im) {
        EmoticonObject * e = NULL;
        for (EmoticonList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
            e = *it;
            if ((e->left < x) && (e->right > x) && (e->top < y) && (e->bottom > y)) {
                emote = map.keys(e).first();

                break;
            }
        }
    }

    return emote;
}

void EmoticonFactory::createEmoticonMap(const QDomNode &root){
    if (root.isNull())
        return;

    QDomNode r = findSectionByName(root, EmoticonSectionName);

    if (r.isNull())
        return;

    DomNodeList emoNodes;
    getSubSectionsByName(r, emoNodes, EmoticonSubsectionName);

    if (emoNodes.isEmpty())
        return;

    clear();

    foreach(QDomNode node, emoNodes){
        EmoticonObject *emot = new EmoticonObject();
        QDomElement el = node.toElement();

        emot->bottom    = el.attribute("bottom").toULong();
        emot->left      = el.attribute("left").toULong();
        emot->right     = el.attribute("right").toULong();
        emot->top       = el.attribute("top").toULong();
        emot->id        = list.size();

        DomNodeList emoTexts;
        getSubSectionsByName(node, emoTexts, EmoticonTextSectionName);

        if (emoTexts.isEmpty()){
            delete emot;

            continue;
        }

        int registered = 0;
        foreach (QDomNode node, emoTexts){
            QDomElement el = node.toElement();

            if (el.isNull())
                continue;

            QString text = el.text();

            if (text.isEmpty() || map.contains(text))
                continue;

            registered++;

            map.insert(text, emot);
        }

        if (registered > 0)
            list.push_back(emot);
        else
            delete emot;
    }
}

void EmoticonFactory::clear(){
    qDeleteAll(list);
    map.clear();

    delete im;
}

QDomNode EmoticonFactory::findSectionByName(const QDomNode &node, const QString &name){
    QDomNode domNode = node.firstChild();

    while (!domNode.isNull()){
        if (domNode.isElement()){
            QDomElement domElement = domNode.toElement();

            if (!domElement.isNull() && domElement.tagName().toLower() == name.toLower())
                return domNode;
        }

        domNode = domNode.nextSibling();
    }

    return QDomNode();
}
void EmoticonFactory::getSubSectionsByName(const QDomNode &node, EmoticonFactory::DomNodeList &list, const QString &name){
    QDomNode domNode = node.firstChild();

    while (!domNode.isNull()){
        list << domNode;

        domNode = domNode.nextSibling();
    }
}
