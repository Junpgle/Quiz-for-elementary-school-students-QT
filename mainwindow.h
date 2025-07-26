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
#include <QRandomGenerator>

struct LeaderboardEntry {
    QString username;  // 用户名
    int score;         // 得分
    int timeSeconds;   // 做题时间（秒）
};

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
    void resetAnswers();
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
    void updateLeaderboard(int timeSeconds);
    void updateLeaderboardTable();
    void loadHistory();

    // 在MainWindow类中添加排序函数
    void sortLeaderboard();


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
    QList<LeaderboardEntry> leaderboardData;

    // 历史记录页面组件
    QWidget *historyPage;
    QListWidget *historyList;
    QTextEdit *historyTextEdit;
};


class AddQuestionDialog : public QDialog {
    Q_OBJECT
private:
    Question question;
    QLineEdit *num1Edit;
    QComboBox *operatorCombo;
    QLineEdit *num2Edit;
    QLabel *answerLabel;  // 用于显示计算结果，替换原来的answerEdit

public:
    AddQuestionDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("添加试题");
        setFixedSize(300, 250);

        QVBoxLayout *layout = new QVBoxLayout(this);

        // 数字1
        QHBoxLayout *num1Layout = new QHBoxLayout();
        num1Layout->addWidget(new QLabel("数字1:"));
        num1Edit = new QLineEdit();
        num1Edit->setValidator(new QIntValidator(0, 1000, this));
        num1Layout->addWidget(num1Edit);
        layout->addLayout(num1Layout);

        // 运算符
        QHBoxLayout *opLayout = new QHBoxLayout();
        opLayout->addWidget(new QLabel("运算符:"));
        operatorCombo = new QComboBox();
        operatorCombo->addItems({"+", "-", "x", "÷"});
        opLayout->addWidget(operatorCombo);
        layout->addLayout(opLayout);

        // 数字2
        QHBoxLayout *num2Layout = new QHBoxLayout();
        num2Layout->addWidget(new QLabel("数字2:"));
        num2Edit = new QLineEdit();
        num2Edit->setValidator(new QIntValidator(0, 1000, this));
        num2Layout->addWidget(num2Edit);
        layout->addLayout(num2Layout);

        // 答案（改为显示标签，而非可编辑的输入框）
        QHBoxLayout *answerLayout = new QHBoxLayout();
        answerLayout->addWidget(new QLabel("答案:"));
        answerLabel = new QLabel("");
        answerLabel->setStyleSheet("font-weight: bold; color: #3498db;");
        answerLayout->addWidget(answerLabel);
        layout->addLayout(answerLayout);

        // 随机生成按钮
        QPushButton *randomButton = new QPushButton("随机生成一题");
        connect(randomButton, &QPushButton::clicked, this, &AddQuestionDialog::generateRandomQuestion);
        layout->addWidget(randomButton);

        // 确认和取消按钮
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *okButton = new QPushButton("确认");
        QPushButton *cancelButton = new QPushButton("取消");
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);
        layout->addLayout(buttonLayout);

        // 连接信号，实现自动计算
        connect(num1Edit, &QLineEdit::textChanged, this, &AddQuestionDialog::calculateAnswer);
        connect(operatorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddQuestionDialog::calculateAnswer);
        connect(num2Edit, &QLineEdit::textChanged, this, &AddQuestionDialog::calculateAnswer);

        connect(okButton, &QPushButton::clicked, this, &AddQuestionDialog::accept);
        connect(cancelButton, &QPushButton::clicked, this, &AddQuestionDialog::reject);

        // 初始计算
        calculateAnswer();
    }

    Question getQuestion() const {
        return question;
    }

private slots:
    void generateRandomQuestion() {
        // 随机生成1-100的数字
        int num1 = QRandomGenerator::global()->bounded(100) + 1;
        int num2 = QRandomGenerator::global()->bounded(100) + 1;
        bool isAddition = QRandomGenerator::global()->bounded(2) == 0;

        // 确保减法不会出现负数
        if (!isAddition && num1 < num2) {
            std::swap(num1, num2);
        }

        // 更新UI
        num1Edit->setText(QString::number(num1));
        operatorCombo->setCurrentIndex(isAddition ? 0 : 1);
        num2Edit->setText(QString::number(num2));

        // 触发计算
        calculateAnswer();
    }

    void calculateAnswer() {
        // 获取输入值
        bool ok1, ok2;
        int num1 = num1Edit->text().toInt(&ok1);
        int num2 = num2Edit->text().toInt(&ok2);
        QChar op = operatorCombo->currentText()[0];

        // 验证输入
        if (!ok1 || !ok2 || num1Edit->text().isEmpty() || num2Edit->text().isEmpty()) {
            answerLabel->setText("请输入有效数字");
            return;
        }

        // 计算答案
        int answer;
        if (op == '+') {
            answer = num1 + num2;
        } else if (op == '-'){
            // 确保减法结果非负
            if (num1 < num2) {
                answerLabel->setText("减法中数字1必须大于等于数字2");
                return;
            }
            answer = num1 - num2;
        } else if(op == 'x'){ // 乘法
            answer = num1 * num2;
        } else { // 除号
            answer = num1 / num2;
        }

        // 更新显示
        answerLabel->setText(QString::number(answer));

        // 更新内部问题对象
        question.num1 = num1;
        question.operatorSymbol = op.toLatin1();
        question.num2 = num2;
        question.correctAnswer = answer;
    }

    void accept() override {
        // 验证输入
        if (num1Edit->text().isEmpty() || num2Edit->text().isEmpty()) {
            QMessageBox::warning(this, "输入错误", "请填写所有字段");
            return;
        }

        // 验证减法运算
        if (operatorCombo->currentText() == "-" &&
            num1Edit->text().toInt() < num2Edit->text().toInt()) {
            QMessageBox::warning(this, "输入错误", "减法运算中，第一个数必须大于等于第二个数");
            return;
        }

        QDialog::accept();
    }
};

#endif // MAINWINDOW_H
