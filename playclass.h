#ifndef PLAYCLASS_H
#define PLAYCLASS_H
#include <QString>


class playClass
{
public:
    playClass();
public:
    QString name;
    int theatre;
    int duration;
    int leastGap;
    int times;
    int * day;
    int * stTime;
};

#endif // PLAYCLASS_H
