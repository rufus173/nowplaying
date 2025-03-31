#ifndef _MAIN_H
#define _MAIN_H

#include <QObject>
#include <QLabel>
#include <QGridLayout>
#include <QWindow>
#include <QProgressBar>
#include "media_players.h"

class MainWindow : public QWidget{
	Q_OBJECT
	private:
	MediaPlayers *players;
	QProgressBar *song_progress_bar;
	QGridLayout *grid;
	QLabel *song_info_label;
	
	public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
};

#endif
