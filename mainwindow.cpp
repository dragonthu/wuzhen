#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QtCore/qmath.h>
//#include <QAxObject>

extern QString directoryOf();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//默认打开程序后，读入theatre和play的信息，安排进二维数组，写到schedule.txt里
    input();
    arrange();
    display();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::input() {
//输入剧场信息
    QString inFileName = directoryOf() +"/theatres.txt";

    QFile file(inFileName);
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "can't open " << inFileName;
        return;
    }
    QTextStream in(&file);
    in >> nTheatres;
    theatres = new theatreClass[nTheatres];

    for (int i = 0; i < nTheatres; i++) {
        in >> theatres[i].name;
        qDebug() << theatres[i].name;
    }

    file.close();

//输入剧目信息
    inFileName = directoryOf() + "/plays.txt";
    file.setFileName(inFileName);
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "can't open " << inFileName;
        return;
    }
    in.setDevice(&file);
    in >> nPlays;
    plays = new playClass[nPlays];
    for (int i = 0; i < nPlays; i++) {
        in >> plays[i].name;
        QString tempName;
        in >> tempName;
        for (int j = 0; j < nTheatres; j++) {
            if (QString::compare(theatres[j].name, tempName) == 0) {
                plays[i].theatre = j;
                break;
            }
        }
        in >> plays[i].duration >> plays[i].leastGap >> plays[i].times;
        plays[i].day = new int[plays[i].times];
        plays[i].stTime = new int[plays[i].times];
        for (int j = 0; j < plays[i].times; j++) {
            int tempD, tempH, tempM;
            in >> tempD >> tempH >> tempM;
            plays[i].day[j] = tempD - 19;
            plays[i].stTime[j] = tempH * 60 + tempM;
        }
        qDebug() << i << '\n' << plays[i].name << '\n' << plays[i].theatre;
    }
    file.close();
}
void MainWindow::arrange() {
//将读入的剧目时间信息放置入二维矩阵
    int nLine;
    nLine = nLineOneDay * numOfDays;
    schedule = new int * [nLine];
    for (int i = 0; i < nLine; i++) {
        schedule[i] = new int [nTheatres];
        for (int j = 0; j < nTheatres; j++)
            schedule[i][j] = -1;
    }

    for (int i = 0; i < nPlays; i++) {
        for (int j = 0; j < plays[i].times; j++) {
            int stLine, enLine;
            stLine = plays[i].day[j] * nLineOneDay +
                    (plays[i].stTime[j] - dayStTime) / unitTime;
            enLine = stLine + qCeil(1.0 * plays[i].duration / unitTime);
            for (int k = stLine; k < enLine; k++) {
                schedule[k][plays[i].theatre] = i;
            }
        }
    }

}
void MainWindow::judge() {
    qDebug() << "judge!";
//judge
    QString resultText;

    //每天的戏的数量
    int * numOfPlaysPerDay = new int[numOfDays];
    for (int i = 0; i < numOfDays; i++) numOfPlaysPerDay[i] = 0;
    for (int i = 0; i < nPlays; i++) {
        for (int j = 0; j < plays[i].times; j++) {
            numOfPlaysPerDay[plays[i].day[j]] ++;
        }
    }
    resultText += "每天剧目数量\n";
    for (int i = 0; i < numOfDays; i++) {
        resultText += "10月";
        resultText += QString::number(i+19, 10);
        resultText += "日\t";
        resultText += QString::number(numOfPlaysPerDay[i], 10);
        resultText += "\n";
    }
    ui->dayStatusBrowser->setText(resultText);
    //每部戏连续两场演出的实际间隔时间
    resultText = "";
    for (int i = 0; i < nPlays; i++) {
        int bigGap = numOfDays * 24 * 60;
        int minGap = bigGap, minDay = -1;
        for (int j = 1; j < plays[i].times; j++) {
            if (plays[i].day[j] == plays[i].day[j-1]) {
                int tempGap = plays[i].stTime[j] - plays[i].stTime[j-1] - plays[i].duration;
                if (minGap > tempGap) {
                    minGap = tempGap;
                    minDay = plays[i].day[j];
                }
            }
        }
        if (minGap < bigGap) {
            resultText += plays[i].name;
            resultText += "\t最小间隔 ";
            resultText += (QString::number(minGap / 60, 10) + "时" + QString::number(minGap % 60, 10) + "分 在" +
                           QString::number(minDay + 19) + "日\n");
        }
    }
    ui->gapResultBrowser->setText(resultText);

    //观众能看到的最多的戏以及方案
    resultText = "";
    maxNumOfPlay = 0;
    longestTime = 0;
    solution = new int * [nLineOneDay];
    for (int i = 0; i < nLineOneDay; i++) {
        solution[i] = new int[numOfDays];
        for (int j = 0; j < numOfDays; j++)
            solution[i][j] = -1;
    }
    ifSeePlay = new int[nPlays];
    for (int i = 0; i < nPlays; i++)
        ifSeePlay[i] = -1;
    //递归搜索，每得到一个解，就更新输出一下
    hasSolution = 0;
    findSolution(0, 0, 0);

// 输出judge的信息

}
void MainWindow::showSolution() {
    //显示解决方案
    qDebug() << "showSolution";
    hasSolution ++;
    QString resultText = "";
    resultText += ("当前最佳方案，共可看" + QString::number(maxNumOfPlay, 10)
            + "部戏，共计"+QString::number(longestTime,10) + "分钟\n");
    resultText += "还不能看：";
    for (int i = 0; i < nPlays; i++) {
        if (ifSeePlay[i] == -1) {
            resultText += plays[i].name + ' ';
        }
    }
    resultText += '\n';
    //在界面上显示每个戏的方案
    for (int i = 0; i < nPlays; i++) {
        if (ifSeePlay[i] > -1) {
            int t = ifSeePlay[i];
            resultText += (plays[i].name + " " +
                           QString::number(plays[i].day[t] + 19, 10) + "日 " +
                           QString::number(plays[i].stTime[t] / 60, 10) + "时" +
                           QString::number(plays[i].stTime[t] % 60, 10) + "分 到" +
                           QString::number((plays[i].stTime[t] + plays[i].duration) / 60, 10) + "时"+
                           QString::number((plays[i].stTime[t] + plays[i].duration) % 60, 10) + "分\t在" +
                           theatres[plays[i].theatre].name + "\n");
        }
    }
    qDebug() << resultText;
    ui->judgeResultBrowser->setText(resultText);
    //在文件中打印该人行程

    QString outFileName = directoryOf() +"/solution.txt";

    QFile file(outFileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
        qDebug() << "can't open " << outFileName;
        return;
    }
    QTextStream out(&file);
    out << "时间\t";
    for (int day = 0; day < numOfDays; day ++) {
        out << QString::number(day + 19, 10) + "日\t";
    }
    out << '\n';

    for (int n = 0; n < nLineOneDay; n ++) {
        out << (dayStTime + n * unitTime) / 60;
        out << ":";
        int min = (dayStTime + n * unitTime) % 60;
        if (min == 0) out << "00";
        else out << min;
        out << '\t';

        for (int day = 0; day < numOfDays; day ++) {
            if (solution[n][day] != -1) {
                out << plays[solution[n][day]].name << "\t";
            } else {
                out << "_\t";
            }
        }
        out << "\n";
    }
    file.close();

    //todo
}
void MainWindow::findSolution(int nowPlay, int nowNumOfPlay, int nowTime) {
    //递归找看最多戏的方法
//    qDebug() << "findSolution " << nowPlay << ' ' << nowNumOfPlay << '\n';

    //如果当前的方案加上后面所有都比不过，则不用再往下继续
    if ((nowNumOfPlay + nPlays - nowPlay) <= maxNumOfPlay)
        return ;

    if (nowPlay >= nPlays) {
        if (nowNumOfPlay > maxNumOfPlay) {
            maxNumOfPlay = nowNumOfPlay;
            longestTime = nowTime;
            showSolution();
        }
        return;
    }
    //遍历当前戏的每一场
    for (int i = 0; i < plays[nowPlay].times; i++) {
        //判断是否没有重叠
        int nowDay = plays[nowPlay].day[i];

        int stLine = (plays[nowPlay].stTime[i] - dayStTime) / unitTime;
        int enLine = stLine + qCeil(1.0 * plays[nowPlay].duration / unitTime);
        bool flag = true;

        for (int n = stLine - 2; n < enLine + 2; n++) {
            if (n < 0 || n >= nLineOneDay) {
                //qDebug() << "hahaha" << n << endl;
                flag = false;
                break;
            }
            if (solution[n][nowDay] != -1) {
                flag = false;
                break;
            }
        }

        if (flag) {
            //修改solution数组
            for (int n = stLine; n < enLine; n++) {
                if (n < 0 || n >= nLineOneDay) {
                    qDebug() << "hahaha!!!" << n << endl;
                    continue;
                }
                solution[n][nowDay] = nowPlay;
            }

            //进入下一层
            ifSeePlay[nowPlay] = i;
            findSolution(nowPlay + 1, nowNumOfPlay + 1, nowTime + plays[i].duration);
            //if (hasSolution > 3) return ;

            //恢复solution数组
            for (int n = stLine; n < enLine; n++) {
                solution[n][nowDay] = -1;
            }

        }
    }
    //不看当前戏，进入下一个
    ifSeePlay[nowPlay] = -1;
    findSolution(nowPlay + 1, nowNumOfPlay, nowTime);
    return ;
}
void MainWindow::display() {
// 输出到schedule里面，是详细的大表
//to do    输出成每日一行的大表。这个还没写
    QString outFileName = directoryOf() +"/schedule.txt";

    QFile file(outFileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
        qDebug() << "can't open " << outFileName;
        return;
    }
    QTextStream out(&file);

    for (int nDay = 0; nDay < numOfDays; nDay ++) {
        out << QString("日期");
        out << '\t';
        out << QString("时间");
        out << '\t';
        for (int j = 0; j < nTheatres; j++) {
            out << theatres[j].name << '\t';
        }
        out << '\n';
        for (int i = 0; i < nLineOneDay; i++) {
            out << nDay + 19 << QString("日");
            out << '\t';
            out << (dayStTime + i * unitTime) / 60;
            out << ":";
            int min = (dayStTime + i * unitTime) % 60;
            if (min == 0) out << "00";
            else out << min;
            out << '\t';

/*            if ((dayStTime + i * unitTime) % 60 == 0) {
                out << (dayStTime + i * unitTime) / 60;
                out << ":00";
                out << '\t';
            } else {
                out <<":\t";
            }
            */
            for (int j = 0; j < nTheatres; j++) {
                if (schedule[nDay * nLineOneDay + i][j] == -1) {
                    out << '_';
                } else {
                    out << plays[schedule[nDay * nLineOneDay + i][j]].name;
                }
                out << '\t';
            }
            out << '\n';
        }
    }
}
void MainWindow::inputFromExcel() {
//本来要读取excel，但是似乎免费版无法操作 QT += axcontainer；这个待考察，现在只能手动勤快点，把调整后的excel，全部拷贝，放到一个文本文件里
    //从newschedule.txt里读取
    QString inFileName = directoryOf() +"/newschedule.txt";

    QFile file(inFileName);
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        qDebug() << "can't open " << inFileName;
        return;
    }
    QTextStream in(&file);

    //把plays里面的部分信息清零，theatre变成-1，times变为0；
    for (int i = 0; i < nPlays; i++) {
        plays[i].times = 0;
        plays[i].theatre = -1;
    }

    for (int nDay = 0; nDay < numOfDays; nDay ++) {
        QString tempS;
        in >> tempS;
        qDebug() << tempS;
        in >> tempS;
        qDebug() << tempS;
        for (int j = 0; j < nTheatres; j++) {
            in >> tempS;
            qDebug() << tempS;
        }
        //以上为读表头无用信息

        for (int n = 0; n < nLineOneDay; n++) {
            in >> tempS;
            qDebug() << tempS;
                    in >> tempS;
                    qDebug() << tempS;
            for (int j = 0; j < nTheatres; j++) {
                in >> tempS;
                qDebug() << tempS;
                if (QString::compare(tempS,"_")!=0) {
                    int tempN = findPlayNumber(tempS);
                    if (tempN == -1 ){
                        qDebug() << "no such plays " << tempS << ' ' << nDay << ' ' << n << ' ' << j;
                        return ;
                    }
                    schedule[nDay * numOfDays +n][j] = tempN;
//                    qDebug() << tempN << '\n' << plays[tempN].name;
                } else {
                    schedule[nDay * numOfDays +n][j] = -1;
                }
            }
        }


        for (int n = 0; n < nLineOneDay; n++) {
            for (int j = 0; j < nTheatres; j++) {
                int tempN = schedule[nDay * numOfDays +n][j];
                if (tempN == -1) continue;
                if (plays[tempN].theatre == -1) {
                    plays[tempN].theatre = j;
                } else {
                    if (plays[tempN].theatre != j) {
                        qDebug() << "different theatres for one play " << plays[tempN].name
                                 << ' ' << plays[tempN].theatre << ' ' << j;
                        return ;
                    }
                }
                if (n == 0 || schedule[nDay * numOfDays + n - 1][j] == -1) {
                    int tempT = plays[tempN].times;
                    plays[tempN].times++;
                    plays[tempN].day[tempT] = nDay;
                    plays[tempN].stTime[tempT] = n * unitTime + dayStTime;
                }
            }
        }
    }

    file.close();
}
void MainWindow::on_pushButton_clicked()
{
}



void MainWindow::on_judgeButton_clicked()
{
    judge();
}

int MainWindow::findPlayNumber(QString tempS) {
    for (int i = 0; i < nPlays; i++) {
        if (QString::compare(plays[i].name,tempS)==0) {
            return i;
        }
    }
    return -1;

}
void MainWindow::saveToNewPlay() {
//把更新后的schedule以之前读取的方式存储到newplays.txt中
//to do，输出成每日一行的大表。这个还没写
    QString outFileName = directoryOf() +"/newplays.txt";

    QFile file(outFileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
        qDebug() << "can't open " << outFileName;
        return;
    }
    QTextStream out(&file);
    out << nPlays << '\n';
    for (int i = 0; i < nPlays; i ++) {
        out << plays[i].name << '\n';
        out << theatres[plays[i].theatre].name << '\n';
        out << QString("时长") << plays[i].duration << QString("分钟\t")
            << plays[i].leastGap << QString("\t共演") << plays[i].times << QString("场\n");
        for (int j = 0; j < plays[i].times; j++) {
            out << plays[i].day[j] + 19 << QString("日\t")
                << plays[i].stTime[j] / 60 << QString("时\t")
                << plays[i].stTime[j] % 60 << QString("分\n");
        }
    }
    file.close();
}

void MainWindow::on_getTheatrePlayInfoButton_clicked()
{
    qDebug() << "getTheatrePlayInfo";
    input();// 输入theatre和play的信息
}

void MainWindow::on_arrangeButton_clicked()
{
    qDebug() << "arrange";
    arrange();
}

void MainWindow::on_inputFromNewScheduleButton_clicked()
{
    qDebug() << "inputFromNewSchedule";
    inputFromExcel();
}

void MainWindow::on_displayButton_clicked()
{
    qDebug() << "display";
    display();
}

void MainWindow::on_saveToNewPlayButton_clicked()
{
    qDebug() << "saveToNewPlay";
    saveToNewPlay();
}
