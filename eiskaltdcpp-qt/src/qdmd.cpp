/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDir>
#include <QPixmap>
#include <QTextCodec>
#include <QBuffer>
#include <QDebug>

#include "WulforUtil.h"

#include "qdmd.h"

QDmd::QDmd()
{
    error = "";
    model = new TreeModel();
}

bool QDmd::saveFile(QString FileName)
{

    if (cover.width() > 256){
        cover = cover.scaledToWidth(256);
    }
    if (cover.height() > 256){
        cover = cover.scaledToHeight(256);
    }

    for (int i = 0; i < images.count(); ++i){
        if (images[i].width() > 1024){
            images[i] = images[i].scaledToWidth(1024);
        }
        if (images[i].height() > 768){
            images[i] = images[i].scaledToHeight(768);
        }
    }

    QFile f1(QDir::tempPath()+QDir::separator()+tr("xml"));
    QFile f2(QDir::tempPath()+QDir::separator()+tr("html"));

    if (!f1.open(QIODevice::WriteOnly)){
        error = tr("Error write temporaly file");
        return false;
    }
    if (!f2.open(QIODevice::WriteOnly)){
        error = tr("Error write temporaly file");
        return false;
    }

    f1.write(this->model->toXml().toAscii()); f1.close();
    f2.write(this->html.toAscii()); f2.close();

    QPixmap pix;
    QString fName;

    fName = QDir::tempPath()+QDir::separator()+tr("cover");
    if (QFile::exists(fName)){
	QFile::remove(fName);
    }
    pix = QPixmap::fromImage(cover);
    if (!pix.save(fName, "JPG", 80)){
        error = tr("Error write temporaly file");
        return false;
    }

    QStringList files;
    files << QDir::tempPath()+QDir::separator()+tr("xml")
          << QDir::tempPath()+QDir::separator()+tr("html")
          << QDir::tempPath()+QDir::separator()+tr("cover");

    for (int i = 0; i < images.count(); ++i){
        fName = QDir::tempPath()+QDir::separator()+tr("img%1").arg(i+1);
        if (QFile::exists(fName)){
            QFile::remove(fName);
        }
        pix = QPixmap::fromImage(images[i]);
        if (!pix.save(fName, "JPG", 80)){
            error = tr("Error write temporaly file");
            return false;
        }
        files << fName;
    }

    if (!lib7z::create7z(lib7z::standart, FileName, files, &error)){
        return false;
    }

    return true;
}

bool QDmd::load(QString FileName)
{
    QStringList list = lib7z::getIncFiles(FileName);

    if (list.count() > 6){
	error = tr("The file does not corresponds format");
	return false;
    }
    if (list.count() < 3){
	error = tr("The file does not corresponds format");
	return false;
    }

    bool right = true;
    if (list.indexOf(tr("xml")) == -1) right = false;
    if (list.indexOf(tr("html")) == -1) right = false;
    if (list.indexOf(tr("cover")) == -1) right = false;

    for (int i = 0; i < list.count()-3; ++i){
        if (list.indexOf(tr("img%1").arg(i+1)) == -1) right = false;
    }

    if (!right){
	error = tr("The file does not corresponds format");
	return false;
    }

    QString err;

    QByteArray xml;
    if (!lib7z::extractFile(FileName, tr("xml"), &xml, &err)){
        error = tr("Error of load list of files");
	return false;
    }

    QByteArray html;
    if (!lib7z::extractFile(FileName, tr("html"), &html, &err)){
        error = tr("Error of load");
	return false;
    }

    this->html = QTextCodec::codecForName("UTF-8")->toUnicode(html);

    QString xmlStr = QTextCodec::codecForName("UTF-8")->toUnicode(xml);
    if (!this->model->fromXml(xmlStr)){
	error = tr("Logic error of format");
	return false;
    }

    QByteArray img;
    QPixmap tmp;

    img.clear();
    if (!lib7z::extractFile(FileName, tr("cover"), &img, &err)){
        error = tr("Error of load image");
        return false;
    }
    try{
        tmp.loadFromData(img);
        cover = tmp.toImage();
    }catch (const std::exception &e){
        error = tr("Error of load image");
        return false;
    }

    for (int i = 0; i < list.count()-3; ++i){
        img.clear();
        if (!lib7z::extractFile(FileName, tr("img%1").arg(i+1), &img, &err)){
            error = tr("Error of load image");
            return false;
        }
        try{
            tmp.loadFromData(img);
            images << tmp.toImage();
        }catch (const std::exception &e){
            error = tr("Error of load image");
            return false;
        }
    }

    return true;
}

bool QDmd::create(QString aHtml, TreeModel *aModel, QImage aCover, QList<QImage> aImages)
{
    html = aHtml;

    delete model;
    model = aModel;

    cover = aCover;

    images.clear();
    images << aImages;

    error = "";
    return true;
}

TreeModel *QDmd::getXmlModel(QString FileName){
    TreeModel *rez = new TreeModel();

    QStringList list = lib7z::getIncFiles(FileName);

    bool right = true;
    if (list.indexOf(tr("xml")) == -1) right = false;
    if (list.indexOf(tr("html")) == -1) right = false;
    if (list.indexOf(tr("cover")) == -1) right = false;

    for (int i = 0; i < list.count()-3; ++i){
        if (list.indexOf(tr("img%1").arg(i+1)) == -1) right = false;
    }

    if (!right){
	return rez;
    }

    QString err;

    QByteArray xml;
    if (!lib7z::extractFile(FileName, tr("xml"), &xml, &err)){
	return rez;
    }

    if (!rez->fromXml(QTextCodec::codecForName("UTF-8")->toUnicode(xml))){
	return rez;
    }
    return rez;
}

Node::~Node()
{
    qDeleteAll(this->children);
}

TreeModel::TreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    rootNode = 0;
    this->editable = false;
    //this->setRootNode(new Node("", ""));
}

TreeModel::~TreeModel()
{
    delete this->rootNode;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (rootNode){
	Node *paretnNode = nodeFromIndex(parent);
	if (paretnNode){
	    return createIndex(row, column, paretnNode->children[row]);
	}
    }
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &child) const
{
    Node* node = nodeFromIndex(child);
    if (!node){
	return QModelIndex();
    };
    Node *parentNode = node->parent;
    if (!parentNode){
	return QModelIndex();
    }

    Node *grandParentNode = parentNode->parent;
    if (!grandParentNode){
	return QModelIndex();
    }
    int row = grandParentNode->children.indexOf(parentNode);
    return createIndex(row, child.column(), parentNode);
}

bool TreeModel::hasChildren(const QModelIndex &parent) const
{
    Node *parentNode = nodeFromIndex(parent);
    if (!parentNode) return false;
    return (parentNode->children.count() > 0);
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole){
	return false;
    }
    int col = index.column();
    if (col != 0){
	return false;
    }
    Node* node = nodeFromIndex(index);
    if (col == 0){
	node->str1 = value.toString();
    }
    return true;
}

bool TreeModel::fromXml(QString xml)
{
    delete rootNode;
    rootNode = new Node("", "", 0);
    QDomDocument dom;
    if (!dom.setContent(xml)){
	return false;
    }

    QDomElement root = dom.documentElement();

    if (root.attribute("version", "error") != "1.0"){
	return false;
    }
    addtree(rootNode, root.firstChild());
    return true;
}

int TreeModel::rowCount(const QModelIndex &index) const
{
    Node *node = nodeFromIndex(index);
    if (node){
	return node->children.count();
    } else {
	return 0;
    }
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
	if (section == 0)
	    return QObject::tr("Name");
	if (section == 1)
	    return QObject::tr("Href");
	if (section == 2)
	    return QObject::tr("Size");
    }
    return QVariant();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    Node* node = nodeFromIndex(index);
    if ((role == Qt::DisplayRole) || role == Qt::EditRole){
	if (!node)
	    return QVariant();
	else
	    if (col == 0)
		return node->str1;
	    else
		if (col == 1)
		    return node->str2;
		else
		    if (col == 2){
			if ((node->children.count() != 0) || (node->size == 0)){
			    return "";
			} else {
			    return QVariant(WulforUtil::formatBytes(node->size));
			}
		    }else
			return QVariant();
    }
    return QVariant();
}

void TreeModel::setRootNode(Node *node)
{
    delete rootNode;
    rootNode = node;
    return;
}

void TreeModel::addElement(QModelIndex index, QString name, QString href, int size)
{
    Node *node = nodeFromIndex(index);
    Node *n = new Node(name, href, size, node);
    node->children.append(n);
}

bool TreeModel::delElement(QModelIndex index)
{
    Node *node = nodeFromIndex(index);
    if (node == this->rootNode){
	return false;
    }
    if (node == this->rootNode->children[0]){
	return false;
    }
    node->parent->children.removeOne(node);
    node->~Node();
    return true;
}

void TreeModel::addNode(QDomDocument *document, QDomElement *element, Node *node)
{
    QDomElement *x = new QDomElement();
    for (int i = 0; i < node->children.count(); ++i){
	if (node->children.at(i)->children.count()>0){
	    *x = document->createElement("folder");
	    x->setAttribute("name", node->children.at(i)->str1);
	    addNode(document, x, node->children.at(i));
	    element->appendChild(*x);
	} else {
	    *x = document->createElement("file");
	    x->setAttribute("name", node->children.at(i)->str1);
	    x->setAttribute("href", node->children.at(i)->str2);
	    x->setAttribute("size", node->children.at(i)->size);
	    element->appendChild(*x);
	}
    }
}

void TreeModel::addtree(Node *rootNode, QDomNode node)
{
    while (!node.isNull()){
	if (!((node.toElement().tagName() == "folder")
	       || (node.toElement().tagName() == "file"))){
	    node = node.nextSibling();
	    continue;
	}
	QString name, href;
	int size;
	static int foldernumber;
	static int filenumber;

	if (node.toElement().tagName() == "folder"){
	    name = node.toElement().attribute("name", QObject::tr("folder %1").arg(foldernumber++));
	} else {
	    name = node.toElement().attribute("name", QObject::tr("file %1").arg(filenumber++));
	}
	href = node.toElement().attribute("href", QObject::tr(""));
	bool ok;
	size = node.toElement().attribute("size", QObject::tr("")).toInt(&ok);
	Node *newNode = new Node(name, href, size, rootNode);
	rootNode->children.append(newNode);
	if (node.toElement().tagName() != "folder"){
	    if ((href == "") || (!ok)){
		node = node.nextSibling();
		continue;
	    }
	    file f;
	    f.name = name;
	    f.tth = href;
	    f.size = size;
	    files.append(f);
	}

	if (node.toElement().tagName() == "folder"){
	    addtree(newNode, node.firstChild());
	}
	node = node.nextSibling();
    }

    return;
}

Node* TreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()){
	return static_cast<Node *>(index.internalPointer());
    } else {
	return this->rootNode;
    }
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (index.isValid()){
	if ((this->editable) && (index.column() == 0))
	    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
	else
	    return Qt::ItemIsEnabled;
    } else {
	return 0;
    }
}

QString TreeModel::toXml()
{
    QDomDocument *document = new QDomDocument();
    QDomElement *d = new QDomElement();
    *d = document->createElement(QObject::tr("tree"));
    document->appendChild(*d);
    d->setAttribute(QObject::tr("version"), QObject::tr("1.0"));
    QDomElement *x = new QDomElement();
    for (int i = 0; i < this->rootNode->children.count(); ++i){
	if (rootNode->children.at(i)->children.count()>0){
	    //delete x;
	    *x = document->createElement(QObject::tr("folder"));
	    x->setAttribute(QObject::tr("name"), rootNode->children.at(i)->str1);
	    d->appendChild(*x);
	    addNode(document, x, rootNode->children.at(i));
	} else {
	    //delete x;
	    *x = document->createElement(QObject::tr("file"));
	    x->setAttribute(QObject::tr("name"), rootNode->children.at(i)->str1);
	    x->setAttribute(QObject::tr("href"), rootNode->children.at(i)->str2);
	    x->setAttribute(QObject::tr("size"), rootNode->children.at(i)->size);
	    d->appendChild(*x);
	}
    }
    QDomNode n = document->createProcessingInstruction(QObject::tr("xml"),
						      QObject::tr("version=\"1.0\" encoding=\"UTF-8\""));
    document->insertBefore(n, document->firstChild());
    QString str = document->toString(1);
    return str;
}

QList<file> TreeModel::getList(QString fileName)
{
    TreeModel *model = QDmd::getXmlModel(fileName);
    QList<file> ret = model->getList();
    return ret;
}

bool QDmd::isValid(QString FileName)
{
    QStringList list = lib7z::getIncFiles(FileName);

    bool right = true;
    if (list.indexOf(tr("xml")) == -1) right = false;
    if (list.indexOf(tr("html")) == -1) right = false;
    if (list.indexOf(tr("cover")) == -1) right = false;

    for (int i = 0; i < list.count()-3; ++i){
        if (list.indexOf(tr("img%1").arg(i+1)) == -1) right = false;
    }

    if (!right){
	return false;
    }
    return true;
}

bool delEmp(Node *n)
{
    for (int i = 0; i < n->children.count(); ++i){
	if (n->children[i]->str2.length() == 0){
	    if (n->children[i]->children.count() == 0){
		delete n->children[i];
		n->children.removeAt(i);
		i--;
	    } else {
		if (delEmp(n->children[i])){
		    delete n->children[i];
		    n->children.removeAt(i);
		    i--;
		}
	    }
	}
    }
    return (n->children.count() == 0);
}

void TreeModel::delEmpty()
{
    for (int i = 0; i < rootNode->children.count(); ++i){
	if (rootNode->children[i]->str2.length() == 0){
	    if (delEmp(rootNode->children[i])){
		delete rootNode->children[i];
		rootNode->children.removeAt(i);
		i--;
	    }
	}
    }
}
