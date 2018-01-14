#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "theatreclass.h"
#include "playclass.h"
#include <QString>
#include <QtCore/qmath.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    int nTheatres;
    int nPlays;
    theatreClass * theatres;
    playClass * plays;
    int **schedule;
    const int dayStTime = 780;
    const int dayEnTime = 1500;
    const int unitTime = 15;
    const int numOfDays = 11;
    const int nLineOneDay = qCeil(1.0 * (dayEnTime - dayStTime) / unitTime);

    int maxNumOfPlay;
    int longestTime;
    int **solution;
    int * ifSeePlay;

    int hasSolution;

public:
    explicit MainWindow(QWidget *parent = 0);

    void input();
    void inputFromExcel();
    void arrange();
    void judge();
    void display();
    void saveToNewPlay();
    int findPlayNumber(QString);
    void findSolution(int, int, int);
    void showSolution();

    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_judgeButton_clicked();

    void on_getTheatrePlayInfoButton_clicked();

    void on_arrangeButton_clicked();

    void on_inputFromNewScheduleButton_clicked();

    void on_displayButton_clicked();

    void on_saveToNewPlayButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
