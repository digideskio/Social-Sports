#include "race.h"

Race::Race()
{

}

Race::Race(QString id, QString name, QString location, int distance, QDate date)
{
    this->_id = id;
    this->_name = name;
    this->_location = location;
    this->_distance = distance;
    this->_date = date;
}

QString Race::id(){
    return this->_id;
}

QString Race::name()
{
    return this->_name;
}

QString Race::location()
{
    return this->_location;
}

int Race::distance()
{
    return this->_distance;
}

QDate Race::date()
{
    return this->_date;
}

void Race::join(Group *group)
{
    this->enrollments.insert(group->name(), group);
}

void Race::leave(Group *group) {
    this->enrollments.remove(group->name());
}

bool Race::isEnrolled(Group *group) {
    return this->enrollments.contains(group->name());
}
