#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port=new AbstractSerial(this);
    port->setTotalReadConstantTimeout(0);

    timer=new QTimer();

    p_uso=new proto_uso(this,port);
     QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));





     connect(p_uso,SIGNAL(DEV_INFO_RESPONSED(QByteArray)),this,SLOT(on_dev_info_responsed(QByteArray)));
     connect(p_uso,SIGNAL(GET_ALL_DATA_RESPONSED()),this,SLOT(on_dev_get_data_responsed()));

  //   connect(ui->action_LOAD,SIGNAL(triggered()),this,SLOT(on_menu_load_settings_clicked()));
   //  connect(ui->action_SAVE,SIGNAL(triggered()),this,SLOT(on_menu_save_settings_clicked()));

     connect(timer,SIGNAL(timeout()),this,SLOT(Get_All_Data()));


     strlist<<tr("±10 мВ")<<tr("±20 мВ")<<tr("±40 мВ")<<tr("±80 мВ")<<tr("±160 мВ")<<tr("±320 мВ")<<tr("±640 В")<<tr("±1.28 В");
     read_flag=false;
     ui->comboBox->addItems(strlist);



}

MainWindow::~MainWindow()
{

    port->close();
    delete port;
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
   if(!port->isOpen())
    {
    port->setDeviceName(com_dlg.ui->comboBox->currentText());


    if ( !port->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered) ) {
        qDebug() << "Serial device by default: " << port->deviceName() << " open fail.";
        ui->statusBar->showMessage("Port Open Fail");
        return;
    }
    if (!port->setBaudRate(AbstractSerial::BaudRate57600/*com_dlg.ui->comboBox_2->currentText()*/)) {
        qDebug() << "Set baud rate  error.";
        return;
    };

    if (!port->setDataBits(AbstractSerial::DataBits8)) {
        qDebug() << "Set data bits " <<  AbstractSerial::DataBits8 << " error.";
        return;
    }
    if (!port->setParity(AbstractSerial::ParityNone)) {
        qDebug() << "Set parity " <<  AbstractSerial::ParityNone << " error.";
        return;
    }

   if(com_dlg.ui->radioButton->isChecked())
    {
        if (!port->setStopBits(AbstractSerial::StopBits1)) {
            qDebug() << "Set stop bits " <<  AbstractSerial::StopBits1 << " error.";
            return;
        }
    }
   else
   {
       if (!port->setStopBits(AbstractSerial::StopBits2)) {
           qDebug() << "Set stop bits " <<  AbstractSerial::StopBits2 << " error.";
           return;
       }
   }

    if (!port->setFlowControl(AbstractSerial::FlowControlOff)) {
        qDebug() << "Set flow " <<  AbstractSerial::FlowControlOff << " error.";
        return;
    }

    if (port->openMode() & AbstractSerial::Unbuffered)
        port->setTotalReadConstantTimeout(10);

     //port->setCharIntervalTimeout(int((8*1000000/com_dlg.ui->comboBox_2->currentText().toInt())));

    ui->pushButton->setText(tr("Отключить"));

     //timer->start(30);

     qDebug() << "Serial device : " << port->deviceName() << " opened!!!.";
     qDebug() << "Serial device baudrate : " << port->baudRate() ;
     qDebug() << "Serial device databits : " << port->dataBits() ;
     qDebug() << "Serial device stopbits : " << port->stopBits() ;
     qDebug() << "Serial device flowcontrol : " << port->flowControl() ;


 }
else
   {
        port->close();
        ui->pushButton->setText(tr("Подключить"));
\
         //timer->stop();
    }
}


void MainWindow::on_action_COM_triggered()
{
    com_dlg.show();
}

/*void MainWindow::MessageReadOK(bool mess)
{
    if(mess)
    {
       ui->statusBar->showMessage("Device Read OK");
       //Get_All_Data();
    }
    else
    {
       ui->statusBar->showMessage("Device Read Error");
    }
}*/





void MainWindow::on_pushButton_3_clicked()
{
    p_uso->GET_DEV_INFO_REQ(ui->comboBox_5->currentText().toInt());
}

void MainWindow::on_dev_info_responsed(QByteArray response)
{
    union
    {
        quint32 res;
        float result;
    };

    quint8 adc_diap=0;
    quint16 display_time=0;

    res=(((quint32)(response.data()[7]&0xff)<<16  |(response.data()[6]&0xff)<<24|(response.data()[9]&0xff) |(response.data()[8]&0xff)<<8 ));
    ui->doubleSpinBox->setValue(result);

    res=(((quint32)(response.data()[11]&0xff)<<16  |(response.data()[10]&0xff)<<24|(response.data()[13]&0xff) |(response.data()[12]&0xff)<<8 ));
    ui->doubleSpinBox_2->setValue(result);

    res=(((quint32)(response.data()[15]&0xff)<<16  |(response.data()[14]&0xff)<<24|(response.data()[17]&0xff) |(response.data()[16]&0xff)<<8 ));
    ui->doubleSpinBox_3->setValue(result);

    res=(((quint32)(response.data()[19]&0xff)<<16  |(response.data()[18]&0xff)<<24|(response.data()[21]&0xff) |(response.data()[20]&0xff)<<8 ));
    ui->doubleSpinBox_4->setValue(result);

    adc_diap=response.data()[22];
    ui->comboBox->setCurrentIndex(adc_diap&0x7);

    display_time=((quint16)(response.data()[23]&0xff)<<8)|(quint32)(response.data()[24]&0xff);
    ui->spinBox->setValue(display_time);

    return;
}

void MainWindow::on_set_dev_info_clicked(void)
{

    QByteArray settings;
    union
    {
        quint8 res[4];
        float result;
    };

    settings.append((char)0x0);
    settings.append((char)0xD7);
    settings.append((char)0x29);//заголовок кадра
    settings.append(ui->comboBox_5->currentText().toInt());

    settings.append(p_uso->CHANNEL_SET_PARAMETERS_REQ_);

    settings.append(25);//длина оставшейся части кадра

    result=(float)ui->doubleSpinBox->value();
    settings.append(res[3]);
    settings.append(res[2]);
    settings.append(res[1]);
    settings.append(res[0]);

    result=(float)ui->doubleSpinBox_2->value();
    settings.append(res[3]);
    settings.append(res[2]);
    settings.append(res[1]);
    settings.append(res[0]);

    result=(float)ui->doubleSpinBox_3->value();
    settings.append(res[3]);
    settings.append(res[2]);
    settings.append(res[1]);
    settings.append(res[0]);

    result=(float)ui->doubleSpinBox_4->value();
    settings.append(res[3]);
    settings.append(res[2]);
    settings.append(res[1]);
    settings.append(res[0]);

    settings.append(ui->comboBox->currentIndex()&0x7);

    settings.append((quint8)((ui->spinBox->value()>>8)&0xff));
    settings.append((quint8)((ui->spinBox->value())&0xff));

    settings.append((char)0);//заглушка для ID
    settings.append((char)0);
    settings.append((char)0);
    settings.append((char)0);
    settings.append((char)0);

    qDebug()<<settings.toHex();
    p_uso->CHANNEL_SET_PARAMETERS_REQ(settings);

    return;
}

void MainWindow::on_pushButton_4_clicked()
{
    on_set_dev_info_clicked();
}
