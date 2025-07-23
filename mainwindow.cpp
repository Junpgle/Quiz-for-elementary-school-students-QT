#include "mainwindow.h"
#include <QApplication> // 确保包含 QApplication
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QIntValidator>
#include <QRandomGenerator>
#include <algorithm>
#include <ctime>

// Question 类实现
Question::Question() : num1(0), num2(0), operatorSymbol('+'),
    correctAnswer(0), userAnswer(0), isAnswered(false) {}

// Student 类实现
Student::Student() : score(0) {}

void Student::reset() {
    username.clear();
    password.clear();
    score = 0;
    testQuestions.clear();
}

void Student::saveTestRecord() {
    QString filename = username + ".txt";
    QFile file(filename);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "用户名: " << username << "\n";
        out << "得分: " << score << "\n";
        out << "开始时间: " << startTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "结束时间: " << endTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
        out << "试题信息:\n";

        for (int i = 0; i < testQuestions.size(); ++i) {
            const Question& q = testQuestions[i];
            out << (i+1) << ". " << q.num1 << " " << q.operatorSymbol << " "
                << q.num2 << " = ";
            if (q.isAnswered) {
                out << q.userAnswer;
                if (q.userAnswer == q.correctAnswer) {
                    out << " (正确)";
                } else {
                    out << " (错误, 正确答案: " << q.correctAnswer << ")";
                }
            } else {
                out << "未回答";
            }
            out << "\n";
        }
        out << "*******************************\n";
        file.close();
    }
}

QString Student::getUsername() const {
    return username;
}

// LoginDialog 类实现
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("小学生数学测验系统 - 登录");
    setFixedSize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("小学生数学测验系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 20px;");

    QFormLayout *formLayout = new QFormLayout();
    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("密码:", passwordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *loginButton = new QPushButton("登录", this);
    QPushButton *registerButton = new QPushButton("注册", this);

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    layout->addWidget(titleLabel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
}

Student LoginDialog::getStudent() const {
    return currentStudent;
}

void LoginDialog::onLoginClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
        return;
    }

    // 检查用户是否存在
    QFile userFile("users.txt");
    bool userExists = false;
    bool passwordCorrect = false;

    if (userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&userFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ");
            if (parts.size() >= 2 && parts[0] == username) {
                userExists = true;
                if (parts[1] == password) {
                    passwordCorrect = true;
                }
                break;
            }
        }
        userFile.close();
    }

    if (!userExists) {
        // 自动注册新用户
        if (userFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&userFile);
            out << username << " " << password << "\n";
            userFile.close();

            currentStudent.username = username;
            currentStudent.password = password;
            emit loginSuccess();
            accept();
        } else {
            QMessageBox::critical(this, "错误", "无法保存用户信息");
        }
    } else if (passwordCorrect) {
        currentStudent.username = username;
        currentStudent.password = password;
        emit loginSuccess();
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "密码错误");
    }
}

void LoginDialog::onRegisterClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
        return;
    }

    // 检查用户是否已存在
    QFile userFile("users.txt");
    bool userExists = false;

    if (userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&userFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ");
            if (parts.size() >= 1 && parts[0] == username) {
                userExists = true;
                break;
            }
        }
        userFile.close();
    }

    if (userExists) {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
    } else {
        if (userFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&userFile);
            out << username << " " << password << "\n";
            userFile.close();

            currentStudent.username = username;
            currentStudent.password = password;
            emit loginSuccess();
            accept();
            QMessageBox::information(this, "注册成功", "新用户注册成功！");
        } else {
            QMessageBox::critical(this, "错误", "无法保存用户信息");
        }
    }
}

// MainWindow 类实现
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentQuestionIndex(0) {
    setWindowTitle("小学生数学测验系统");
    setMinimumSize(800, 600);

    // 创建中央部件和主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    // 创建欢迎标签
    welcomeLabel = new QLabel("欢迎使用小学生数学测验系统", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px 0;");

    mainLayout->addWidget(welcomeLabel);

    // 创建功能按钮
    createMenuButtons();

    // 创建堆栈窗口
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    // 创建各个功能页面
    createTestPage();
    createQuestionEditPage();
    createLeaderboardPage();
    createHistoryPage();

    // 初始显示欢迎页面
    showWelcomePage();

    // 加载排行榜
    loadLeaderboard();
}

void MainWindow::setStudent(const Student &student) {
    currentStudent = student;
    welcomeLabel->setText("欢迎 " + currentStudent.username + " 使用小学生数学测验系统");
    generateQuestions();
}

void MainWindow::createMenuButtons() {
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *startTestButton = createMenuButton("开始答题", "background-color: #3498db; color: white;");
    QPushButton *editQuestionsButton = createMenuButton("编辑试题", "background-color: #2ecc71; color: white;");
    QPushButton *leaderboardButton = createMenuButton("排行榜", "background-color: #e74c3c; color: white;");
    QPushButton *historyButton = createMenuButton("历史记录", "background-color: #9b59b6; color: white;");
    QPushButton *switchAccountButton = createMenuButton("切换账号", "background-color: #f39c12; color: white;");
    QPushButton *exitButton = createMenuButton("退出系统", "background-color: #7f8c8d; color: white;");

    connect(startTestButton, &QPushButton::clicked, this, &MainWindow::onStartTestClicked);
    connect(editQuestionsButton, &QPushButton::clicked, this, &MainWindow::onEditQuestionsClicked);
    connect(leaderboardButton, &QPushButton::clicked, this, &MainWindow::onLeaderboardClicked);
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::onHistoryClicked);
    connect(switchAccountButton, &QPushButton::clicked, this, &MainWindow::onSwitchAccountClicked);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);

    buttonLayout->addWidget(startTestButton);
    buttonLayout->addWidget(editQuestionsButton);
    buttonLayout->addWidget(leaderboardButton);
    buttonLayout->addWidget(historyButton);
    buttonLayout->addWidget(switchAccountButton);
    buttonLayout->addWidget(exitButton);

    mainLayout->addLayout(buttonLayout);
}

QPushButton* MainWindow::createMenuButton(const QString &text, const QString &style) {
    QPushButton *button = new QPushButton(text, this);
    button->setStyleSheet("QPushButton { padding: 15px; font-size: 16px; border-radius: 8px; " + style + " }"
                                                                                                         "QPushButton:hover { opacity: 0.9; }");
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void MainWindow::createTestPage() {
    testPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(testPage);

    // 添加提示标签（初始隐藏）
    startHintLabel = new QLabel("请点击左侧的「开始答题」按钮开始测验", testPage);
    startHintLabel->setAlignment(Qt::AlignCenter);
    startHintLabel->setStyleSheet("font-size: 18px; color: #e74c3c; margin: 30px 0;");
    layout->addWidget(startHintLabel);

    questionLabel = new QLabel("", testPage);
    questionLabel->setAlignment(Qt::AlignCenter);
    questionLabel->setStyleSheet("font-size: 36px; font-weight: bold; margin: 30px 0;");
    questionLabel->setVisible(false);  // 初始隐藏题目
    layout->addWidget(questionLabel);

    answerEdit = new QLineEdit(testPage);
    answerEdit->setPlaceholderText("请输入答案...");
    answerEdit->setStyleSheet("font-size: 24px; padding: 10px;");
    answerEdit->setValidator(new QIntValidator(0, 100, this));
    answerEdit->setEnabled(false);  // 初始禁用

    // 添加回车键监听
    connect(answerEdit, &QLineEdit::returnPressed, this, [this]() {
        // 按下Enter时，触发与"下一题"按钮相同的逻辑
        onNextQuestionClicked();
    });

    layout->addWidget(answerEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    prevButton = new QPushButton("上一题", testPage);
    nextButton = new QPushButton("下一题", testPage);


    // 设置按钮样式
    prevButton->setStyleSheet("padding: 10px; font-size: 16px; background-color: #3498db; color: white;");
    nextButton->setStyleSheet("padding: 10px; font-size: 16px; background-color: #2ecc71; color: white;");


    // 初始禁用按钮
    prevButton->setEnabled(false);
    nextButton->setEnabled(false);


    connect(prevButton, &QPushButton::clicked, this, &MainWindow::onPrevQuestionClicked);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextQuestionClicked);


    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);


    progressLabel = new QLabel("", testPage);
    progressLabel->setAlignment(Qt::AlignCenter);
    progressLabel->setStyleSheet("font-size: 16px; color: #7f8c8d;");
    progressLabel->setVisible(false);  // 初始隐藏进度
    layout->addLayout(buttonLayout);
    layout->addWidget(progressLabel);

    stackedWidget->addWidget(testPage);
}

void MainWindow::createQuestionEditPage() {
    questionEditPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(questionEditPage);

    questionTable = new QTableWidget(0, 4, this);
    QStringList headers = {"序号", "题目", "正确答案", "操作"};
    questionTable->setHorizontalHeaderLabels(headers);
    questionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    questionTable->verticalHeader()->setVisible(false);
    questionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    questionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("添加题目", questionEditPage);
    QPushButton *deleteButton = new QPushButton("删除题目", questionEditPage);
    QPushButton *saveButton = new QPushButton("保存试题", questionEditPage);
    QPushButton *loadButton = new QPushButton("加载试题", questionEditPage);
    QPushButton *backButton = new QPushButton("返回主菜单", questionEditPage);

    addButton->setStyleSheet("padding: 10px; background-color: #2ecc71; color: white;");
    deleteButton->setStyleSheet("padding: 10px; background-color: #e74c3c; color: white;");
    saveButton->setStyleSheet("padding: 10px; background-color: #3498db; color: white;");
    loadButton->setStyleSheet("padding: 10px; background-color: #9b59b6; color: white;");
    backButton->setStyleSheet("padding: 10px; background-color: #f39c12; color: white;");

    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddQuestionClicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteQuestionClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveQuestionsClicked);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadQuestionsClicked);
    connect(backButton, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(0); });

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(backButton);

    layout->addWidget(questionTable);
    layout->addLayout(buttonLayout);

    stackedWidget->addWidget(questionEditPage);
}

void MainWindow::createLeaderboardPage() {
    leaderboardPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(leaderboardPage);

    leaderboardTable = new QTableWidget(0, 3, this);
    QStringList headers = {"排名", "用户名", "得分"};
    leaderboardTable->setHorizontalHeaderLabels(headers);
    leaderboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    leaderboardTable->verticalHeader()->setVisible(false);
    leaderboardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QPushButton *backButton = new QPushButton("返回主菜单", leaderboardPage);
    backButton->setStyleSheet("padding: 10px; background-color: #f39c12; color: white;");
    connect(backButton, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(0); });

    layout->addWidget(leaderboardTable);
    layout->addWidget(backButton);

    stackedWidget->addWidget(leaderboardPage);
}

void MainWindow::createHistoryPage() {
    historyPage = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(historyPage);

    // 替换QListWidget为QTextEdit，用于显示完整文件内容
    historyTextEdit = new QTextEdit(this);
    historyTextEdit->setReadOnly(true); // 设置为只读，防止编辑
    historyTextEdit->setStyleSheet("font-size: 14px; padding: 10px;");
    historyTextEdit->setLineWrapMode(QTextEdit::NoWrap); // 不自动换行（保持原格式）
    layout->addWidget(historyTextEdit);

    QPushButton *backButton = new QPushButton("返回主菜单", historyPage);
    backButton->setStyleSheet("padding: 10px; background-color: #f39c12; color: white;");
    connect(backButton, &QPushButton::clicked, this, [this]() { stackedWidget->setCurrentIndex(0); });

    layout->addWidget(backButton);

    stackedWidget->addWidget(historyPage);
}

void MainWindow::showWelcomePage() {
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::generateQuestions() {
    currentStudent.testQuestions.clear();
    currentStudent.score = 0;

    std::mt19937 rng(std::time(nullptr));

    for (int i = 0; i < 10; ++i) {
        Question q;
        q.num1 = rng() % 50;
        q.num2 = rng() % 50;
        int op = rng() % 2;
        q.operatorSymbol = (op == 0) ? '+' : '-';

        if (q.operatorSymbol == '+') {
            q.correctAnswer = q.num1 + q.num2;
            if (q.correctAnswer > 50) {
                i--;
                continue;
            }
        } else {
            if (q.num1 < q.num2) {
                std::swap(q.num1, q.num2);
            }
            q.correctAnswer = q.num1 - q.num2;
        }

        currentStudent.testQuestions.append(q);
    }

    currentStudent.startTime = QDateTime::currentDateTime();
}

void MainWindow::showQuestion(int index) {
    if (index < 0 || index >= currentStudent.testQuestions.size()) return;

    const Question &q = currentStudent.testQuestions[index];
    questionLabel->setText(QString("%1 %2 %3 = ?")
                               .arg(q.num1)
                               .arg(q.operatorSymbol)
                               .arg(q.num2));

    if (q.isAnswered) {
        answerEdit->setText(QString::number(q.userAnswer));
    } else {
        answerEdit->clear();
    }

    progressLabel->setText(QString("题目: %1/%2").arg(index+1).arg(currentStudent.testQuestions.size()));

    prevButton->setEnabled(index > 0);
    nextButton->setText(index < currentStudent.testQuestions.size() - 1 ? "下一题" : "完成测试");
}

bool MainWindow::validateAnswer() {
    if (answerEdit->text().isEmpty()) {
        QMessageBox::warning(this, "未完成", "请回答当前题目");
        return false;
    }

    int answer = answerEdit->text().toInt();
    Question &q = currentStudent.testQuestions[currentQuestionIndex];
    q.userAnswer = answer;
    q.isAnswered = true;

    if (q.userAnswer == q.correctAnswer) {
        currentStudent.score += 10;
    }

    return true;
}

void MainWindow::finishTest() {
    currentStudent.endTime = QDateTime::currentDateTime();

    // 保存测试记录
    currentStudent.saveTestRecord();

    // 更新排行榜
    updateLeaderboard();

    // 显示结果
    QString result;
    result += "测试完成!\n";
    result += "用户名: " + currentStudent.username + "\n";
    result += "得分: " + QString::number(currentStudent.score) + "/100\n";
    result += "评价: ";

    if (currentStudent.score >= 90) {
        result += "SMART";
    } else if (currentStudent.score >= 80) {
        result += "GOOD";
    } else if (currentStudent.score >= 70) {
        result += "OK";
    } else if (currentStudent.score >= 60) {
        result += "PASS";
    } else {
        result += "TRY AGAIN";
    }

    QMessageBox::information(this, "测试结果", result);

    // 重置测试页面状态
    startHintLabel->setVisible(true);
    questionLabel->setVisible(false);
    progressLabel->setVisible(false);
    answerEdit->setEnabled(false);
    prevButton->setEnabled(false);
    nextButton->setEnabled(false);


    stackedWidget->setCurrentIndex(0);
}

void MainWindow::updateQuestionTable() {
    questionTable->setRowCount(currentStudent.testQuestions.size());

    for (int i = 0; i < currentStudent.testQuestions.size(); ++i) {
        const Question &q = currentStudent.testQuestions[i];

        QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i+1));
        QTableWidgetItem *questionItem = new QTableWidgetItem(
            QString("%1 %2 %3 = ?").arg(q.num1).arg(q.operatorSymbol).arg(q.num2));
        QTableWidgetItem *answerItem = new QTableWidgetItem(QString::number(q.correctAnswer));

        QPushButton *deleteBtn = new QPushButton("删除");
        deleteBtn->setStyleSheet("padding: 5px; background-color: #e74c3c; color: white;");
        connect(deleteBtn, &QPushButton::clicked, this, [this, i]() {
            currentStudent.testQuestions.removeAt(i);
            updateQuestionTable();
        });

        questionTable->setItem(i, 0, indexItem);
        questionTable->setItem(i, 1, questionItem);
        questionTable->setItem(i, 2, answerItem);
        questionTable->setCellWidget(i, 3, deleteBtn);
    }
}

void MainWindow::loadLeaderboard() {
    leaderboardData.clear();

    QFile file("leaderboard.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" ");
            if (parts.size() == 2) {
                leaderboardData.append(qMakePair(parts[0], parts[1].toInt()));
            }
        }
        file.close();
    }

    // 按分数排序
    std::sort(leaderboardData.begin(), leaderboardData.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.second > b.second;
              });

    updateLeaderboardTable();
}

void MainWindow::updateLeaderboard() {
    // 添加当前用户成绩
    leaderboardData.append(qMakePair(currentStudent.username, currentStudent.score));

    // 按分数排序
    std::sort(leaderboardData.begin(), leaderboardData.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.second > b.second;
              });

    // 只保留前10名
    if (leaderboardData.size() > 10) {
        leaderboardData.resize(10);
    }

    // 保存到文件
    QFile file("leaderboard.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const auto &entry : leaderboardData) {
            out << entry.first << " " << entry.second << "\n";
        }
        file.close();
    }

    updateLeaderboardTable();
}

void MainWindow::updateLeaderboardTable() {
    leaderboardTable->setRowCount(leaderboardData.size());

    for (int i = 0; i < leaderboardData.size(); ++i) {
        const auto &entry = leaderboardData[i];

        QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(i+1));
        QTableWidgetItem *userItem = new QTableWidgetItem(entry.first);
        QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(entry.second));

        // 设置样式
        if (i == 0) {
            rankItem->setBackground(QColor(255, 215, 0));
            userItem->setBackground(QColor(255, 215, 0));
            scoreItem->setBackground(QColor(255, 215, 0));
        } else if (i == 1) {
            rankItem->setBackground(QColor(192, 192, 192));
            userItem->setBackground(QColor(192, 192, 192));
            scoreItem->setBackground(QColor(192, 192, 192));
        } else if (i == 2) {
            rankItem->setBackground(QColor(205, 127, 50));
            userItem->setBackground(QColor(205, 127, 50));
            scoreItem->setBackground(QColor(205, 127, 50));
        }

        leaderboardTable->setItem(i, 0, rankItem);
        leaderboardTable->setItem(i, 1, userItem);
        leaderboardTable->setItem(i, 2, scoreItem);
    }
}

void MainWindow::loadHistory() {
    historyTextEdit->clear(); // 清空原有内容

    QString filename = currentStudent.username + ".txt";
    QFile file(filename);

    if (file.exists()) { // 检查文件是否存在
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            // 读取文件全部内容（包括换行和格式）
            QString fileContent = in.readAll();
            file.close();

            // 直接显示文件完整内容
            historyTextEdit->setText(fileContent);
        } else {
            // 打开失败时显示错误信息
            historyTextEdit->setText("无法打开历史记录文件，请检查权限！");
        }
    } else {
        // 文件不存在时显示提示
        historyTextEdit->setText("暂无历史记录（未找到 " + filename + "）");
    }
}

// 槽函数实现
void MainWindow::onStartTestClicked() {

    if (currentStudent.testQuestions.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有试题，将自动生成10道题");
        generateQuestions();
    }

    stackedWidget->setCurrentWidget(testPage);
    currentQuestionIndex = 0;

    // 显示题目相关控件，隐藏提示
    startHintLabel->setVisible(false);
    questionLabel->setVisible(true);
    progressLabel->setVisible(true);

    // 启用输入框和按钮
    answerEdit->setEnabled(true);
    prevButton->setEnabled(true);
    nextButton->setEnabled(true);


    showQuestion(currentQuestionIndex);
}

void MainWindow::onEditQuestionsClicked() {
    stackedWidget->setCurrentWidget(questionEditPage);
    updateQuestionTable();
}

void MainWindow::onLeaderboardClicked() {
    stackedWidget->setCurrentWidget(leaderboardPage);
}

void MainWindow::onHistoryClicked() {
    loadHistory();
    stackedWidget->setCurrentWidget(historyPage);
}

void MainWindow::onSwitchAccountClicked() {
    LoginDialog loginDialog(this);
    if (loginDialog.exec() == QDialog::Accepted) {
        setStudent(loginDialog.getStudent());
    }
}

void MainWindow::onExitClicked() {
    QApplication::quit();
}

void MainWindow::onNextQuestionClicked() {
    if (!validateAnswer()) return;

    if (currentQuestionIndex < currentStudent.testQuestions.size() - 1) {
        currentQuestionIndex++;
        showQuestion(currentQuestionIndex);
    } else {
        finishTest();
    }
}

void MainWindow::onPrevQuestionClicked() {
    if (currentQuestionIndex > 0) {
        currentQuestionIndex--;
        showQuestion(currentQuestionIndex);
    }
}

void MainWindow::onFinishTestClicked() {
    if (!validateAnswer()) return;
    finishTest();
}

void MainWindow::onAddQuestionClicked() {
    // 弹出手动输入对话框
    AddQuestionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 添加用户输入的试题到列表
        currentStudent.testQuestions.append(dialog.getQuestion());
        updateQuestionTable(); // 刷新表格显示
    }
}

void MainWindow::onDeleteQuestionClicked() {
    int row = questionTable->currentRow();
    if (row >= 0 && row < currentStudent.testQuestions.size()) {
        currentStudent.testQuestions.removeAt(row);
        updateQuestionTable();
    }
}

void MainWindow::onSaveQuestionsClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存试题", "", "文本文件 (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < currentStudent.testQuestions.size(); ++i) {
            const Question &q = currentStudent.testQuestions[i];
            out << (i+1) << ". " << q.num1 << " " << q.operatorSymbol << " "
                << q.num2 << " = " << q.correctAnswer << "\n";
        }
        file.close();
        QMessageBox::information(this, "保存成功", "试题已成功保存到文件");
    } else {
        QMessageBox::warning(this, "保存失败", "无法保存试题文件");
    }
}

void MainWindow::onLoadQuestionsClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "加载试题", "", "文本文件 (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        currentStudent.testQuestions.clear();

        while (!in.atEnd()) {
            QString line = in.readLine();
            // 简化解析逻辑
            QStringList parts = line.split(" ");
            if (parts.size() >= 5) {
                Question q;
                q.num1 = parts[1].toInt();
                q.operatorSymbol = parts[2][0].toLatin1();
                q.num2 = parts[3].toInt();
                q.correctAnswer = parts[5].toInt();
                currentStudent.testQuestions.append(q);
            }
        }
        file.close();
        updateQuestionTable();
        QMessageBox::information(this, "加载成功", "试题已成功从文件加载");
    } else {
        QMessageBox::warning(this, "加载失败", "无法加载试题文件");
    }
}
