#ifndef KMATCH_H
#define KMATCH_H

#include <QMainWindow>

namespace Ui {
class KMatch;
}

class QTimer;
class QTcpSocket;
class RewardInfo;
class MysqlQuery;
class TableModel;

///记录
struct Record{

    int     boxid;
    double  score;
    bool    status;
    QString roomno;
    QString roomname;
    QString url;
};

///award
struct Award{
    QString desc;
    QString aid;
};

struct Rewards{
    int         id;
    int         boxid;
    int         time;
    int         remaintime;
    int         status;
    int         selectedboxid;
    int         serial_id;
    int         mid;
    QString     roomno;
    QString     roomname;
    QString     name;
    QString     singer;
    Award       award;
    QList<Record> records;
};

class KMatch : public QMainWindow
{
    Q_OBJECT

public:
    explicit KMatch(QWidget *parent = 0);
    ~KMatch();
    void paintEvent(QPaintEvent *);

public slots:
    void sendMatch(const RewardInfo &info);

private slots:
    void on_pushButton_clicked();

    void show_detail(const int &_row);
    void select_win(const int &boxid);
    void over_reward(const int &row);

    void clearAlertLabelText();
    void testHeart();
    void readData();

    void on_pushButton_2_clicked();

private:
    void initWidget();
    void initTableView();
    void setRecordColumnWidth();

    void setRecordAndListValue();
    QString returnStatus(const int &status);

    void getHeader(const int &num, const int &length, char *str);
    bool sendSocket(const char *str, const int &length);

    void heart();
    void addReward();
    void notifyAddReward();
    void getMyReward();
    void selectReward();
    void cancelReward();
    void alertPlayReward();
    Award ParserAward(QJsonValue value);
    Record ParserRecord(QJsonValue value);
    QList<Record> pareserRecordMulti(QJsonValue value);

public:

    static void writeLog(const QString &log);
    static QJsonValue jsonValue(const QString &json, const QString &key);
    static QVector<QJsonValue> jsonValueArray(const QString &json, const QString &key);

signals:
    void emitSuccess();

private:
    Ui::KMatch *ui;

    MysqlQuery    *_sqlinfo;
    QList <Rewards> rewardValues;
    QList< QStringList > rowList_record, rowList_list;
    TableModel *model_record;
    TableModel *model_list;

    QTcpSocket *socket;
    QString infoIp;
    QString roomno;
    int boxId;
    int row;

    QString packetStr;
    QTimer *timer;
    QTimer *clearalert;
};

#endif // KMATCH_H
