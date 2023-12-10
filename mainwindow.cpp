#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //TCP Part
    TCP_Server = new QTcpServer();
    if(TCP_Server->listen(QHostAddress::Any, TcpPort)){
        connect(TCP_Server, &QTcpServer::newConnection, this, &MainWindow::newTcpConnection);
    }else{
        QMessageBox::information(this, "TCP Server Error", TCP_Server->errorString());
    }

    connect(ui->pushButton_SendTcpMessage, &QPushButton::clicked, this, &MainWindow::sendTcpMessage);


    //UDP part
    UDP_Server = new QUdpSocket();
    if(UDP_Server->bind(QHostAddress::Any, UdpPort, QUdpSocket::ShareAddress)){
         connect(UDP_Server, &QUdpSocket::readyRead, this, &MainWindow::readUdpData);
    }else{
        QMessageBox::information(this, "UDP Server Error", UDP_Server->errorString());
    }

    connect(ui->pushButton_SendUdpMessage, &QPushButton::clicked, this, &MainWindow::sendUdpMessage);

    ui->textEdit_UdpMessage->setText("000 aaa 12.13 42.44 16.44 33.21"); //initial text to test


    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendUdpMessage()));
    timer->start(UdpSendMessageFrequency);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newTcpConnection(){

    while(TCP_Server->hasPendingConnections()){
        AddToTcpSocketList(TCP_Server->nextPendingConnection());
    }
}

void MainWindow::AddToTcpSocketList(QTcpSocket *socket){

    TcpClientList.append(socket);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readTcpData);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardTcpSocket);
    ui->comboBox_TcpClientList->addItem(QString::number(socket->socketDescriptor()));

}

void MainWindow::readTcpData(){
    QTcpSocket *socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray DataBuffer;
    QDataStream socketstream(socket);

    socketstream.startTransaction();
    socketstream >> DataBuffer;

    if(socketstream.commitTransaction() == false){
        return;
    }

    QString receivedMessage = QString::fromUtf8(DataBuffer);

    qintptr clientDescriptor = socket->socketDescriptor();
    QString messageWithClientId = "Client " + QString::number(clientDescriptor) + ":\n" + receivedMessage + "\n";
    ui->textBrowser_TcpInbox->append(messageWithClientId);

}

void MainWindow::discardTcpSocket(){

    QTcpSocket *socket = reinterpret_cast<QTcpSocket*>(sender());

    int SocketIndex = TcpClientList.indexOf(socket);

    if(SocketIndex > -1){
        TcpClientList.removeAt(SocketIndex);
    }

    ui->comboBox_TcpClientList->clear();

    foreach(QTcpSocket *socketTemp, TcpClientList){
        ui->comboBox_TcpClientList->addItem(QString::number(socketTemp->socketDescriptor()));
    }



    socket->deleteLater();

}

void MainWindow::sendTcpMessage(){

    QString message = ui->textEdit_TcpMessage->toPlainText();

    if(ui->comboBox_TcpTransferType->currentText() == "Broadcast"){
        foreach(QTcpSocket *socketTemp, TcpClientList){
            if(socketTemp && socketTemp->isOpen()){
                QByteArray messageData = message.toUtf8();

                QDataStream socketStream(socketTemp);
                socketStream << messageData;
            }
        }
    }else if(ui->comboBox_TcpTransferType->currentText() == "Receiver"){
        QString receiverId = ui->comboBox_TcpClientList->currentText();
        foreach(QTcpSocket *socketTemp, TcpClientList){
            if(socketTemp->socketDescriptor() == receiverId.toLongLong()){
                if(socketTemp && socketTemp->isOpen()){
                    QByteArray messageData = "text:" + message.toUtf8();

                    QDataStream socketStream(socketTemp);
                    socketStream << messageData;
                }
            }
        }

    }

}

void MainWindow::readUdpData(){

    while(UDP_Server->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(UDP_Server->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;

        UDP_Server->readDatagram(datagram.data(), datagram.size(),&senderAddress, &senderPort);

        QString receivedMessage = QString::fromUtf8(datagram);
        QString senderInfo = "Sender address: " + senderAddress.toString() + "\tSender Port: " + QString::number(senderPort) + ":\n";
        ui->textBrowser_UDPInbox->append(senderInfo + receivedMessage + "\n");

    }

}

void MainWindow::sendUdpMessage(){

    if(UDP_Server){
        QString message = ui->textEdit_UdpMessage->toPlainText();
        QHostAddress udpHostAddress(IPAddress);

        QByteArray messageData;

        char firstByte = 0x41;
        messageData.append(firstByte);
        char secondByte = 0x42;
        messageData.append(secondByte);
        char thirdByte = 0x43;
        messageData.append(thirdByte);


        UDP_Server->writeDatagram(messageData, udpHostAddress, ReceiverUdpPort);
    }

}


