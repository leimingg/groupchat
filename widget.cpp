#include "widget.h"
#include "ui_widget.h"
#include <QDataStream>
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>
#include <QFileDialog>
#include <QTextStream>

Widget::Widget(QWidget *parent,QString name)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    udpSocket=new QUdpSocket(this);
    uName=name;
    this->port=9999;
    udpSocket->bind(this->port,QUdpSocket::ShareAddress |QUdpSocket::ReuseAddressHint);
    sndMsg(UsrEnter);
    connect(ui->sendBtn,&QPushButton::clicked,[=](){
        sndMsg(Msg);
    });

    connect(udpSocket,&QUdpSocket::readyRead,this,&Widget::ReceiveMessage);

    connect(ui->exitBtn,&QPushButton::clicked,[=](){
        this->close();
    });

    //Additional functions:
    //    font
    connect(ui->fontCbx,&QFontComboBox::currentFontChanged,[=](const QFont &font){
        ui->msgTxtEdit->setCurrentFont(font);
        ui->msgTxtEdit->setFocus();
    });
    //font size
    void(QComboBox::*cbxsingal)(const QString &text)=&QComboBox::currentTextChanged;
    connect(ui->sizeCbx,cbxsingal,[=](const QString &text){
        ui->msgTxtEdit->setFontPointSize(text.toDouble());
        ui->msgTxtEdit->setFocus();
    });
    //bold
    connect(ui->boldTBtn,&QToolButton::clicked,[=](bool isCheck){
        if(isCheck){
            ui->msgTxtEdit->setFontWeight(QFont::Bold);
        }else{
            ui->msgTxtEdit->setFontWeight(QFont::Normal);
        }
    });
    //Italic
    connect(ui->italicTBtn,&QToolButton::clicked,[=](bool check){
        ui->msgTxtEdit->setFontItalic(check);
    });
    //underline
    connect(ui->underlineTBtn,&QToolButton::clicked,[=](bool check){
        ui->msgTxtEdit->setFontUnderline(check);
    });
    //font color
    connect(ui->colorTBtn,&QToolButton::clicked,[=](){
        QColor color=QColorDialog::getColor(Qt::red);
        ui->msgTxtEdit->setTextColor(color);
    });
    //empty chat history
    connect(ui->clearTBtn,&QToolButton::clicked,[=](){
       ui->msgBrowser->clear();
    });
    //save chat history
    connect(ui->saveTBtn,&QToolButton::clicked,[=](){
        if(ui->msgBrowser->document()->isEmpty()){
            QMessageBox::warning(this,"Warning","Path or content can not be empty!");
            return;
        }else{
             QString path=QFileDialog::getSaveFileName(this,"Save History","Chat History","(*.txt)");
            QFile file(path);
            file.open(QIODevice::WriteOnly|QIODevice::Text);
            QTextStream stream(&file);
            stream<<ui->msgBrowser->toPlainText();
            file.close();
        }
    });
}


void Widget::ReceiveMessage(){
    qint64 size=udpSocket->pendingDatagramSize();
    QByteArray array=QByteArray(10000,0);
    udpSocket->readDatagram(array.data(),size);
    QDataStream stream(&array,QIODevice::ReadOnly);
    int msgType;
    QString usrName;
    QString msg;
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    stream>>msgType;
    switch (msgType) {
    case Msg:
        stream>>usrName>>msg;
        ui->msgBrowser->setTextColor(Qt::blue);
        ui->msgBrowser->append("["+usrName+"]"+time);
        ui->msgBrowser->append(msg);
        break;
    case UsrEnter:
        stream>>usrName;
        usrEnter(usrName);
        break;

    case UsrLeft:
        stream>>usrName;
        usrLeft(usrName,time);
        break;
    default:
        break;
    }
}

void Widget::usrLeft(QString usrname,QString time){
   bool isEmpty= ui->usrTblWidget->findItems(usrname,Qt::MatchExactly).isEmpty();
    if(!isEmpty){
        int row=ui->usrTblWidget->findItems(usrname,Qt::MatchExactly).first()->row();
        ui->usrTblWidget->removeRow(row);

        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 left at %2 ").arg(usrname).arg(time));
        ui->usrNumLbl->setText(QString("Online: %1 Person").arg(ui->usrTblWidget->rowCount()));
    }
}
void Widget::usrEnter(QString username){

    bool isEmpty=ui->usrTblWidget->findItems(username,Qt::MatchExactly).isEmpty();
    if(isEmpty){
        QTableWidgetItem *usr=new QTableWidgetItem(username);
        ui->usrTblWidget->insertRow(0);
        ui->usrTblWidget->setItem(0,0,usr);

        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 Online").arg(username));

        ui->usrNumLbl->setText(QString("Online: %1 Person").arg(ui->usrTblWidget->rowCount()));
        sndMsg(UsrEnter);

    }
}
void Widget::sndMsg(MsgType type){
    QByteArray array;
    QDataStream stream(&array,QIODevice::WriteOnly);

    stream<<type<<getUsr();//add 1st part to stream
    switch(type){
    case Msg://normal chat message
        if(ui->msgTxtEdit->toPlainText()=="")//not send if nothing input
        {
                QMessageBox::warning(this,"Warning","Sent message cannot be empty");
                return;
        }
        stream<<getMsg();
        break;
    case UsrEnter://user enter chat
        break;
    case UsrLeft://user left chat
    default:
        break;
    }
    udpSocket->writeDatagram(array,QHostAddress::Broadcast,port);
}

QString Widget::getUsr(){
    return this->uName;

}
QString Widget::getMsg(){
    QString str=ui->msgTxtEdit->toHtml();
    ui->msgTxtEdit->clear();
    ui->msgTxtEdit->setFocus();
    return str;
}
void Widget:: closeEvent(QCloseEvent *e){
    emit this->closeWidget();
    sndMsg(UsrLeft);
    udpSocket->close();
    udpSocket->destroyed();
    QWidget::closeEvent(e);
}

Widget::~Widget()
{
    delete ui;
}

