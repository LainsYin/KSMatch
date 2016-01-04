#ifndef SELECTWINNERDIALOG_H
#define SELECTWINNERDIALOG_H

#include <QDialog>

namespace Ui {
class SelectWinnerDialog;
}

class Media;
class Record;
class TableModel;
class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class SelectWinnerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectWinnerDialog(QWidget *parent = 0, int _selectId = 0);
    ~SelectWinnerDialog();
    void closeEvent(QCloseEvent *);
    void setTableValue(QList<Record> _records);
    void hiddenOperate(bool hidden = true);

public slots:
    void sendSuccess();
private:
    void initTable();

private slots:
    void leftListen(const int &row);
    void rightWinner(const int &row);

    void playerStop();

signals:
    void winner(const int &row);
    void listen(const int &row);

private:
    Ui::SelectWinnerDialog *ui;
    TableModel *model;
    QList< QStringList > rowList;
    QList<Record> records;
    int selectBoxId;

    ///
    ////// \brief _instance
    ///
    VlcInstance *_instance;
    VlcMedia *_media;
    VlcMediaPlayer *_player;
};

#endif // SELECTWINNERDIALOG_H
