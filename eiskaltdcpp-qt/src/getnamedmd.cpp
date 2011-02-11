#include "getnamedmd.h"
#include "ui_getnamedmd.h"

#include <QFile>
#include <QDir>

getNameDMD::getNameDMD(QString postfix, QStringList folders, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::getNameDMD),
    postfix(postfix)
{
    ui->setupUi(this);
    ui->comboBox->addItems(folders);
    ui->comboBox->setCurrentIndex(0);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeData()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeData()));
    changeData();
}

getNameDMD::~getNameDMD()
{
    delete ui;
}

void getNameDMD::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void getNameDMD::changeData()
{
    if (ui->lineEdit->text().length() == 0){
	ui->pushButton_OK->setEnabled(false);
	ui->label->setText(tr("Select file name"));
	return;
    }
    if (ui->lineEdit->text().indexOf(QDir::separator()) != -1){
	ui->pushButton_OK->setEnabled(false);
	ui->label->setText(tr("Error in file name"));
	return;
    }
    if (QFile::exists(ui->comboBox->currentText()+ui->lineEdit->text()+postfix)){
	ui->pushButton_OK->setEnabled(false);
	ui->label->setText(tr("This file exist"));
	return;
    }
    ui->pushButton_OK->setEnabled(true);
    ui->label->setText("");
}

void getNameDMD::on_lineEdit_returnPressed()
{
    if (!ui->pushButton_OK->isEnabled())
	return;
    this->on_pushButton_OK_clicked();
}

void getNameDMD::on_pushButton_OK_clicked()
{
    this->ok = true;
    this->close();
}

void getNameDMD::on_pushButton_CANSEL_clicked()
{
    this->ok = false;
    this->close();
}

bool getNameDMD::isOk()
{
    return this->ok;
}

QString getNameDMD::getFileName()
{
    return ui->comboBox->currentText()+ui->lineEdit->text()+postfix;
}
