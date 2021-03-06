#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include "clientid.h"
#include "Spotify.h"
#include "playlist.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QMediaPlaylist>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , spotify()
    , playlistManager()

{
    ui->setupUi(this);
    spotify.login();
    playlistManager.loadPlaylists();
    player = new QMediaPlayer(this);
    mediaPlaylist = new QMediaPlaylist(this);

}


MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::refreshAuxListWidget()
{
    // Mostra as playlists já atualizadas na tela auxiliar
    ui->auxListWidget->clear();
    // Inicializa as playlists na tela auxiliar
    if(playlistManager.getAllPlaylists().isEmpty())
    {
        QString playlistEmpty = "No playlists to show";
        ui->auxListWidget->addItem(playlistEmpty);
    }
    else
    {
        ui->auxListWidget->addItems(playlistManager.getAllPlaylistsNames());
    }
}

void MainWindow::showTracksOnScreen(QList<track> trackList)
{
    ui->listWidget->clear();
    // Primeiro, verifica se há alguma track na lista
    if(trackList.isEmpty())
    {
        QString playlistEmpty = "Your playlist is empty";
        ui->listWidget->addItem(playlistEmpty);
    }
    else
    {
        // Varrer a lista pra exibir seus itens
        int i=0;
        for(i=0; i<trackList.size();i++)
        {
            QString trackname = trackList[i].getTrackName();
            QString trackalbum = trackList[i].getTrackAlbum();
            QString trackartist = trackList[i].getTrackArtist();
            //qDebug() << trackname;
            QString listItem = trackname + " || " + trackartist + " || " + trackalbum;
            ui->listWidget->addItem(listItem);
        }
    }
}


void MainWindow::on_iniciarButton_clicked()
{
    refreshAuxListWidget();
}


void MainWindow::on_searchButton_clicked()
{
    // Lê o que o user escreveu para realizar a busca
    QString searchInput;
    searchInput = ui->teOutput->toPlainText();

    if(!searchInput.isEmpty())
    {
        spotify.searchTrack(searchInput);
        ui->showResultsButton->setEnabled(true);
    }
}


void MainWindow::on_showResultsButton_clicked()
{
    ui->addTrackButton->setEnabled(true);
    ui->removeTrackButton->setDisabled(true);
    ui->previewTrackButton->setEnabled(true);
    // Primeiro, limpa a tela teOutput
    ui->teOutput->clear();
    // Vamos mostrar o resultado da pesquisa na tela listWidget
    QList<track> results = spotify.getSearch();
    showTracksOnScreen(results);
}


void MainWindow::on_renamePlaylistButton_clicked()
{
    // Verifica se há algum item selecionado
    if(ui->auxListWidget->currentRow()>=0)
    {
        // Usa um InputDialog para obter o novo nome da playlist
        QString playlistName = QInputDialog::getText(this, "Rename playlist", "Enter new playlist name");
        if(!playlistName.isEmpty())
        {
            // Recebe a row da playlist atualmente selecionada
            int selectedPlaylist = ui->auxListWidget->currentRow();
            playlistManager.renamePlaylist(playlistName, selectedPlaylist);
            refreshAuxListWidget();
        }
    }
}


void MainWindow::on_newPlaylistButton_clicked()
{
    // Cria uma nova playlist e exibe na tela auxiliar

    // Usa um InputDialog para obter o nome da nova playlist
    QString playlistName;
    playlistName = QInputDialog::getText(this, "New playlist", "Enter new playlist name");

    if(!playlistName.isEmpty())
    {
        // Cria o novo objeto playlist com o nome
        playlist playlist;
        playlist.setPlaylistName(playlistName);
        // Insere a nova playlist no container de playlists
        playlistManager.appendPlaylist(playlist);
        refreshAuxListWidget();
    }
}

void MainWindow::on_auxListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->addTrackButton->setDisabled(true);
    ui->removeTrackButton->setEnabled(true);
    // Double click na playlist para exibir suas tracks na tela principal
    int row = ui->auxListWidget->row(item);
    playlist playlist = playlistManager.getAllPlaylists()[row];
    showTracksOnScreen(playlist.getPlaylistTracks());
}

void MainWindow::on_addTrackButton_clicked()
{
    /*  A track selecionada será adicionada na playlist selecionada
        Primeiro, confere se há alguma playlist existente
        Se não houver, já inicia a criação da primeira
        Se houver, escolhe a partir de uma lista */

    // Verifica se há algum item selecionado
    if(ui->listWidget->currentRow()>=0)
    {
        // Primeiro, obtemos a row selecionada na tela principal listWidget
        int row = ui->listWidget->currentRow();

        // Criamos o objeto track com as informações selecionadas
        track selectedTrack = spotify.getSearch()[row];

        if(playlistManager.getAllPlaylists().isEmpty())
        {
            MainWindow::on_newPlaylistButton_clicked();
            playlistManager.addTrackToPlaylist(selectedTrack, playlistManager.getAllPlaylistsNames().first());
        }
        else
        {
            // Abre uma lista para escolher em qual playlist adicionar a track
            QString selectedPlaylist;
            selectedPlaylist = QInputDialog::getItem(this, "Select playlist", "Choose a playlist to add track",
                                                     playlistManager.getAllPlaylistsNames());

            playlistManager.addTrackToPlaylist(selectedTrack, selectedPlaylist);
        }
        refreshAuxListWidget();
    }
}

void MainWindow::on_removeTrackButton_clicked()
{
    // A track selecionada será removida da playlist atual

    // Verifica se há algum item selecionado
    if(ui->listWidget->currentRow()>=0)
    {
        // Primeiro, obtemos a row selecionada na tela principal listWidget
        int row = ui->listWidget->currentRow();
        // Recebe a row da playlist atualmente selecionada
        int selectedPlaylist = ui->auxListWidget->currentRow();
        // Faz a remoção
        playlistManager.removeTrackFromPlaylist(row, selectedPlaylist);
        // Vamos atualizar a tela para mostrar a playlist já atualizada
        ui->listWidget->clear();
        playlist playlist = playlistManager.getAllPlaylists()[selectedPlaylist];
        showTracksOnScreen(playlist.getPlaylistTracks());
    }
}

void MainWindow::on_deletePlaylistButton_clicked()
{
    if(ui->auxListWidget->currentRow()>=0)
    {
        // Primeiro, vamos confirmar se realmente deseja excluir
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Delete playlist", "Are you sure?", QMessageBox::Yes | QMessageBox::No);
        // Se sim, deleta a playlist
        if(reply == QMessageBox::Yes)
        {
            // Recebe a row da playlist atualmente selecionada
            int selectedPlaylist = ui->auxListWidget->currentRow();
            playlistManager.deletePlaylist(selectedPlaylist);
            refreshAuxListWidget();
            ui->listWidget->clear();
        }
    }
}

void MainWindow::on_savePlaylistButton_clicked()
{
    if(ui->auxListWidget->currentRow()>=0)
    {
        int row = ui->auxListWidget->currentRow();
        playlistManager.savePlaylist(playlistManager.getAllPlaylists()[row]);
    }
}

void MainWindow::on_playButton_clicked()
{
    if(ui->auxListWidget->currentRow()>=0)
    {
        int row = ui->auxListWidget->currentRow();

        playlist playlist = playlistManager.getAllPlaylists()[row];
        // Loop para carregar as tracks na mediaPlaylist
        int i=0;
        for(i=0; i<playlist.getPlaylistTracks().size(); i++)
        {
            if(!playlist.getPlaylistTracks()[i].getTrackPreview().isEmpty())
            {
                mediaPlaylist->addMedia(QUrl(playlist.getPlaylistTracks()[i].getTrackPreview()));
            }
        }
        // Configura a mediaPlaylist e toca
        mediaPlaylist->setPlaybackMode(QMediaPlaylist::Sequential);
        mediaPlaylist->setCurrentIndex(0);
        player->setPlaylist(mediaPlaylist);
        player->play();
    }
}


void MainWindow::on_previewTrackButton_clicked()
{
    // Testar a track para decidir se quer add na playlist
    if(ui->listWidget->currentRow()>=0)
    {
        int row = ui->listWidget->currentRow();
        QString trackUrl = spotify.getSearch()[row].getTrackPreview();
        player->setMedia(QUrl(trackUrl));
        player->play();
    }
}

void MainWindow::on_pauseButton_clicked()
{
    player->pause();
    ui->resumeButton->setEnabled(true);
}

void MainWindow::on_stopButton_clicked()
{
    player->stop();
    mediaPlaylist->clear();
    ui->resumeButton->setDisabled(true);
}

void MainWindow::on_resumeButton_clicked()
{
    player->play();
}
