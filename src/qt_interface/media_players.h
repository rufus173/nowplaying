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
};

class MediaPlayers : public QObject{
	Q_OBJECT
public:
	MediaPlayers();
	~MediaPlayers();
	Track *get_current_track();
	void epic(){
		qDebug() << "You shouldnt be here";
	}

public slots:
	void dbus_clients_change();

private:
	std::list<Player *> players = {};
	std::mutex players_mutex;
};

#endif
