#ifndef RESIMBUL_H
#define RESIMBUL_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_sexCombo_currentIndexChanged(const QString &arg1);

    void on_categoryCombo_currentIndexChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
};

#endif // RESIMBUL_H
