#ifndef _MEDIA_PLAYERS_H
#define _MEDIA_PLAYERS_H

#include <list>
#include <QDBusConnection>
#include <QString>
#include <QDBusInterface>

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

class MediaPlayers {
private:
	std::list<Player *> players = {};
public:
	MediaPlayers();
	~MediaPlayers();
	Track *get_current_track();
};

#endif
