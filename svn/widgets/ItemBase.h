#ifndef ITEMBASE_H
#define ITEMBASE_H







#include <QFrame>









class ItemBase: public QFrame{


    Q_OBJECT


private:

    int dayweek;
    int hour, minute, second;

    QString filePath;
    double filesecond;


public:


    explicit ItemBase(QWidget *parent = 0);
    ~ItemBase();

    void setdayweek(int dayweek);
    void sethour(int hour);
    void setminute(int minute);
    void setsecond(int second);

    void setfilePath(QString filePath);
    void setfilesecond(double filesecond);



    int getdayweek() const;;
    int gethour() const;;
    int getminute() const;;
    int getsecond() const;;

    QString getfilePath() const;
    double getfilesecond()const ;

virtual ItemBase* copy() const = 0; // Método virtual puro



protected:

    void mousePressEvent(QMouseEvent *event);


private slots:

public slots:



};





#endif // ITEMBASE_H
