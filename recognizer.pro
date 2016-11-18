QT += widgets
qtHaveModule(printsupport): QT += printsupport

HEADERS       = recognizer.h \
                facedetect.h
SOURCES       = recognizer.cpp \
                main.cpp \
                facedetect.cpp

# install
target.path = /Users/lfrias/Dropbox/dev/recognizer/recognizer
INSTALLS += target

wince*: {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

CONFIG += MYHOTOSXMACHINE

MYHOTOSXMACHINE {
    INCLUDEPATH += /Users/lfrias/Dropbox/dev/recognizer/include/
    LIBS += -L/usr/local/lib/ \
            -lopencv_core \
            -lopencv_imgproc \
            -lopencv_highgui \
            -lopencv_objdetect \
            -lopencv_contrib
}

