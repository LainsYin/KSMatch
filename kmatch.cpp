#include "kmatch.h"
#include "ui_kmatch.h"
#include "tablemodel.h"
#include "yqcdelegate.h"
#include "mysqlquery.h"
#include "selectwinnerdialog.h"

#include "senddialog.h"
#include "windows.h"
#include "winsock.h"

#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QtEndian>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QDataStream>
#include <QTextCodec>
#include <QFile>

#define REQ_KEEPALIVE               11
#define REQ_ADD_REWARD				14000 /*添加悬赏，向服务器添加悬赏*/
#define	REQ_GET_MY_REWARD     		14002 /*获取悬赏*/
#define	REQ_PLAY_REWARD     		14004 /*演唱悬赏*/ //演唱结束后向信息服务器发送积分录音信息  工具不做处理
#define REQ_GET_LIST_REWARD 		14006 /*悬赏列表*/
#define	REQ_SELECT_REWARD			14008 /*选择胜利包厢*/
#define	REQ_CANCEL_REWARD			14009 /*取消悬赏*/
#define	REQ_READY_PLAY_REWARD		14010 /*准备演唱悬赏*/

#define	 BOX_NOTIFY_ADD_REWARD      14001 /* 通知 有新的悬赏 */ //
#define	 BOX_ALERT_ENDING_REWARD    14003 /* 提醒发起人 */
#define	 BOX_NOTIFY_ENDED_REWARD    14005 /* 通知所有人结束 */
#define	 BOX_ALERT_PLAY_REWARD      14007 /* 提醒发起人 有人演唱 */
#define	 BOX_ALERT_CANCEL_REWARD	14011  /* 提醒取消 */
#define	 BOX_ALERT_ME_CALCEL_REWARD 14012  /*无人参加结束提醒*/


#define  CLEAN_LABEl_TIME  5000


KMatch::KMatch(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KMatch)
{
    ui->setupUi(this);
    initWidget();
    initTableView();


    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    clearalert = new QTimer(this);
    _sqlinfo = new MysqlQuery();

    connect(timer, &QTimer::timeout, this, &KMatch::testHeart);
    connect(clearalert, &QTimer::timeout, this, &KMatch::clearAlertLabelText);

    QSettings *config;
    config = new QSettings("config.ini", QSettings::IniFormat);
    config->setIniCodec("UTF-8");
    infoIp = config->value("SERVER/host", "127.0.0.1").toString();
    boxId = config->value("SERVER/boxId", "53").toInt();
    roomno = config->value("SERVER/roomno", "10001").toString();

    socket->connectToHost(infoIp, 58849, QIODevice::ReadWrite);//25377
    timer->start(1000);
    connect(socket, &QTcpSocket::readyRead, this, &KMatch::readData);


    ui->pushButton_2->setHidden(true);
    on_pushButton_2_clicked();

    row = -1;
}

KMatch::~KMatch()
{
    delete ui;

    if(timer->isActive())
        timer->stop();
}

void KMatch::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void KMatch::initWidget()
{
    ui->listWidget->setFocusPolicy(Qt::NoFocus);
    ui->listWidget->setMouseTracking(true);
    ui->listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->listWidget->setMaximumWidth(150);
    ui->pushButton->setMinimumSize(96, 36);
    ui->pushButton->setText("发起悬赏");

    ui->label_connect->setHidden(true);
    ui->label_connect->clear();
    ui->label_alertPlayReward->clear();
    ui->label_connect_text->setText("未连接信息服务器-_-~~~");


    QFile qss(":/Resources/title.qss");
    qss.open(QFile::ReadOnly);
    this->setStyleSheet(qss.readAll());
    qss.close();

    ui->pushButton->setObjectName("Button");
}

void KMatch::initTableView()
{
    model_record = new TableModel(this);
    model_list = new TableModel(this);
    ui->widget_record->setModel(model_record);
    ui->widget_record->setAlternatingRowColors(true);

    ui->widget_record->setItemDelegate(new NoFocusDelegate());
    ui->widget_record->setSelectionMode(QAbstractItemView::NoSelection);
    ui->widget_record->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->widget_record->horizontalHeader()->setHighlightSections(false);
    ui->widget_record->verticalHeader()->setVisible(false);
    ui->widget_record->setShowGrid(false);
    ui->widget_record->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->widget_record->verticalHeader()->setDefaultSectionSize(70);
    QHeaderView *headerView = ui->widget_record->horizontalHeader();
    headerView->setStretchLastSection(true);  ////最后一行适应空余部分
    headerView->setSectionResizeMode(QHeaderView::Stretch); //平均列宽

    ui->widget_record->show();
    ui->widget_record->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headerList;
    headerList << "编号" << "发起方" << "歌曲名_MID" << "时间" << "奖励" << "状态" << "演唱记录" << "操作";

    ButtonDelegate *showDelegate = new ButtonDelegate(this);
    ButtonDelegate *opDelegate = new ButtonDelegate(this);
    showDelegate->setButtonText("查看");
    opDelegate->setButtonText("结束");
    ui->widget_record->setItemDelegateForColumn(6, showDelegate);
    ui->widget_record->setItemDelegateForColumn(7, opDelegate);

    connect(showDelegate, &ButtonDelegate::currentRow, this, &KMatch::show_detail);
    connect(opDelegate, &ButtonDelegate::currentRow, this, &KMatch::over_reward);

    model_record->setHorizontalHeaderList(headerList);
    model_record->refrushModel();



    ui->widget_list->setModel(model_list);
    ui->widget_list->setAlternatingRowColors(true);

    ui->widget_list->setItemDelegate(new NoFocusDelegate());
    ui->widget_list->setSelectionMode(QAbstractItemView::NoSelection);
    ui->widget_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->widget_list->horizontalHeader()->setHighlightSections(false);
    ui->widget_list->verticalHeader()->setVisible(false);
    ui->widget_list->setShowGrid(false);
    ui->widget_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->widget_list->verticalHeader()->setDefaultSectionSize(70);
    QHeaderView *headerView_list = ui->widget_list->horizontalHeader();
//        headerView->setStretchLastSection(true);  ////最后一行适应空余部分
    headerView_list->setSectionResizeMode(QHeaderView::Stretch); //平均列宽

    ui->widget_list->show();
    ui->widget_list->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headers;
    headers << "流水单号" << "编号" << "所在KTV" << "歌曲名_MID" << "有效时间" << "奖励";

    model_list->setHorizontalHeaderList(headers);
    model_list->refrushModel();
}

void KMatch::setRecordColumnWidth()
{
    int wid = ui->record->width()/8;
    ui->widget_record->setColumnWidth(0, wid*0.5);
    ui->widget_record->setColumnWidth(1, wid*1);
    ui->widget_record->setColumnWidth(2, wid*1);
    ui->widget_record->setColumnWidth(3, wid*1.5);
    ui->widget_record->setColumnWidth(4, wid*1);
    ui->widget_record->setColumnWidth(5, wid*0.5);
    ui->widget_record->setColumnWidth(6, wid*0.5);
    ui->widget_record->setColumnWidth(7, wid*2);
}

void KMatch::setRecordAndListValue()
{
    rowList_record.clear();
    rowList_list.clear();

    for (int i=0; i<rewardValues.size(); i++){

        QStringList value;
        QStringList listValue;
        listValue.clear();
        value.clear();

        Rewards  reward = rewardValues.at(i);
        value << QString::number(i+1);
        value << reward.roomname;
        QString ktv = QString("%1_%2").arg(reward.name).arg(reward.serial_id);
        value << ktv;
        QDateTime datetime;
        datetime.setTime_t(reward.time);
        QString time = QString("%1").arg(datetime.toString("yyyy-MM-dd hh:mm:ss"));
        value << time;
        value << reward.award.desc;
        value << returnStatus(reward.status);
        rowList_record.append(value);


        listValue << QString::number(reward.id);
        listValue << QString::number(i+1);
        listValue << reward.roomname;
        listValue << ktv;
        listValue << time;
        listValue << reward.award.desc;
        rowList_list.append(listValue);

    }
    model_record->setModalDatas(&rowList_record);
    model_list->setModalDatas(&rowList_list);
}

QString KMatch::returnStatus(const int &status)
{
    if(status == 0){
        return "进行中";
    }else if(status == 1){
        return "已结束";
    }else if(status == 2){
        return "已取消";
    }
}

void KMatch::getHeader(const int &num, const int &length, char *str)
{
    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    int rand = qrand()%1000;

    int header[6];
    header[0] = 17;
    header[1] = 100;
    header[2] = num;
    header[3] = rand;
    header[4] = length;
    header[5] = boxId;

    for (int i = 0 ; i < 6; ++i)
    {
        header[i] = qToBigEndian(header[i]);
    }
    memcpy(str, header, 24);
}

bool KMatch::sendSocket(const char *str, const int &length)
{
    if(socket->state() != QAbstractSocket::UnconnectedState){
        int retLength = socket->write(str, length);

        writeLog(QString(" [socket send] 发送成功字节：%1 字节数：%2").arg(retLength).arg(length));

        return true;

    }else{
        QMessageBox::warning(this, "提示", "与信息服务器断开连接！");
        writeLog(QString(" [info ip] %1 %2").arg(infoIp).arg("与信息服务器断开连接"));
    }

    return false;
}

void KMatch::heart()
{
    if(packetStr.indexOf("Authentication failure") != -1)
    {
        ui->label_connect->setText(QString::number(0));
        ui->label_connect_text->setText("验证失败");
        ui->label_connect_text->setStyleSheet("color:rgb(255, 0, 0);");
    }
}

void KMatch::addReward()
{
    bool confirm = jsonValue(packetStr, "confirm").toBool();
    if(!confirm){

        writeLog(QString(" [%1] confirm = %2").arg(REQ_ADD_REWARD).arg(confirm));
        QMessageBox::warning(this, "提示", "添加悬赏失败。");
    }
}

void KMatch::notifyAddReward()
{

}

void KMatch::getMyReward()
{
    QJsonValue status = jsonValue(packetStr, "status");
    qDebug() << " status :  " << status.toInt();
    QVector<QJsonValue> values = jsonValueArray(packetStr, "rewards");

    rewardValues.clear();
    foreach (QJsonValue json, values) {

        QJsonDocument document;
        document.setObject(json.toObject());
        QByteArray array = document.toJson(QJsonDocument::Indented);
//        qDebug() << " array : " << array;

        QList<Record> records;
        Award award;
        Rewards reward;
        records.clear();
        QString value = QString(array);
        QJsonValue tempJson = jsonValue(value, "award");
        if(!tempJson.isNull()){

            award = ParserAward(tempJson);
        }
        reward.award = award;

        QJsonValue recordJson = jsonValue(value, "records");
        if(!recordJson.isNull()){
            records = pareserRecordMulti(json);
        }
        if(!records.isEmpty())
            reward.records = records;

        reward.id = jsonValue(value, "id").toInt();
        reward.boxid = jsonValue(value, "boxid").toInt();
        reward.time = jsonValue(value, "time").toInt();
        reward.remaintime = jsonValue(value, "remaintime").toInt();
        reward.status = jsonValue(value, "status").toInt();
        reward.selectedboxid = jsonValue(value, "selectedboxid").toInt();
        reward.serial_id = jsonValue(value, "serial_id").toString().toInt();
        reward.roomno = jsonValue(value, "roomno").toString();
        reward.roomname = jsonValue(value, "roomname").toString();
        reward.name = jsonValue(value, "name").toString();
        reward.singer = jsonValue(value, "singer").toString();

        rewardValues.append(reward);
    }

    setRecordAndListValue();
}

void KMatch::selectReward()
{
    bool confirm;
    confirm = jsonValue(packetStr, "confirm").toBool();

    if(!confirm){
        QMessageBox::information(this, "标题", "选择胜者返回失败。");
        writeLog(QString("选择胜者返回失败。"));
    } else {

        on_pushButton_2_clicked();
        emit(emitSuccess());
    }
}

void KMatch::cancelReward()
{
    bool confirm;
    confirm = jsonValue(packetStr, "confirm").toBool();

    if(!confirm){
        QMessageBox::information(this, "标题", "结束悬赏失败。");

        writeLog(QString("结束悬赏失败。"));
    } else {
        on_pushButton_2_clicked();
    }
}

void KMatch::alertPlayReward()
{
    QString no = jsonValue(packetStr, "roomno").toString();
//    QString name = jsonValue(packetStr, "roomname");
//    int time = jsonValue(packetStr, "id");
//    QDateTime datetime;
//    datetime.setTime_t(time);
//    QString datetimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    clearalert->start(CLEAN_LABEl_TIME);
    QString text = QString("房间号：%1 演唱结束").arg(no);
    ui->label_alertPlayReward->setText(text);
    on_pushButton_2_clicked();
}

Award KMatch::ParserAward(QJsonValue value)
{
    QJsonDocument document;
    document.setObject(value.toObject());
    QByteArray array = document.toJson(QJsonDocument::Indented);

    Award award;
    QString ss = QString::fromLocal8Bit(array);
    award.aid = jsonValue(QString(array), "aid").toString();
    award.desc = jsonValue(ss, "desc").toString();
    qDebug() << " desc : " << award.desc;

    return award;
}

Record KMatch::ParserRecord(QJsonValue value)
{
    QJsonDocument document;
    document.setObject(value.toObject());
    QByteArray array = document.toJson(QJsonDocument::Indented);


    Record  record;
    QString _value(array);
    record.boxid = jsonValue(_value, "boxid").toInt();
    record.roomname = jsonValue(_value, "roomname").toString();
    record.roomno = jsonValue(_value, "roomno").toString();
    record.score = jsonValue(_value, "score").toDouble();
    record.status = jsonValue(_value, "status").toBool();
    record.url = jsonValue(_value, "url").toString();

    return record;
}

QList<Record> KMatch::pareserRecordMulti(QJsonValue value)
{
    QJsonDocument document;
    document.setObject(value.toObject());
    QByteArray array = document.toJson(QJsonDocument::Indented);
    QVector<QJsonValue> values = jsonValueArray(QString(array), "records");

    qDebug() << " record :  " << QString(array);

    QList<Record> records;
    records.clear();
    foreach (QJsonValue json, values) {

        records.append(ParserRecord(json));
    }

    return records;
}

void KMatch::on_pushButton_clicked()
{
    SendDialog *send = new SendDialog(this);
    connect(send, &SendDialog::sendMatch, this, &KMatch::sendMatch);
    connect(this, &KMatch::emitSuccess, send, &SendDialog::sendSuccess);
    send->exec();
}

///发送悬赏
void KMatch::sendMatch(const RewardInfo &info)
{
    QJsonObject json;
    json.insert("boxid", QJsonValue(boxId));
    json.insert("roomno", QJsonValue(roomno));
    json.insert("roomname", QJsonValue(info.ktv_name));
    json.insert("serial_id", QJsonValue(info.mid));
    json.insert("name", QJsonValue(info.name));
    json.insert("singer", QJsonValue(info.singer));

    QJsonObject awardJ;
    awardJ.insert("desc", QJsonValue(info.reward));
    awardJ.insert("aid", QJsonValue(info.ids));

    json.insert("award", QJsonValue(awardJ));

    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    int length = byteArray.length();
    qDebug() << " data length :  " << byteArray.length()<< " byte Array : " << byteArray;

    ///socket 信息服务器
    char str[length+24];
    memset(str, 0, length+24);
    getHeader(REQ_ADD_REWARD, length, str);
    memcpy(str+24, byteArray, length);

    if(sendSocket(str, length+24)) {

        QMessageBox::information(this, "提示", "成功发起悬赏！");
        on_pushButton_2_clicked();
        emit(emitSuccess());
    } else {
        QMessageBox::information(this, "提示", "悬赏发起失败！");
    }
}

///从信息服务器获取悬赏列表
void KMatch::on_pushButton_2_clicked()
{
    char str[24];
    memset(str, 0, 24);
    getHeader(REQ_GET_MY_REWARD, 0, str);

    sendSocket(str, 24);
}

///发送心跳
void KMatch::testHeart()
{
    int header[6];
    header[0] = 17;
    header[1] = 100;
    header[2] = 11;
    header[3] = 0; //随机数
    header[4] = 0; //长度
    header[5] = boxId;

    for (int i = 0 ; i < 6 ; ++i)
    {
        header[i] = qToBigEndian(header[i]); //  htonl(header[i]); //  ntohs
    }
    char sendArray[24];
    memset(sendArray, 0, 24);
    memcpy(sendArray, header, 24);

    if(socket->state() != QAbstractSocket::UnconnectedState){
        int retLength = socket->write(sendArray, 24); //(char *)header, 24
        if(retLength == 24){
            ui->label_connect->setText(QString::number(1));
            ui->label_connect_text->setText("已连接信息服务器");
            ui->label_connect_text->setStyleSheet("color:rgb(0, 0, 255);");
        }
    }else{
        ui->label_connect->setText(QString::number(0));
        ui->label_connect_text->setText("信息服务器连接断开!!!");
        ui->label_connect_text->setStyleSheet("color:rgb(255, 0, 0);");
    }
}

///接收服务器数据
void KMatch::readData()
{
    packetStr.clear();
    QByteArray readArray = socket->readAll();

    ///包头
    int array[6];
    memset(array, 0, 24);
    QByteArray headerArray = readArray.left(24);
    memcpy(array, headerArray, headerArray.length());

    //数据包
    int length = readArray.length() - 24;
    QByteArray dataArray = readArray.right(length);
    packetStr = QString(dataArray);

    for(int i=0; i<6; i++){
        array[i] = qFromBigEndian(array[i]);
    }

    switch (array[2]) {
    case REQ_KEEPALIVE:         heart();           break;
    case BOX_NOTIFY_ADD_REWARD: notifyAddReward(); break;
    case REQ_ADD_REWARD:        addReward();       break;
    case REQ_GET_MY_REWARD:     getMyReward();     break;
    case REQ_SELECT_REWARD:     selectReward();    break;
    case REQ_CANCEL_REWARD:     cancelReward();    break;
    case BOX_ALERT_PLAY_REWARD: alertPlayReward(); break;

    default: writeLog(QString(" Request not supported   %1").arg(packetStr));   break;
    }
}

void KMatch::show_detail(const int &_row)
{
    Rewards reward = rewardValues.at(_row);
    SelectWinnerDialog *dialog = new SelectWinnerDialog(this, reward.selectedboxid);
    connect(dialog, &SelectWinnerDialog::winner, this, &KMatch::select_win);
    connect(this, &KMatch::emitSuccess, dialog, &SelectWinnerDialog::sendSuccess);
    row = _row;
    dialog->setTableValue(reward.records);
    dialog->hiddenOperate(false);
    dialog->exec();
    row = -1;
}

void KMatch::select_win(const int &boxid)
{
    Rewards reward = rewardValues.at(row);
    if(reward.status == 0){
        QJsonObject json;
        json.insert("id", QJsonValue(reward.id));
        json.insert("boxid", QJsonValue(boxid));

        QJsonDocument document;
        document.setObject(json);
        QByteArray byteArray = document.toJson(QJsonDocument::Compact);
        int length = byteArray.length();
        char str[length+24];
        memset(str, 0, length+24);
        getHeader(REQ_SELECT_REWARD, length, str);
        memcpy(str+24, byteArray, length);

        sendSocket(str, length+24);
    }else{
        QMessageBox::warning(this, "提示", "该悬赏取消或已结束。");
    }
}

///取消悬赏
void KMatch::over_reward(const int &row)
{
//    Rewards reward = rewardValues.at(row);
//    Award award = reward.award;

//    QList<Record> records = reward.records;
//    QJsonArray recordArray;
//    for(int i=0; i<records.size(); i++)
//    {
//        QJsonObject json;
//        json.insert("boxid", QJsonValue(records.at(i).boxid));
//        json.insert("roomname", QJsonValue(records.at(i).roomname));
//        json.insert("roomno", QJsonValue(records.at(i).roomno));
//        json.insert("score", QJsonValue(records.at(i).score));
//        json.insert("status", QJsonValue(records.at(i).status));
//        json.insert("url", QJsonValue(records.at(i).url));

//        recordArray.append(json);
//    }

//    QJsonObject awardJson;
//    awardJson.insert("aid", QJsonValue(award.aid));
//    awardJson.insert("desc", QJsonValue(award.desc));

//    QJsonObject json;
//    json.insert("id", QJsonValue(reward.id));
//    json.insert("award", QJsonValue(awardJson));
//    json.insert("serial_id", QJsonValue(reward.serial_id));
//    json.insert("name", QJsonValue(reward.name));
//    json.insert("singer", QJsonValue(reward.singer));
//    json.insert("records", QJsonValue(recordArray));



    Rewards reward = rewardValues.at(row);
    if(reward.status == 0){

        QJsonObject json;
        json.insert("id", QJsonValue(reward.id));

        QJsonDocument document;
        document.setObject(json);
        QByteArray byteArray = document.toJson(QJsonDocument::Compact);
        int length = byteArray.length();
        char str[length+24];
        memset(str, 0, length+24);
        getHeader(REQ_CANCEL_REWARD, length, str);
        memcpy(str+24, byteArray, length);

        sendSocket(str, length+24);
    }else{
        QMessageBox::warning(this, "提示", "该悬赏取消或已结束。");
    }
}

void KMatch::clearAlertLabelText()
{
    ui->label_alertPlayReward->clear();

    if(clearalert->isActive()){
        clearalert->stop();
    }
}

void KMatch::writeLog(const QString &log)
{
    QString filepath = QApplication::applicationDirPath();
    filepath.append("/kmatch.log");
    QFile logfile(filepath);
    if(logfile.open(QIODevice::Append | QIODevice::WriteOnly))
    {
        QString text = QString("[%1]  ").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        text.append(log);
        text.append("\r\n");
        logfile.write(text.toLocal8Bit());

        logfile.close();
    }
}

QJsonValue KMatch::jsonValue(const QString &json, const QString &key)
{
    QJsonValue retValue;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(json.toLocal8Bit(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject json = parse_doucment.object();
            if(json.contains(key)){
                retValue = json.take(key);
            }
        }
    }

    return retValue;
}

QVector<QJsonValue> KMatch::jsonValueArray(const QString &json, const QString &key)
{
    QVector<QJsonValue> values;
    values.clear();
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(json.toLocal8Bit(), &json_error);

    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject json = parse_doucment.object();
            if(json.contains(key))
            {
                QJsonArray array = json[key].toArray();
                for (int i=0; i<array.size(); i++)
                {
                    QJsonValue value = array.at(i);
                    values.append(value);
                }
            }
        }
    }

    return values;
}

