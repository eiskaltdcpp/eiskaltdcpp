#ifndef CHATEDIT_H
#define CHATEDIT_H

#include <QPlainTextEdit>
#include <QStringListModel>

#include <QDebug>

#include "UserListModel.h"

class QCompleter;

class NickCompletionModel: public QStringListModel
{
	Q_OBJECT

public:
	NickCompletionModel(QObject *parent = 0) : QStringListModel(parent)
	{}
	NickCompletionModel(const QStringList &strings, QObject *parent = 0) : QStringListModel(strings, parent)
	{}
	virtual ~NickCompletionModel()
        { }

	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
	{
		QVariant result = QStringListModel::data(index, role);

		if (result.isValid() && role == Qt::EditRole) {
			int end = -1;
			QString nick = result.toString();
			if (nick.startsWith("[") && (end = nick.lastIndexOf("]")) > 0 && nick.length() > ++end) {
				nick.remove(0, end);
				result = nick;
			}

		}
		return result;
	}
};

class ChatEdit : public QPlainTextEdit
{
	Q_OBJECT

public:
	ChatEdit(QWidget *parent = 0);
	virtual ~ChatEdit();

	void setCompleter(QCompleter *, UserListModel *);

protected:
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent *);

private slots:
	void insertCompletion(const QModelIndex &);

private:
	QString textUnderCursor() const;
	void insertToPos(const QString &, int);
	void complete();

	UserListModel *cc_model;
    QCompleter *cc;
};

#endif // CHATEDIT_H
