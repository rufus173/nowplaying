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

class Player : public QObject{
	Q_OBJECT
private:
	std::recursive_mutex attributes_mutex;
	QString address;
	QVariantMap player_properties;
	long long int current_track_position = 0;
public:
	Player(QString address);
	~Player();
	QString name();
	QVariantMap *properties();
	int64_t get_current_position();
public slots:
	void dbus_properties_changed(QString, QVariantMap, QStringList);
signals:
	void started_playing(QString name);
};

class MediaPlayers : public QObject{
	Q_OBJECT
public:
	MediaPlayers();
	~MediaPlayers();
	int get_current_track_position();
	int get_current_track_length();
	QString get_current_track_name();
	QString get_current_track_artist();
public slots:
	void dbus_clients_change(QString name, QString new_owner, QString old_owner);
	void move_player_to_front(QString name);
private:
	std::list<Player *> players = {};
	std::mutex players_mutex;
};

#endif
