#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QMapIterator>

#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

const QString MainWindow::MSG_COMMAND_NOT_RECOGNIZED = QString("Your command was not recognized. Use 'HELP' command to get a list of available commands.");
const QString MainWindow::MSG_REGISTRATION_REQUIRED = QString("You have to register to the service before using this function.");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->buttonSend, SIGNAL(clicked()), SLOT(sendCommand()));
    connect(ui->lineEditCommand, SIGNAL(returnPressed()), SLOT(sendCommand()));
    connect(ui->btnSimulateInvitation, SIGNAL(clicked()), SLOT(simulateInvitation()));

    this->initRaces();
    this->initHelp();
    this->initGroupsAndUsers();

    this->theUser = 0;
}

void MainWindow::initGroupsAndUsers()
{
    Group *group = createGroup("NakedJoggers");
    createUser("Mark")->join(group);
    createUser("Jane")->join(group);
    createUser("Peter")->join(group);
    createUser("JamesJones");
    createUser("FastRunner");
    createUser("AnneWhite");
    createUser("LauraGreen");
    Race *nakukymppi = this->raceLookup["NAKU11"];
    nakukymppi->join(group);
}

User* MainWindow::createUser(QString username)
{
    User *user = new User(username);
    this->users.insert(username, user);
    return user;
}

Group* MainWindow::createGroup(QString name)
{
    Group *group = new Group(name);
    this->groups.insert(name, group);
    return group;
}

void MainWindow::initRaces()
{
    this->races.append(new Race("LVJL11", QString::fromUtf8("Länsiväyläjuoksu (long)"), "Espoo", 17400, QDate(2011, 4, 17)));
    this->races.append(new Race("LVJS11", QString::fromUtf8("Länsiväyläjuoksu (short)"), "Espoo", 6500, QDate(2011, 4, 17)));
    this->races.append(new Race("LVJW11", QString::fromUtf8("Länsiväyläjuoksu (walk)"), "Espoo", 6200, QDate(2011, 4, 17)));
    this->races.append(new Race("HCR11", "Helsinki City Run", "Helsinki", 21098, QDate(2011, 5, 7)));
    this->races.append(new Race("NK11", "Naisten kymppi", "Helsinki", 10000, QDate(2011, 5, 22)));
    this->races.append(new Race("NAKU11", "Nakukymppi", "Padasjoki", 10000, QDate(2011, 6, 17)));
    this->races.append(new Race("PNM11", "Paavo Nurmi Marathon", "Turku", 42195, QDate(2011, 7, 2)));
    this->races.append(new Race("HKM11", "Helsinki City Marathon", "Helsinki", 42195, QDate(2011, 8, 20)));

    foreach (Race *race, this->races) {
        this->raceLookup.insert(race->id(), race);
    }
}

void MainWindow::initHelp()
{
    this->help.insert("HELP", "HELP -- Returns a list of available commands.\nHELP <command> -- Returns help on the specified command.");
    this->help.insert("RACE LIST", "RACE LIST -- Returns a list of upcoming foot races, most recent first.");
    this->help.insert("RACE INFO", "RACE INFO <id> -- Returns detailed information about the race with the given id.");
    this->help.insert("RACE JOIN", "RACE JOIN <raceid> <groupname> -- Enroll a group to a race.");
    this->help.insert("RACE LEAVE", "RACE INFO <raceid> <groupname> -- Cancel the group enrollment to a race.");
    this->help.insert("RACE PRERANK", "RACE PRERANK <raceid> <groupname> -- Get group's current ranking among other groups participating to a race.");
    this->help.insert("REGISTER", "REGISTER <username> -- Register to the service with the given username.");
    this->help.insert("UNREGISTER", "UNREGISTER -- Unregister from the service.");
    this->help.insert("MY FITNESS", "MY FITNESS -- Returns your current personal fitness values and feedback about your training.");
    this->help.insert("MY INVITATIONS", "MY INVITATIONS -- Returns your pending group invitations.");
    this->help.insert("GROUP MEMBERS", "GROUP MEMBERS <group name> -- Returns a list of members in the given group.");
    this->help.insert("GROUP CREATE", "GROUP CREATE <group name> [<username>, ...] -- Creates a group with the given name, and sends invitations to the users given.");
    this->help.insert("GROUP FITNESS", "GROUP FITNESS <group name> -- Returns the average fitness values of a group.");
    this->help.insert("GROUP INVITE", "GROUP INVITE <group name> [<username>, ...] -- Invites users to a group with the given name.");
    this->help.insert("GROUP JOIN", "GROUP JOIN <group name> -- Join the group with the given name. You need an invitation to join the group.");
    this->help.insert("GROUP LEAVE", "GROUP LEAVE <group name> -- Leave the group with the given name.");
    this->help.insert("NEWS", "NEWS -- Returns a list of recent activity in your groups.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString MainWindow::doHelp(QString command)
{
    QString commandUpper = command.toUpper();
    if (command == "") {
        return "Available commands: " + QStringList(this->help.keys()).join(", ");
    } else if (this->help.contains(commandUpper)) {
        return this->help[commandUpper];
    } else {
        return "No help available on '" + command + "'.";
    }
}

QString MainWindow::doRaceList()
{
    QStringList response;
    foreach (Race *race, this->races) {
        response.append(QString("%1: %2").arg(race->id(), race->name()));
    }
    return response.join("\n");
}

QString MainWindow::doRaceInfo(QString id)
{
    QString idUpper = id.toUpper();
    if (this->raceLookup.contains(idUpper)) {
        Race *race = this->raceLookup[idUpper];
        return QString("%1 (%2) is %3 m long race organized in %4 on %5.").arg(
                race->name(), race->id(), QString::number(race->distance()),
                race->location(), race->date().toString("dd.MM.yyyy"));
    } else {
        return QString("There is no race with the id '%1'").arg(id);
    }
}

QString MainWindow::doRaceJoin(QString raceId, QString groupName)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    raceId = raceId.toUpper();
    if (!this->raceLookup.contains(raceId)) {
        return QString("There is no race with the id '%1'").arg(raceId);
    }
    Race *race = this->raceLookup[raceId];
    if (!this->groups.contains(groupName)) {
        return QString("The group '%0' does not exist.").arg(groupName);
    }
    Group *group = this->groups[groupName];
    if (!this->theUser->isMemberOf(group)) {
        return QString("You must be a member of a group in order to enroll the group to a race.");
    }
    if (race->isEnrolled(group)) {
        return QString("This group is already enrolled to this race.");
    }
    race->join(group);
    return QString("You have enrolled group '%0' to '%1'.").arg(groupName, race->name());
}

QString MainWindow::doRaceLeave(QString raceId, QString groupName)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    raceId = raceId.toUpper();
    if (!this->raceLookup.contains(raceId)) {
        return QString("There is no race with the id '%1'").arg(raceId);
    }
    Race *race = this->raceLookup[raceId];
    if (!this->groups.contains(groupName)) {
        return QString("The group '%0' does not exist.").arg(groupName);
    }
    Group *group = this->groups[groupName];
    if (!this->theUser->isMemberOf(group)) {
        return QString("You must be a member of a group in order to cancel an enrollment to a race.");
    }
    if (!race->isEnrolled(group)) {
        return QString("This group has not enrolled to this race.");
    }
    race->leave(group);
    return QString("You have cancelled enrollment of group '%0' to '%1'.").arg(groupName, race->name());
}

QString MainWindow::doRacePrerank(QString raceId, QString groupName)
{
    raceId = raceId.toUpper();
    if (!this->raceLookup.contains(raceId)) {
        return QString("There is no race with the id '%1'").arg(raceId);
    }
    Race *race = this->raceLookup[raceId];
    if (!this->groups.contains(groupName)) {
        return QString("The group '%0' does not exist.").arg(groupName);
    }
    Group *group = this->groups[groupName];
    if (!race->isEnrolled(group)) {
        return QString("This group has not enrolled to this race.");
    }
    QString response = "1. ABB_Runners\n"
                       "2. Tieto_Runners\n"
                       "3. HU_RunningTeam\n"
                       "4. TampereU_Run\n"
                       "...\n"
                       "31. Helsinki_Running_club\n"
                       "32. %0\n"
                       "33. YOUTH_Runners\n"
                       "...\n"
                       "Total number of groups: 230";
    return response.arg(groupName);
}

QString MainWindow::doRace(QStringList args)
{
    if (args.length() > 0) {
        QString command = args.takeFirst().toUpper();
        if (command == "LIST") {
            return this->doRaceList();
        } else if (command == "INFO") {
            if (args.length() == 1) {
                return this->doRaceInfo(args[0]);
            } else {
                return this->doHelp("RACE INFO");
            }
        } else if (command == "JOIN") {
            if (args.length() == 2) {
                return this->doRaceJoin(args[0], args[1]);
            } else {
                return doHelp("RACE JOIN");
            }
        } else if (command == "LEAVE") {
            if (args.length() == 2) {
                return this->doRaceLeave(args[0], args[1]);
            } else {
                return doHelp("RACE LEAVE");
            }
        } else if (command == "PRERANK") {
            if (args.length() == 2) {
                return this->doRacePrerank(args[0], args[1]);
            } else {
                return doHelp("RACE PRERANK");
            }
        }
    }
    return this->MSG_COMMAND_NOT_RECOGNIZED;
}

QString MainWindow::doGroupMembers(QString groupName)
{
    if (this->groups.contains(groupName)) {
        Group *group = this->groups[groupName];
        QString response = QString("Members of group '%0': ").arg(groupName);
        QStringList usernames;
        foreach (User *user, group->getMembers()) {
            usernames.append(user->username());
        }
        response += usernames.join(", ");
        return response;
    } else {
        return QString("There is no group called '%0'.").arg(groupName);
    }
}

QString MainWindow::doGroupCreate(QString groupName, QStringList usernames)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (this->groups.contains(groupName)) {
        return QString("Group name '%0' is already taken. Please choose another name.").arg(groupName);
    } else {
        QStringList usersNotFound;
        QStringList usersInvited;
        Group *group = this->createGroup(groupName);
        this->theUser->join(group);
        foreach (QString username, usernames) {
            if (this->users.contains(username)) {
                User *user = this->users[username];
                usersInvited.append(user->username());
                theUser->invite(user, group);
            } else {
                usersNotFound.append(username);
            }
        }
        QString response = QString("You created a group called '%0'.").arg(groupName);
        if (usersInvited.length() > 0) {
            response += "The following users were invited to join the group: ";
            response += usersInvited.join(", ");
            response += ". ";
        }
        if (usersNotFound.length() > 0) {
            response += "The following users were not found: ";
            response += usersNotFound.join(", ");
            response += ". ";
        }
        return response;
    }
}

QString MainWindow::doGroupInvite(QString groupName, QStringList usernames)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (!this->groups.contains(groupName)) {
        return QString("The group '%0' does not exist.").arg(groupName);
    } else {
        Group *group = this->groups[groupName];
        if (!this->theUser->isMemberOf(group)) {
            return QString("You must be a member of the group in order to invite people to join it.");
        }
        QStringList usersNotFound;
        QStringList usersInvited;
        QStringList usersAlreadyMember;
        QStringList usersAlreadyInvited;
        foreach (QString username, usernames) {
            if (this->users.contains(username)) {
                User *user = this->users[username];
                if (user->isMemberOf(group)) {
                    usersAlreadyMember.append(username);
                } else if (user->hasInvitation(group)) {
                    usersAlreadyInvited.append(username);
                } else {
                    usersInvited.append(user->username());
                    theUser->invite(user, group);
                }
            } else {
                usersNotFound.append(username);
            }
        }
        QString response;
        if (usersInvited.length() > 0) {
            response += "You have invited the following users to group " + groupName + ": ";
            response += usersInvited.join(", ");
            response += ". ";
        }
        if (usersNotFound.length() > 0) {
            response += "The following users were not found: ";
            response += usersNotFound.join(", ");
            response += ". ";
        }
        if (usersAlreadyMember.length() > 0) {
            response += "The following users were already member of the group: ";
            response += usersAlreadyMember.join(", ");
            response += ". ";
        }
        if (usersAlreadyInvited.length() > 0) {
            response += "The following users were already invited to the group: ";
            response += usersAlreadyInvited.join(", ");
            response += ". ";
        }
        return response;
    }
}

QString MainWindow::doGroupJoin(QString groupName)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (!this->groups.contains(groupName))
    {
        return QString("There is no group called '%0'.").arg(groupName);
    }
    Group *group = this->groups[groupName];
    if (!this->theUser->hasInvitation(group))
    {
        return QString("You need an invitation to join this group.");
    }
    this->theUser->acceptInvitation(group);
    return QString("You have joined the group called '%0'.").arg(groupName);
}

QString MainWindow::doGroupLeave(QString groupName)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (!this->groups.contains(groupName))
    {
        return QString("There is no group called '%0'.").arg(groupName);
    }
    Group *group = this->groups[groupName];
    if (!this->theUser->isMemberOf(group))
    {
        return QString("You are not a member of this group.");
    }
    this->theUser->leave(group);
    return QString("You have left the group called '%0'.").arg(groupName);
}

QString MainWindow::doNews()
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    return "TimoAalto ran 11km in 45 minutes yesterday.\nLauraGreen joined TKK_Runners.";
}

QString MainWindow::doGroupFitness(QString groupName)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (!this->groups.contains(groupName))
    {
        return QString("There is no group called '%0'.").arg(groupName);
    }
    QString response = QString("Group %0 has average fitness levels of endurance 55, strength 70 and flexibility 83. ").arg(groupName);
    Group *group = this->groups[groupName];
    if (this->theUser->isMemberOf(group))
    {
        response += "TimoAalto has reached his all time high in flexibility level, 90.";
    }
    return response;
}

QString MainWindow::doGroup(QStringList args)
{
    if (args.length() > 0) {
        QString command = args.takeFirst().toUpper();
        if (command == "MEMBERS") {
            return this->doGroupMembers(args.join(" "));
        } else if (command == "CREATE") {
            if (args.length() < 1) {
                return this->doHelp("GROUP CREATE");
            }
            QString groupName = args.takeFirst();
            return this->doGroupCreate(groupName, args);
        } else if (command == "INVITE") {
            if (args.length() < 1) {
                return this->doHelp("GROUP INVITE");
            }
            QString groupName = args.takeFirst();
            return this->doGroupInvite(groupName, args);
        } else if (command == "JOIN") {
            if (args.length() != 1) {
                return this->doHelp("GROUP JOIN");
            }
            return this->doGroupJoin(args[0]);
        } else if (command == "LEAVE") {
            if (args.length() != 1) {
                return this->doHelp("GROUP LEAVE");
            }
            return this->doGroupLeave(args[0]);
        } else if (command == "FITNESS") {
            if (args.length() != 1) {
                return this->doHelp("GROUP FITNESS");
            }
            return this->doGroupFitness(args[0]);
        }
    }
    return this->MSG_COMMAND_NOT_RECOGNIZED;
}

QString MainWindow::doRegister(QString username)
{
    if (this->theUser) {
        return QString("You have already registered to the service with username '%0'.").arg(this->theUser->username());
    } else {
        if (this->users.contains(username)) {
            return QString("Registration failed. The username '%0' is already taken. Please try some other username.").arg(username);
        }
        this->theUser = new User(username);
        this->users.insert(username, this->theUser);
        this->ui->btnSimulateInvitation->setEnabled(true);
        return QString("You have succesfully registered to the service with username '%0'.").arg(username);
    }
}

QString MainWindow::doUnregister()
{
    if (this->theUser) {
        this->users.remove(this->theUser->username());
        delete this->theUser;
        this->theUser = 0;
        this->ui->btnSimulateInvitation->setEnabled(false);
        return QString("You have succesfully unregistered from the service.");
    } else {
        return QString("Cannot unregister you from the service, because you have not registered to the service.");
    }
}

void MainWindow::simulateInvitation() {
    User *mark = this->users["Mark"];
    Group *nakedJoggers = this->groups["NakedJoggers"];
    mark->invite(this->theUser, nakedJoggers);
    this->receiveMessage("Mark has invited you to join a group called 'NakedJoggers'. Send 'GROUP JOIN NakedJoggers' to accept this invitation.");
    this->ui->btnSimulateInvitation->setEnabled(false);
}

QString MainWindow::doMy(QStringList args)
{
    if (!this->theUser) {
        return MSG_REGISTRATION_REQUIRED;
    }
    if (args.length() > 0) {
        QString command = args.takeFirst().toUpper();
        if (command == "FITNESS") {
            return this->doMyFitness();
        } else if (command == "INVITATIONS") {
            return this->doMyInvitations();
        }
    }
    return this->MSG_COMMAND_NOT_RECOGNIZED;
}

QString MainWindow::doMyInvitations()
{
    QString response;
    QList<QString> groupNames = this->theUser->getInvitations();
    if (groupNames.length() == 0) {
        return "You do not have any pending group invitations at the moment.";
    }
    response = "You have been invited to the following groups: ";
    QStringList invitations;
    foreach (QString groupName, groupNames) {
        Group *group = this->groups[groupName];
        User *inviter = this->theUser->getInviter(group);
        invitations.append(QString("%0 by %1").arg(groupName, inviter->username()));
    }
    response += invitations.join(", ");
    return response;
}

QString MainWindow::doMyFitness()
{
    return "Your endurance is 70, strength 89 and flexibility 90. Your flexibility level has reached its all time high. Try to improve your endurance by running longer distances.";
}

void MainWindow::receiveMessage(QString message)
{
    this->ui->textEditResponse->setPlainText(message);
}

void MainWindow::sendCommand()
{
    QString command = this->ui->lineEditCommand->text();
    this->ui->lineEditCommand->clear();

    QStringList args = command.trimmed().split(QRegExp("\\s+"));
    command = args.takeFirst().toUpper();

    QString response;
    if (command == "HELP" || command == "HLP" || command == "H") {
        response = doHelp(args.join(" "));
    } else if (command == "RACE" || command == "RC") {
        response = doRace(args);
    } else if (command == "GROUP" || command == "GRP") {
        response = doGroup(args);
    } else if (command == "REGISTER" || command == "RGSTR" || command == "RGS") {
        if (args.length() == 1) {
            response = doRegister(args[0]);
        } else {
            response = doHelp("REGISTER");
        }
    } else if (command == "UNREGISTER" || command == "NRGSTR" || command ="UNRGS") {
        if (args.length() == 0) {
            response = doUnregister();
        } else {
            response = doHelp("UNREGISTER");
        }
    } else if (command == "MY" || command == "M") {
        response = doMy(args);
    } else if (command == "NEWS" || command == "NWS") {
        response = doNews();
    } else {
        response = this->MSG_COMMAND_NOT_RECOGNIZED;
    }

    this->receiveMessage(response);
}