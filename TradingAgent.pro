#-------------------------------------------------
#
# Project created by QtCreator 2013-11-13T13:24:43
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TradingAgent
TEMPLATE = app

LIBS += "/home/arch/Projects/Master/AI/BUILD/TradingAgent/3rdparty/libSMTPEmail.a"
LIBS += "/usr/lib/libfann.so"
#LIBS += -L"$$_PRO_FILE_PWD_/3rdparty/" -llibSMTPEmail
#LIBS += libSMTPEmail

SOURCES += main.cpp\
        mainwindow.cpp \
    assetsmanager.cpp \
    logger.cpp \
    transactionlogger.cpp \
    neuralnet.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    assetsmanager.h \
    logger.h \
    transactionlogger.h \
    neuralnet.h \
    util.h \
    constants.h \
    TrainingSet.h \
    Config.h \
    NeuralConfig.h

FORMS    += mainwindow.ui

CONFIG += c++11
