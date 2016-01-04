#include "selectwinnerdialog.h"
#include "ui_selectwinnerdialog.h"

#include "tablemodel.h"
#include "yqcdelegate.h"
#include "kmatch.h"

#include <Common.h>
#include "Instance.h"
#include "Media.h"
#include "MediaPlayer.h"

SelectWinnerDialog::SelectWinnerDialog(QWidget *parent, int _selectId) :
    QDialog(parent),
    selectBoxId(_selectId),
    ui(new Ui::SelectWinnerDialog)
{
    ui->setupUi(this);
    initTable();
    _instance = new VlcInstance(VlcCommon::args(), this);
    _player = new VlcMediaPlayer(_instance);
    connect(_player, &VlcMediaPlayer::stopped, this, &SelectWinnerDialog::playerStop);

    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->clear();
}

SelectWinnerDialog::~SelectWinnerDialog()
{
    delete _player;
    delete _media;
    delete _instance;
    delete ui;
}

void SelectWinnerDialog::closeEvent(QCloseEvent *)
{
    if(_player)
        _player->stop();
}

void SelectWinnerDialog::initTable()
{
    model = new TableModel(this);
    ui->widget_tableView->setModel(model);
    ui->widget_tableView->setAlternatingRowColors(true);

    ui->widget_tableView->setItemDelegate(new NoFocusDelegate());
    ui->widget_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->widget_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->widget_tableView->horizontalHeader()->setHighlightSections(false);
    ui->widget_tableView->verticalHeader()->setVisible(false);
    ui->widget_tableView->setShowGrid(false);
    ui->widget_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->widget_tableView->verticalHeader()->setDefaultSectionSize(70);
    QHeaderView *headerView = ui->widget_tableView->horizontalHeader();
    //    headerView->setStretchLastSection(true);  ////最后一行适应空余部分
    headerView->setSectionResizeMode(QHeaderView::Stretch); //平均列宽

    ui->widget_tableView->show();
    ui->widget_tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headerList;
    headerList << "包房号"  << "分数" << "MP3" << "音频" << "操作";

    ButtonDelegate *delegate = new ButtonDelegate(this);
    ButtonDelegate *winDelegate = new ButtonDelegate(this);
    delegate->setButtonText("收听");
    winDelegate->setButtonText("胜者");
    ui->widget_tableView->setItemDelegateForColumn(3, delegate);
    ui->widget_tableView->setItemDelegateForColumn(4, winDelegate);

    connect(delegate, &ButtonDelegate::currentRow, this, &SelectWinnerDialog::leftListen);
    connect(winDelegate, &ButtonDelegate::currentRow, this, &SelectWinnerDialog::rightWinner);

    model->setHorizontalHeaderList(headerList);
    model->refrushModel();
    ui->widget_tableView->setColumnHidden(2, true);
}

void SelectWinnerDialog::setTableValue(QList<Record> _records)
{
    records.clear();
    rowList.clear();
    records = _records;
    if(records.size() > 0){
        for(int i=0; i<records.size(); i++)
        {
            QStringList row;
            if (records.at(i).boxid == selectBoxId && selectBoxId != 0) {

                ui->label->setText(QString("本次悬赏胜者包房号：%1 ").arg(records.at(i).roomno));
            }
//            row << QString::number(i+1);
            row << records.at(i).roomno;
            row << QString::number(records.at(i).score);
            row << records.at(i).url;

            rowList.append(row);
        }
    }

    model->setModalDatas(&rowList);
}

void SelectWinnerDialog::hiddenOperate(bool hidden)
{
    ui->widget_tableView->setColumnHidden(3, hidden);
}

void SelectWinnerDialog::sendSuccess()
{
    this->close();
}

void SelectWinnerDialog::rightWinner(const int &row)
{
    Record record = records.at(row);
    emit(winner(record.boxid));
}

void SelectWinnerDialog::playerStop()
{
    ui->label->clear();
}

void SelectWinnerDialog::leftListen(const int &row)
{
    QStringList list = rowList.at(row);
    QString url = list.at(2);
    if (url.isEmpty())
        return;

    _media = new VlcMedia(url, _instance);

    _player->open(_media);


    ui->label->setText(QString("正在收听包房号：%1 音频……").arg(list.at(0)));
}

