#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// 首先包含 QApplication 的头文件
#include <QApplication>
#include <QMainWindow>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QList>
#include <QPair>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QDir>
#include <QMessageBox>
#include <QIntValidator>
#include <QComboBox>
#include <QTextEdit>


class Question {
public:
    int num1;
    int num2;
    char operatorSymbol;
    int correctAnswer;
    int userAnswer;
    bool isAnswered;

    Question();
};

class Student {
public:
    QString username;
    QString password;
    int score;
    QList<Question> testQuestions;
    QDateTime startTime;
    QDateTime endTime;

    Student();
    void reset();
    void saveTestRecord();
    QString getUsername() const;
};

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    Student getStudent() const;

signals:
    void loginSuccess();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    Student currentStudent;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setStudent(const Student &student);

private slots:
    void onStartTestClicked();
    void onEditQuestionsClicked();
    void onLeaderboardClicked();
    void onHistoryClicked();
    void onSwitchAccountClicked();
    void onExitClicked();
    void onNextQuestionClicked();
    void onPrevQuestionClicked();
    void onFinishTestClicked();
    void onAddQuestionClicked();
    void onDeleteQuestionClicked();
    void onSaveQuestionsClicked();
    void onLoadQuestionsClicked();

private:
    void createMenuButtons();
    QPushButton* createMenuButton(const QString &text, const QString &style);
    void createTestPage();
    void createQuestionEditPage();
    void createLeaderboardPage();
    void createHistoryPage();
    void showWelcomePage();
    void generateQuestions();
    void showQuestion(int index);
    bool validateAnswer();
    void finishTest();
    void updateQuestionTable();
    void loadLeaderboard();
    void updateLeaderboard();
    void updateLeaderboardTable();
    void loadHistory();


    QVBoxLayout *mainLayout;
    QLabel *welcomeLabel;
    QStackedWidget *stackedWidget;
    Student currentStudent;
    int currentQuestionIndex;

    // 测试页面组件
    QWidget *testPage;
    QLabel *questionLabel, *startHintLabel;
    QLineEdit *answerEdit;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QLabel *progressLabel;

    // 试题编辑页面组件
    QWidget *questionEditPage;
    QTableWidget *questionTable;

    // 排行榜页面组件
    QWidget *leaderboardPage;
    QTableWidget *leaderboardTable;
    QList<QPair<QString, int>> leaderboardData;

    // 历史记录页面组件
    QWidget *historyPage;
    QListWidget *historyList;
    QTextEdit *historyTextEdit;
};

class AddQuestionDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddQuestionDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("添加手动试题");
        setFixedSize(300, 200);

        // 创建输入控件
        num1Edit = new QLineEdit(this);
        num1Edit->setValidator(new QIntValidator(0, 100, this)); // 限制为0-100的整数
        num1Edit->setPlaceholderText("请输入第一个数字");

        num2Edit = new QLineEdit(this);
        num2Edit->setValidator(new QIntValidator(0, 100, this));
        num2Edit->setPlaceholderText("请输入第二个数字");

        operatorCombo = new QComboBox(this);
        operatorCombo->addItems({"+", "-", "*", "/"}); // 支持加减乘除

        // 布局
        QFormLayout *layout = new QFormLayout(this);
        layout->addRow("第一个数字:", num1Edit);
        layout->addRow("运算符:", operatorCombo);
        layout->addRow("第二个数字:", num2Edit);

        // 确认/取消按钮
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *okBtn = new QPushButton("确认添加", this);
        QPushButton *cancelBtn = new QPushButton("取消", this);
        btnLayout->addWidget(okBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addRow(btnLayout);

        connect(okBtn, &QPushButton::clicked, this, &AddQuestionDialog::validateInput);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    }

    // 获取输入的试题信息
    Question getQuestion() const {
        return question;
    }

private slots:
    void validateInput() {
        // 验证输入不为空
        if (num1Edit->text().isEmpty() || num2Edit->text().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "数字不能为空！");
            return;
        }

        // 解析输入
        int num1 = num1Edit->text().toInt();
        int num2 = num2Edit->text().toInt();
        QChar op = operatorCombo->currentText().at(0);

        // 验证特殊运算符（减法/除法）的合法性
        if (op == '-' && num1 < num2) {
            QMessageBox::warning(this, "输入错误", "减法时第一个数字不能小于第二个数字！");
            return;
        }
        if (op == '/') {
            if (num2 == 0) {
                QMessageBox::warning(this, "输入错误", "除数不能为0！");
                return;
            }
            if (num1 % num2 != 0) {
                QMessageBox::warning(this, "输入错误", "除法结果必须为整数！");
                return;
            }
        }

        // 计算正确答案
        int correctAnswer = 0;
        switch (op.toLatin1()) {
        case '+': correctAnswer = num1 + num2; break;
        case '-': correctAnswer = num1 - num2; break;
        case '*': correctAnswer = num1 * num2; break;
        case '/': correctAnswer = num1 / num2; break;
        }

        // 保存试题信息
        question.num1 = num1;
        question.num2 = num2;
        question.operatorSymbol = op.toLatin1();
        question.correctAnswer = correctAnswer;

        accept(); // 关闭对话框并返回成功
    }

private:
    QLineEdit *num1Edit;
    QLineEdit *num2Edit;
    QComboBox *operatorCombo;
    Question question; // 用于存储生成的试题
};

#endif // MAINWINDOW_H
