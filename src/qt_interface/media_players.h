#ifndef _MEDIA_PLAYERS_H
#define _MEDIA_PLAYERS_H

#include <list>
#include <QDBusConnection>
#include <QString>
#include <QDBusInterface>
#include <mutex>
#include <QObject>
#include <QDebug>

struct Track {

};
class Player {
private:
	QString address;
	QDBusInterface *properties_interface;
public:
	Player(QString address);
	~Player();
	QString name();
};

class MediaPlayers : public QObject{
	Q_OBJECT
public:
	MediaPlayers();
	~MediaPlayers();
	Track *get_current_track();
public slots:
	void dbus_clients_change(QString name, QString new_owner, QString old_owner);

private:
	std::list<Player *> players = {};
	std::mutex players_mutex;
};

#endif
