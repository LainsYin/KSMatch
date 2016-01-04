#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QDialog>

namespace Ui {
class SendDialog;
}

struct Media{
    QString name;
    QString singer;
    QString serial_id;
    QString mid;
};

struct RewardInfo{
    QString ids;
    QString ktv_name;
    QString name;
    QString singer;
    QString mid;
    QString serial_id;
    QString startTime;
    QString endTime;
    QString efectTime;
    QString reward;
    QString status;
    QString kboxids;
    QString win_kboxid;
};

class QTimer;
class MysqlQuery;
class SendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendDialog(QWidget *parent = 0);
    ~SendDialog();
    void paintEvent(QPaintEvent *);
private:
    void initWidget();

private slots:
    void on_pushButton_clicked();
    void setSongName(const Media &_media);
    void lineEdit_song_editingFinished();

    void on_pushButton_send_clicked();

    void on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime);

    void on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime);

public slots:
    void sendSuccess();
private slots:
    void timeOver();
signals:
    void sendMatch(const RewardInfo &info);


private:
    Ui::SendDialog *ui;
    MysqlQuery    *_sql;
    MysqlQuery    *_sqlInfo;

    QString serial_id;
    Media media;
    RewardInfo info;

    QTimer *timer;
};

#endif // SENDDIALOG_H
