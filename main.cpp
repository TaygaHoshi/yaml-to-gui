#include "mainwindow.h"
#include "yaml-cpp/yaml.h"

#include <QApplication>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>

class MyTestWindow: public MainWindow {
private:
    // default size
    int mainWindowWidth = 800;
    int mainWindowHeight = 600;
    // minimum size
    int mainWindowWidthMin = 400;
    int mainWindowHeightMin = 300;

    // create and set central widget
    QWidget* central = new QWidget(this);
    QFormLayout* mainWindowLayout = new QFormLayout(central);

    // create menubar
    QMenuBar* menubar = new QMenuBar(this);

    // keep track of all form items
    QList<QWidget*> dynamicFormItems = QList<QWidget*>();

    // fonts
    QFont* basefont = new QFont(QString("Monospace"), 14, 200, false);
    QFontMetrics* basefontMetrics = new QFontMetrics(*this->basefont);
    QFont* basefontBold = new QFont(QString("Monospace"), 14, 600, false);
    QFont* basefontItalic = new QFont(QString("Monospace"), 14, 200, true);
public:
    MyTestWindow(){
        // Set layout and size when initializing
        this->setCentralWidget(central);
        this->setMenuBar(menubar);
        this->setWindowTitle(QString("Yaml to GUI"));
        this->resize(QSize(this->mainWindowWidth, this->mainWindowHeight));
        this->setMinimumSize(QSize(this->mainWindowWidthMin, this->mainWindowHeightMin));
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->central->setLayout(mainWindowLayout);
        this->central->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // toolbar - filemenu
        QMenu* fileMenu = menubar->addMenu(QString("File"));
        QAction* openfileAction = fileMenu->addAction(QString("Select YAML"));
        openfileAction->connect(fileMenu, &QMenu::triggered, [=]() {
            // fill data from the selected yaml file
            //// select a yaml file
            QString fileName = QFileDialog::getOpenFileName(this, "Open YAML File", "", "YAML Files (*.yaml *.yml)");

            //// if a file is chosen:
            if (!fileName.isEmpty()) {
                try {
                    YAML::Node yamlFile = YAML::LoadFile(fileName.toStdString());

                    this->dynamicFormItems.clear();
                    for (QWidget* oldw : this->central->findChildren<QWidget*>(Qt::FindDirectChildrenOnly)) {
                        if (!qobject_cast<QPushButton*>(oldw)){
                            oldw->deleteLater();
                        }
                    }

                    this->fillDataFromYaml(yamlFile, "");

                    QTimer::singleShot(1, [=]() {
                        this->adjustSize();
                    });
                } catch (const std::exception &e) {
                    QMessageBox::warning(this, "Error", "Failed to load the YAML file.");
                }


            }
        });

        // toolbar - help menu
        QMenu* helpMenu = menubar->addMenu(QString("Help"));
        QAction* aboutAction = helpMenu->addAction(QString("About"));
        aboutAction->connect(helpMenu, &QMenu::triggered, [=]() {
            QMessageBox::information(this, "About", "YAML to GUI<br>Created by TaygaHoshi. GPLv3.0.<br><br><a href='https://github.com/TaygaHoshi'>GitHub</a>");
        });

        // create the send button
        QPushButton* finalPushButton = new QPushButton(this->central);
        finalPushButton->setText(QString("Generate"));
        finalPushButton->setFont(*this->basefont);
        finalPushButton->connect(finalPushButton, &QPushButton::clicked, [=]() {
            // this function is ran whenever the button is clicked
            for(QWidget* currentWidget : this->dynamicFormItems) {
                qDebug() << "WidgetResult:" << this->getValueFromWidget(currentWidget);
            }
        });
        this->mainWindowLayout->addRow(finalPushButton);

    }

    QString getValueFromWidget(QWidget* currentWidget) {
        QString returnValue;

        if (QComboBox* currentWidgetCombobox = qobject_cast<QComboBox*>(currentWidget)) {
            returnValue = currentWidgetCombobox->currentText();
        }
        if (QTextEdit* currentWidgetTextEdit = qobject_cast<QTextEdit*>(currentWidget)) {
            returnValue = currentWidgetTextEdit->toPlainText();
        }

        return returnValue;
    }

    QWidget* createComboBox(QString labelText, QList<QString> data) {
        QLabel* comboboxLabel = new QLabel(labelText, this->central);
        comboboxLabel->setFont(*this->basefontBold);

        QComboBox* comboboxWidget = new QComboBox(this->central);
        comboboxWidget->setFont(*this->basefont);
        comboboxWidget->addItems(data);

        this->mainWindowLayout->addRow(comboboxLabel, comboboxWidget);
        this->dynamicFormItems.append(comboboxWidget);
        return comboboxWidget;
    }

    QWidget* createTextEdit(QString labelText) {
        QLabel* textEditLabel = new QLabel(labelText, this->central);
        textEditLabel->setFont(*this->basefontBold);

        QTextEdit* textEditWidget = new QTextEdit(this->central);
        textEditWidget->setFont(*this->basefont);
        textEditWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        textEditWidget->setMinimumHeight(this->basefontMetrics->height());

        this->mainWindowLayout->addRow(textEditLabel, textEditWidget);
        this->dynamicFormItems.append(textEditWidget);
        return textEditWidget;
    }

    void fillDataFromYaml(YAML::Node currentNode, std::string currentParent) {
        // recursive function to parse through the given yaml.
        for(YAML::const_iterator it=currentNode.begin();it != currentNode.end();++it) {
            switch (it->second.Type()) {
            case YAML::NodeType::Null:
                break;
            case YAML::NodeType::Scalar:
            {
                // create a default textbox
                const std::string value = it->second.as<std::string>();
                const std::string key = it->first.as<std::string>();
                this->createTextEdit(QString::fromStdString(currentParent + " " + key));
                break;
            }
            case YAML::NodeType::Sequence:
            {
                // create a combobox
                QList<QString> _temp_qlist = QList<QString>();
                const std::vector<std::string> yamlContents = it->second.as<std::vector<std::string>>();
                const std::string key = it->first.as<std::string>();

                for(std::string data : yamlContents) {
                    _temp_qlist.append(QString::fromStdString(data));
                }

                this->createComboBox(QString::fromStdString(currentParent + " " + key), _temp_qlist);
                break;
            }
            case YAML::NodeType::Map:
                if(currentParent == ""){
                    this->fillDataFromYaml(it->second, it->first.as<std::string>());
                }
                else {
                    this->fillDataFromYaml(it->second, currentParent + " " + it->first.as<std::string>());
                }
                break;
            case YAML::NodeType::Undefined:
                break;
            }
        }
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyTestWindow w;
    w.show();
    return a.exec();
};
