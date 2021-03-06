#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include "track.h"
#include "playlist.h"
#include <QMediaPlayer>
#include <QMediaPlaylist>

QT_BEGIN_NAMESPACE
namespace Ui { class playlistManager; }
QT_END_NAMESPACE


class playlistManager
{

public:

    void appendPlaylist(playlist);
    void addTrackToPlaylist(track, QString);
    void removeTrackFromPlaylist(int, int);
    void renamePlaylist(QString, int);
    void deletePlaylist(int);
    QStringList getAllPlaylistsNames();
    QList<playlist> getAllPlaylists();
    void savePlaylist(playlist);
    void loadPlaylists();
    void playPlaylist(playlist);

private slots:

private:
    QList<playlist> allPlaylists;

};

#endif // PLAYLISTMANAGER_H
