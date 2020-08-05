#include "dialoglist.h"
#include "ui_dialoglist.h"
#include <QToolButton>
#include "widget.h"
#include "QMessageBox"

DialogList::DialogList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DialogList)
{
    ui->setupUi(this);
    //set window title
    setWindowTitle("GroupChat 2020");
    setWindowIcon(QPixmap(":/images/qq.png"));

    QList<QString>nameList;
      nameList << "James" << "Brenda" <<"Steve"<<"Cherry"<<"Sarah"
               <<"JiaoJiao"<<"Lucy"<<"Judy";


      QStringList iconNameList; //图标资源列表
      iconNameList << "spqy"<< "ymrl" <<"qq" <<"Cherry"<< "dr"
                   <<"jj"<<"lswh"<<"qmnn";

      QVector<QToolButton *> vToolBtn;

    for(int i=0;i<8;i++){
        //set user icon
        QToolButton *btn=new QToolButton;
        //set user name
        btn->setText(nameList[i]);
        QString str=QString(":/images/%1.png").arg(iconNameList.at(i));
        btn->setIcon(QPixmap(str));
        btn->setIconSize(QPixmap(str).size());
        btn->setAutoRaise(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        ui->vLayout->addWidget(btn);
        vToolBtn.push_back(btn);
        isShow.push_back(false);
    }

    for(int i=0;i<vToolBtn.size();i++){
        connect(vToolBtn[i],&QToolButton::clicked,[=](){
            if(isShow[i]){
                QString str=QString("%1Window was opened").arg(vToolBtn[i]->text());
                QMessageBox::warning(this,"Warning",str);
                return;
            }
            isShow[i]=true;
            Widget *widget=new Widget(0,vToolBtn[i]->text());
            widget->setWindowTitle(vToolBtn[i]->text());
            widget->setWindowIcon(vToolBtn[i]->icon());
            widget->show();

            connect(widget,&Widget::closeWidget,[=](){
                isShow[i]=false;
            });
        });
    }

}

DialogList::~DialogList()
{
    delete ui;
}
