#ifndef QDMD_H
#define QDMD_H

#include <QString>
#include <QObject>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include <QDomDocument>
#include <QDomNode>
#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QFileIconProvider>
#include <QVariant>
#include <QMap>

#include "lib7z.h"

class Node{
public:
    Node(const QString s1, const QString s2, const int size, Node *parentNode = 0):str1(s1),str2(s2),size(size),parent(parentNode){};
    ~Node();

    QString str1, str2;
    int size;
    Node *parent;
    QList<Node*> children;
};

struct file{
    QString name;
    QString tth;
    int size;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    TreeModel(QObject *parent=0);
    ~TreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    bool hasChildren(const QModelIndex& parent=QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool fromXml(QString xml);

    int rowCount(const QModelIndex& index) const;
    int columnCount(const QModelIndex&) const {return 3;};

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void setRootNode(Node *node);
    void setEditable(bool editable){this->editable = editable; return;};
    void addElement(QModelIndex index, QString name, QString href, int size);
    bool delElement(QModelIndex index);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QString toXml();

    QList<file> getList(){return files;};
    static QList<file> getList(QString fileName);

    Node* getRoot(){return rootNode;};

    void delEmpty();

private:

    void addNode(QDomDocument *document, QDomElement *element, Node *node);
    void addtree(Node *rootNode, QDomNode node);

    Node* nodeFromIndex(const QModelIndex &index) const;
    Node* rootNode;
    bool editable;
    QList<file> files;
};

class QDmd : public QObject
{
    Q_OBJECT
public:
    
    QDmd();

    bool load(QString FileName);
    bool create(QString aHtml, TreeModel *aModel, QImage aCover, QList<QImage> aImages);
    static TreeModel *getXmlModel(QString FileName);

    static bool isValid(QString FileName);

    bool saveFile(QString FileName);

    QString getError(){return error;};

    TreeModel *getModel(){return model;};
    void setModel(TreeModel *model){this->model = model;};

    QString getHTML(){return html;};
    void setHTML(QString html){this->html = html;};

    QImage getImage(int i){return images[i];};
    void addImage(QImage img){images.append(img);};
    int getImageCount(){return images.count();};

    QImage getCover(){return cover;};
    void setCover(QImage aCover){cover = aCover;};

protected:
    QString html;
    TreeModel *model;
    QImage cover;
    QList<QImage> images;
    
    QString error;
};

#endif // QDMD_H
