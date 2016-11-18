#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include "recognizer.h"
#include "facedetect.h"

ImageViewer::ImageViewer() {

    /* Create UI */

    imageLabel = new QLabel;
    imageLabel->setScaledContents(true);
    scrollArea = new QScrollArea;
    scrollArea->setWidget(imageLabel);
    imageResult = new QLabel;
    imageResult->setScaledContents(true);
    scrollArea2 = new QScrollArea;
    scrollArea2->setWidget(imageResult);
    vLayout = new QHBoxLayout;
    vLayout->addWidget(scrollArea);
    vLayout->addWidget(scrollArea2);
    widget=new QWidget;
    widget->setLayout(vLayout);
    setCentralWidget(widget);

    createActions();
    createMenus();

    setWindowTitle(tr("Recognizer"));
    resize(500, 400);
}

void ImageViewer::open() {

    /* Open image */

    // Get file name through image dialog
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), QDir::currentPath());

    if (!fileName.isEmpty()) {

        // Iniitalize QImage object
        QImage image(fileName);

        // If an error has ocurred, alert the user
        if (image.isNull()) {
            QMessageBox::information(this, tr("recognizer"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }

        // Show the image
        imageLabel->setPixmap(QPixmap::fromImage(image));
        scaleFactor = 1.0;

        updateActions();
        imageLabel->adjustSize();

        // Recognize faces in the image and show the result
        char * file_name = fileName.toUtf8().data();
        Image img;
        try {
            img = Image(file_name);
            img.detect_faces();
            img.draw_faces(RED, true);
            imwrite("/Users/lfrias/Dropbox/dev/recognizer/resultados/result.tiff", img.as_mat());
            imageResult->setPixmap(QPixmap::fromImage(img.as_qtimage()));
            imageResult->adjustSize();
        } catch (std::exception const& e) {
            std::string error_str = img.get_errors() + " " + std::string(e.what());
            QMessageBox::information(this, "Error",  error_str.c_str());
            return;
        }

    }
}

void ImageViewer::about() {
    QMessageBox::about(this, tr("Sobre"),
            tr("<p>Trabalho desenvolvido para a disciplina Linguagens de Programação</p> "
               "<p>Alunos: "
               "<ul>"
               "<li>Lea Smazaro</li>"
               "<li>Luiz Fernando de Frias</li>"
               "<li>Rafaela Berberick</li>"
               "</ul>"
               "</p>"));
}

void ImageViewer::createActions() {
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void ImageViewer::createMenus() {
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(aboutAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addMenu(fileMenu);
}

void ImageViewer::updateActions() {
    //normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor) {
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
