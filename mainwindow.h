#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QPair>
#include <climits>
#include <QDebug>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;

    // History for both teams
    QStringList history;

    // Graph representation for teams
    QList<int> team1_edges;
    QList<int> team2_edges;

    // Dijkstra's Algorithm Helper Functions
    int findShortestPath(int team, int targetPoint);
    void updateScoreDisplay();
    int calculateTotalScore(const QList<int>& edges);

};

#endif // MAINWINDOW_H
