#include "senddialog.h"
#include "ui_senddialog.h"
#include "mysqlquery.h"
#include "addsongdialog.h"

#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QSqlQuery>
#include <QPainter>
#include <QDebug>
//#include <QSettings>

#define DATABASE_NAME "yiqiding_ktv"

#define TAKT_TIME 1000

SendDialog::SendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendDialog)
{
    ui->setupUi(this);
    initWidget();

    serial_id = "";
    _sql = new MysqlQuery();
    _sqlInfo = new MysqlQuery();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SendDialog::timeOver);
    connect(ui->lineEdit_song, &QLineEdit::editingFinished, this, &SendDialog::lineEdit_song_editingFinished);
}

SendDialog::~SendDialog()
{
    delete ui;
}

void SendDialog::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SendDialog::initWidget()
{
    ui->lineEdit->setText("当前KTV");
    ui->pushButton->setText("添加歌曲");
    ui->pushButton_send->setText("发起悬赏");

    ui->label_name->setText("发起方");
    ui->label_song->setText("歌曲");
    ui->label_timeStart->setText("开始时间");
    ui->label_timeEnd->setText("结束时间");
    ui->label_info->setText("输入悬赏奖励及数量");

    QDateTime currTime = QDateTime::currentDateTime();
    ui->dateTimeEdit_start->setDateTime(currTime);
    ui->dateTimeEdit_end->setDateTime(currTime.addDays(1));

    ui->label_name->setMinimumHeight(36);
    ui->label_song->setMinimumHeight(36);
    ui->label_timeStart->setMinimumHeight(36);
    ui->label_timeEnd->setMinimumHeight(36);
    ui->label_info->setMinimumHeight(36);

    ui->lineEdit->setMinimumHeight(36);
    ui->lineEdit_info->setMinimumHeight(36);
    ui->lineEdit_song->setMinimumHeight(36);

    ui->pushButton->setMinimumSize(90, 36);
    ui->pushButton_send->setMinimumSize(90, 36);

    ui->dateTimeEdit_start->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_end->setDateTime(ui->dateTimeEdit_start->dateTime().addDays(1));

    ui->label_remain->clear();
    ui->label_tooltip->setText("注：发起后两小时自动结束");
    ui->lineEdit->setEnabled(false);
    ui->lineEdit_song->setEnabled(false);
    ui->label_timeEnd->setHidden(true);
    ui->dateTimeEdit_end->setHidden(true);


    ui->pushButton->setObjectName("Button");
    ui->pushButton_send->setObjectName("Button");
    ui->lineEdit->setObjectName("LineEdit");
    ui->lineEdit_info->setObjectName("LineEdit");
    ui->lineEdit_song->setObjectName("LineEdit");
}



void SendDialog::on_pushButton_clicked()
{
    AddSongDialog *song = new AddSongDialog(this, _sql);
    connect(song, &AddSongDialog::songName, this, &SendDialog::setSongName);
    song->exec();
}

void SendDialog::setSongName(const Media &_media)
{
    media = _media;
    if(!_media.name.isEmpty() && !_media.serial_id.isEmpty()){
        ui->lineEdit_song->setText(_media.name);
        serial_id = _media.serial_id;
    }
}

void SendDialog::lineEdit_song_editingFinished()
{
    if(ui->lineEdit_song->text().isEmpty())
        return;

    if(_sql->isMatch(ui->lineEdit_song->text(), serial_id))
    {

    }
    else
    {
        QMessageBox::warning(this, "提示", QString("歌曲：%1不可K歌。")
                            .arg(ui->lineEdit_song->text()));
    }
}

void SendDialog::on_pushButton_send_clicked()
{
    if(ui->lineEdit_info->text().isEmpty()){
        QMessageBox::warning(this, "提示", "悬赏奖励和数量不能为空。");
        return;
    }
    _sql->openMysql();
    if(!_sql->isMatch(ui->lineEdit_song->text(), serial_id)){
        QMessageBox::warning(this, "提示", QString("歌曲：%1不可K歌。")
                            .arg(ui->lineEdit_song->text()));
        _sql->closeMysql();
        return;
    }
    _sql->closeMysql();

    _sqlInfo->openInfosql();
    info.ktv_name = ui->lineEdit->text().isEmpty()?QString("当前KTV"):ui->lineEdit->text();
    info.name = ui->lineEdit_song->text();
    info.singer = media.singer;
    info.mid = media.mid;
    info.serial_id = media.serial_id;
    info.startTime = ui->dateTimeEdit_start->text();
    info.endTime = ui->dateTimeEdit_end->text();
    info.reward = ui->lineEdit_info->text();
    info.status = QString::number(0);
    info.ids = QString::number(QDateTime::currentDateTime().toTime_t());

//    QString datetime = QDateTime::currentDateTime().toString("yyyyMMdd");
//    qint64 maxIds =  _sqlInfo->getMaxIds(datetime.toLongLong());
//    if(maxIds == -1){
//        maxIds = datetime.toLongLong()*100;
//    }
//    else{
//        maxIds = maxIds+1;
//    }
//    qDebug() << "max IDs : " << maxIds;
//    info.ids = QString::number(maxIds);

//    _sqlInfo->insertReward(info);

    _sqlInfo->closeInfosql();    
    timer->start(TAKT_TIME);
}

void SendDialog::on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime)
{
    QDateTime current = QDateTime::currentDateTime();
    current = current.addSecs(-3);
    if(dateTime < current)
    {
        ui->dateTimeEdit_start->setDateTime(current);
        QMessageBox::warning(this, "提示", QString("开始时间：%1\n"
                                          "要大于当前时间：%2").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"))
                                                           .arg(current.toString("yyyy-MM-dd hh:mm:ss")));

    }
}

void SendDialog::on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime)
{
    QDateTime start = ui->dateTimeEdit_start->dateTime();
    if(dateTime < start)
    {
        start.addDays(1);
        ui->dateTimeEdit_end->setDateTime(start);
        QMessageBox::warning(this, "提示", QString("结束时间：%1\n"
                                          "不得大于开始时间：%2").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"))
                                                           .arg(ui->dateTimeEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss")));

    }
}

void SendDialog::sendSuccess()
{
    this->close();
}

void SendDialog::timeOver()
{
    QDateTime currentDate = QDateTime::currentDateTime();
    QDateTime startDate = ui->dateTimeEdit_start->dateTime();


    int  remain  = startDate.toTime_t() - currentDate.toTime_t();

    if(remain > 0) {

        int h = remain/(60*60);
        int m = (remain%(60*60))/60;
        int s = (remain%(60*60))%60;
        QString str = QString("%1:%2:%3").arg(h).arg(m).arg(s);
        ui->label_remain->setText(QString("剩余时间 ：%1").arg(str));
    } else {

        ui->label_remain->clear();
        emit sendMatch(info);

        if(timer->isActive()){
            timer->stop();
        }
    }
}
