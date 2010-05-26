#ifndef MAINWINDOWSCRIPT_H
#define MAINWINDOWSCRIPT_H

#include <QObject>
#include <QAction>
#include <QIcon>
#include <QMap>
#include <QtScript/QScriptEngine>

class MainWindowScript : public QObject
{
Q_OBJECT
public:
    explicit MainWindowScript(QScriptEngine *engine, QObject *parent = 0);
    virtual ~MainWindowScript();

public slots:
    bool addToolButton(const QString &name, const QString &title, const QIcon &icon);
    bool remToolButton(const QString &name);

private:
    QScriptEngine *engine;
    QMap<QString, QAction*> actions;
};

#endif // MAINWINDOWSCRIPT_H
