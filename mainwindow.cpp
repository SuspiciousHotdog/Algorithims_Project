#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QVector>
#include <QPair>
#include <queue>
#include <climits>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect the pushButton's click signal to the appropriate slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::on_pushButton_2_clicked);

    // Initialize history for both teams
    history = QStringList(2, "");  // Two empty history slots for two teams
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Slot to handle adding points and updating history
void MainWindow::on_pushButton_clicked()
{
    // Get the input point from the line edit
    bool ok;
    int point = ui->points_input->text().toInt(&ok);

    if (ok) {
        // Add the point to the selected team's graph
        int selectedTeam = ui->Team_selection->currentIndex();

        qDebug() << "Team selected:" << selectedTeam << "Point to add:" << point;

        if (selectedTeam == 0) {  // Team 1
            team1_edges.append(point);
        } else if (selectedTeam == 1) {  // Team 2
            team2_edges.append(point);
        }

        // Add the point to the history
        QString historyText = history[selectedTeam];
        historyText += QString::number(point) + ", ";

        // Update the history
        history[selectedTeam] = historyText;

        // Update the score display
        updateScoreDisplay();

        // Clear the input field after adding the point
        ui->points_input->clear();
    } else {
        // If the input is invalid, ignore it or handle gracefully
        // (We don't want to show "Invalid Input" here since it's for adding points)
    }
}

// Method to find the shortest path using Dijkstra's Algorithm
int MainWindow::findShortestPath(int team, int targetPoint)
{
    QVector<QVector<int>> graph;

    // Determine number of nodes based on the team selected
    int numNodes = 0;
    if (team == 0) {
        numNodes = team1_edges.size();  // Number of nodes in Team 1's graph
    } else if (team == 1) {
        numNodes = team2_edges.size();  // Number of nodes in Team 2's graph
    }

    // Find the target point index in the list of points for the selected team
    int targetIndex = -1;
    if (team == 0) {
        targetIndex = team1_edges.indexOf(targetPoint);
    } else if (team == 1) {
        targetIndex = team2_edges.indexOf(targetPoint);
    }

    if (targetIndex == -1) {
        qDebug() << "Target point" << targetPoint << "not found in the graph.";
        return -1;  // Target point not found
    }

    // Resize graph to accommodate the number of nodes
    graph.resize(numNodes);

    // Build the graph based on the edge list
    if (team == 0) {
        // Team 1 graph construction
        for (int i = 0; i < numNodes; ++i) {
            for (int j = i + 1; j < numNodes; ++j) {
                // Calculate the weight (difference between points) for the edge
                int weight = abs(team1_edges[i] - team1_edges[j]);
                graph[i].append(weight);
                graph[j].append(weight);  // Undirected graph: Add edges in both directions
            }
        }
    } else if (team == 1) {
        // Team 2 graph construction
        for (int i = 0; i < numNodes; ++i) {
            for (int j = i + 1; j < numNodes; ++j) {
                int weight = abs(team2_edges[i] - team2_edges[j]);
                graph[i].append(weight);
                graph[j].append(weight);  // Undirected graph: Add edges in both directions
            }
        }
    }

    // Debugging output to verify the graph structure
    qDebug() << "Graph for Team" << team << ":";
    for (const auto& row : graph) {
        qDebug() << row;
    }

    // Dijkstra's algorithm using a priority queue (min-heap)
    QVector<int> dist(numNodes, INT_MAX);  // Distance array initialized to infinity
    QVector<int> prev(numNodes, -1);  // Array to store the previous node in the path
    QVector<bool> visited(numNodes, false);  // Visited array
    dist[0] = 0;  // Starting from node 0

    // Priority Queue to find the next node with the smallest distance
    std::priority_queue<QPair<int, int>, std::vector<QPair<int, int>>, std::greater<QPair<int, int>>> pq;
    pq.push(qMakePair(0, 0));  // {distance, node}

    // Main loop of Dijkstra's algorithm
    while (!pq.empty()) {
        int u = pq.top().second;
        int d = pq.top().first;
        pq.pop();

        if (visited[u]) continue;

        visited[u] = true;

        // Relaxation of edges
        for (int v = 0; v < graph[u].size(); ++v) {
            if (!visited[v] && graph[u][v] != INT_MAX) {
                int newDist = d + graph[u][v];
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                    pq.push(qMakePair(dist[v], v));
                }
            }
        }
    }

    // Reconstruct the path from the target node
    QVector<int> path;
    for (int at = targetIndex; at != -1; at = prev[at]) {
        path.prepend(at);  // Prepend to reconstruct the path in the correct order
    }

    // Check if the path is unreachable
    if (path.isEmpty() || dist[targetIndex] == INT_MAX) {
        qDebug() << "Target point" << targetPoint << "is unreachable.";
        return -1;  // Target is unreachable
    }

    // Display the path and distance
    QString pathStr = "Path: ";
    for (int node : path) {
        pathStr += QString::number(node) + " ";
    }
    qDebug() << pathStr;

    // Return the shortest distance to the target point
    return (dist[targetIndex] == INT_MAX) ? -1 : dist[targetIndex];
}

// Slot for pushButton_2 to find the shortest path
void MainWindow::on_pushButton_2_clicked()
{
    // If the button is already disabled, return early to prevent double processing
    if (!ui->pushButton_2->isEnabled()) {
        qDebug() << "Button click ignored: already disabled.";
        return;
    }

    // Disable the button to prevent multiple clicks during processing
    ui->pushButton_2->setEnabled(false);

    // Get the target point from the second input box
    bool ok;
    int targetPoint = ui->points_input_2->text().toInt(&ok);

    if (ok) {
        // Find the shortest path to the target point
        int shortestPath = findShortestPath(0, targetPoint); // Assume team 0 for now

        if (shortestPath != -1) {
            // Display the result in the text browser
            ui->score_display->append(QString("Shortest path to point %1: %2").arg(targetPoint).arg(shortestPath));
        } else {
            ui->score_display->append(QString("Target point %1 is unreachable.").arg(targetPoint));
        }
    } else {
        ui->score_display->append("Please enter a valid number.");
    }

    // Add a slight delay before re-enabling the button to stop button from activating twice
    QTimer::singleShot(500, [this]() {
        ui->pushButton_2->setEnabled(true);
    });
}

void MainWindow::updateScoreDisplay()
{
    // Start with a container for the display
    QString text = "<div style='font-family: Arial, sans-serif; padding: 10px; background-color: #f4f4f4; border-radius: 16px;'>";

    text += "<h3 style='color: #2d6a4f;'>Team 1</h3>";  // Team 1 heading
    text += "<p style='font-size: 16px; color: #333;'>Score: <b>" + QString::number(calculateTotalScore(team1_edges)) + "</b></p>";
    text += "<p style='font-size: 14px; color: #555;'>History: " + history[0] + "</p>";

    text += "<hr style='border: 1px solid #d1d1d1;'>";  // Divider

    text += "<h3 style='color: #e85d04;'>Team 2</h3>";  // Team 2 heading
    text += "<p style='font-size: 16px; color: #333;'>Score: <b>" + QString::number(calculateTotalScore(team2_edges)) + "</b></p>";
    text += "<p style='font-size: 14px; color: #555;'>History: " + history[1] + "</p>";

    text += "<hr style='border: 1px solid #d1d1d1;'>";  // Divider
    // Close the container div
    text += "</div>";

    // Set the formatted HTML to the score display widget
    ui->score_display->setHtml(text);
}

/*
void MainWindow::updateScoreDisplay()
{
    QString text = "<b>Team 1:</b> " + QString::number(calculateTotalScore(team1_edges)) + "<br>";
    text += "<b>History: </b>" + history[0] + "<br>";

    text += "<b>Team 2:</b> " + QString::number(calculateTotalScore(team2_edges)) + "<br>";
    text += "<b>History: </b>" + history[1] + "<br>";

    ui->score_display->setHtml(text);
}*/

int MainWindow::calculateTotalScore(const QList<int>& edges)
{
    int totalScore = 0;
    for (int score : edges) {
        totalScore += score;
    }
    return totalScore;
}


