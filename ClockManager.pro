TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ClockManager/ClockManager.cpp \
    ClockManager/managercode/TimeWheel.cpp

HEADERS += \
    ClockManager/managercode/TimeWheel.h \
    ClockManager/managercode/ThreadPool.h \
    ClockManager/managercode/spinlock_mutex.h \
    ClockManager/managercode/spin_threadsafe_list.h \
    ClockManager/managercode/CircQueue.h

INCLUDEPATH+= /usr/local/boost_1_61_0
LIBS+= -L/usr/local/boost_1_61_0/stage/lib  -lboost_thread -lboost_regex -lboost_system
