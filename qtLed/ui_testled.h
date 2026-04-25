/********************************************************************************
** Form generated from reading UI file 'testled.ui'
**
** Created by: Qt User Interface Compiler version 5.9.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTLED_H
#define UI_TESTLED_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QGroupBox *groupBox;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QGroupBox *groupBox_2;
    QRadioButton *radioButton_3;
    QRadioButton *radioButton_4;
    QGroupBox *groupBox_3;
    QRadioButton *radioButton_5;
    QRadioButton *radioButton_6;
    QGroupBox *groupBox_4;
    QRadioButton *radioButton_7;
    QRadioButton *radioButton_8;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QStringLiteral("Form"));
        Form->resize(800, 480);
        pushButton = new QPushButton(Form);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(260, 380, 51, 27));
        pushButton_2 = new QPushButton(Form);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(490, 380, 51, 27));
        groupBox = new QGroupBox(Form);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(120, 90, 241, 81));
        radioButton = new QRadioButton(groupBox);
        radioButton->setObjectName(QStringLiteral("radioButton"));
        radioButton->setGeometry(QRect(30, 40, 61, 23));
        radioButton->setChecked(true);
        radioButton_2 = new QRadioButton(groupBox);
        radioButton_2->setObjectName(QStringLiteral("radioButton_2"));
        radioButton_2->setGeometry(QRect(130, 40, 61, 23));
        radioButton_2->setChecked(false);
        groupBox_2 = new QGroupBox(Form);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(120, 210, 241, 81));
        radioButton_3 = new QRadioButton(groupBox_2);
        radioButton_3->setObjectName(QStringLiteral("radioButton_3"));
        radioButton_3->setGeometry(QRect(30, 40, 61, 23));
        radioButton_3->setChecked(true);
        radioButton_4 = new QRadioButton(groupBox_2);
        radioButton_4->setObjectName(QStringLiteral("radioButton_4"));
        radioButton_4->setGeometry(QRect(130, 40, 61, 23));
        radioButton_4->setChecked(false);
        groupBox_3 = new QGroupBox(Form);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(450, 90, 241, 81));
        radioButton_5 = new QRadioButton(groupBox_3);
        radioButton_5->setObjectName(QStringLiteral("radioButton_5"));
        radioButton_5->setGeometry(QRect(30, 40, 61, 23));
        radioButton_5->setChecked(true);
        radioButton_6 = new QRadioButton(groupBox_3);
        radioButton_6->setObjectName(QStringLiteral("radioButton_6"));
        radioButton_6->setGeometry(QRect(130, 40, 61, 23));
        radioButton_6->setChecked(false);
        groupBox_4 = new QGroupBox(Form);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(450, 210, 241, 81));
        radioButton_7 = new QRadioButton(groupBox_4);
        radioButton_7->setObjectName(QStringLiteral("radioButton_7"));
        radioButton_7->setGeometry(QRect(30, 40, 61, 23));
        radioButton_7->setChecked(true);
        radioButton_8 = new QRadioButton(groupBox_4);
        radioButton_8->setObjectName(QStringLiteral("radioButton_8"));
        radioButton_8->setGeometry(QRect(130, 40, 61, 23));
        radioButton_8->setChecked(false);

        retranslateUi(Form);

        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QApplication::translate("Form", "QT-LED\346\216\247\345\210\266\347\250\213\345\272\217", Q_NULLPTR));
        pushButton->setText(QApplication::translate("Form", "\346\211\247\350\241\214", Q_NULLPTR));
        pushButton_2->setText(QApplication::translate("Form", "\351\200\200\345\207\272", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("Form", "LED1\346\216\247\345\210\266", Q_NULLPTR));
        radioButton->setText(QApplication::translate("Form", "\346\211\223\345\274\200", Q_NULLPTR));
        radioButton_2->setText(QApplication::translate("Form", "\345\205\263\351\227\255", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("Form", "LED2\346\216\247\345\210\266", Q_NULLPTR));
        radioButton_3->setText(QApplication::translate("Form", "\346\211\223\345\274\200", Q_NULLPTR));
        radioButton_4->setText(QApplication::translate("Form", "\345\205\263\351\227\255", Q_NULLPTR));
        groupBox_3->setTitle(QApplication::translate("Form", "LED3\346\216\247\345\210\266", Q_NULLPTR));
        radioButton_5->setText(QApplication::translate("Form", "\346\211\223\345\274\200", Q_NULLPTR));
        radioButton_6->setText(QApplication::translate("Form", "\345\205\263\351\227\255", Q_NULLPTR));
        groupBox_4->setTitle(QApplication::translate("Form", "LED4\346\216\247\345\210\266", Q_NULLPTR));
        radioButton_7->setText(QApplication::translate("Form", "\346\211\223\345\274\200", Q_NULLPTR));
        radioButton_8->setText(QApplication::translate("Form", "\345\205\263\351\227\255", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTLED_H
