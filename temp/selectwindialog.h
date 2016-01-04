#ifndef SELECTWINDIALOG_H
#define SELECTWINDIALOG_H

#include <QDialog>

namespace Ui {
class SelectWinDialog;
}

class SelectWinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectWinDialog(QWidget *parent = 0);
    ~SelectWinDialog();

private:
    Ui::SelectWinDialog *ui;
};

#endif // SELECTWINDIALOG_H
