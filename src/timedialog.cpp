#include "timedialog.h"
#include "ui_timedialog.h"

#include <QDate>
#include <QTime>

TimeDialog::TimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeDialog)
{
    ui->setupUi(this);

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();

    ui->hourBox->setValue(time.hour());
    ui->minuteBox->setValue(time.minute());

    ui->yearBox->setValue(date.year());
    ui->monthBox->setValue(date.month());
    ui->dayBox->setValue(date.day());


}

TimeDialog::~TimeDialog()
{
    delete ui;
}

DateTime TimeDialog::getDateTime()
{
    DateTime dt;

    dt.year = ui->yearBox->value();
    dt.month = ui->monthBox->value();
    dt.day = ui->dayBox->value();

    dt.hour = ui->hourBox->value();
    dt.minute = ui->minuteBox->value();

    return dt;
}
