#ifndef _MAIN_H
#define _MAIN_H

#include <QObject>
#include <QLabel>
#include <QGridLayout>
#include <QWindow>
#include <QProgressBar>
#include <QTimer>
#include "media_players.h"

class MainWindow : public QWidget{
	Q_OBJECT
	private:
	MediaPlayers *players;
	QProgressBar *song_progress_bar;
	QGridLayout *grid;
	QLabel *song_info_label;
	QLabel *song_remaining_time_label;
	QTimer *update_loop_timer;
	
	public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	public slots:
	void update_ui();
};

#endif
