#ifndef _MEDIA_PLAYERS_H
#define _MEDIA_PLAYERS_H

#include <list>
#include <QDBusConnection>
#include <QString>
#include <QDBusInterface>
#include <mutex>
#include <QObject>
#include <QDebug>
#include <QVariantMap>

struct Track {
	enum {STATUS_PLAYING,STATUS_PAUSED,STATUS_STOPPED} playback_status;

};
class Player : public QObject{
	Q_OBJECT
private:
	std::mutex attributes_mutex;
	QString address;
	QVariantMap player_properties;
public:
	Player(QString address);
	~Player();
	QString name();
public slots:
	void dbus_properties_changed(QString, QVariantMap, QStringList);
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
