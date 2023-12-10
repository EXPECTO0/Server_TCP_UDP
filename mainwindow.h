#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QByteArray>
#include <QDataStream>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newTcpConnection();
    void AddToTcpSocketList(QTcpSocket *socket);
    void readTcpData();
    void discardTcpSocket();
    void sendTcpMessage();

    void readUdpData();
    void sendUdpMessage();

private:
    Ui::MainWindow *ui;

    QString IPAddress = "192.168.10.246";

    quint16 TcpPort = 44000;
    QTcpServer *TCP_Server;
    QList<QTcpSocket*> TcpClientList;

    QUdpSocket *UDP_Server;
    quint16 UdpPort = 44111;
    quint16 ReceiverUdpPort = 44222;

    QTimer* timer;
    int UdpSendMessageFrequency = 2000; //2 secs

};

#endif // MAINWINDOW_H
