#ifndef EMOTICONFACTORY_H
#define EMOTICONFACTORY_H

#include <QObject>
#include <QList>
#include <QtXml>
#include <QImage>
#include <QTextDocument>

#include "EmoticonObject.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class EmoticonFactory :
        public QObject,
        public dcpp::Singleton<EmoticonFactory>
{
Q_OBJECT
typedef QList<QDomNode> DomNodeList;
typedef QList<QTextDocument*> TextDocumentList;

friend class dcpp::Singleton<EmoticonFactory>;

public:
    void load();
    void unload(){ this->clear(); }

    void addEmoticons(QTextDocument *to);
    QString convertEmoticons(const QString &html);
    QString textForPos(unsigned x, unsigned y);
    QImage getImage() {
        if (im)
            return *im;
        else
            return QImage();
    }

private slots:
    void slotDocDeleted();

private:
    EmoticonFactory();
    ~EmoticonFactory();

    void clear();
    void createEmoticonMap(const QDomNode &root);

    QDomNode findSectionByName(const QDomNode &node, const QString &name);
    void getSubSectionsByName(const QDomNode &node, DomNodeList &list, const QString &name);

    EmoticonMap map;
    EmoticonList list;
    TextDocumentList docs;

    QImage *im;
};

#endif // EMOTICONFACTORY_H
