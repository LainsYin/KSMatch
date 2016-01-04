#include "selectwindialog.h"
#include "ui_selectwindialog.h"

SelectWinDialog::SelectWinDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectWinDialog)
{
    ui->setupUi(this);
}

SelectWinDialog::~SelectWinDialog()
{
    delete ui;
}
