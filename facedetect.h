#ifndef FACEDETECT_H
#define FACEDETECT_H

#endif // FACEDETECT_H

/* OpenCV imports */
#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/* C++ imports */
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include "infix_iterator.h"

/* QT imports */
#include <QDebug>
#include <QImage>

/* Constants definition */
#define AQUA        0
#define WHITE       1
#define BLUE        2
#define RED         3
#define YELLOW      4
#define AQUAMARINE  5
#define BLUE_VIOLET 6
#define GREEN       7
#define CORAL       8
#define CBLUE       9
#define PINK        10

/* Configuration constants */

#define PROJECT_BASE    "/Users/lfrias/Dropbox/dev/recognizer/"
#define HAAR_BASE_DIR   PROJECT_BASE"haars"
#define IMAGES_BASE_DIR PROJECT_BASE"faces"

#define FACE_HAAR_FILE  HAAR_BASE_DIR"/haarcascade_frontalface_alt.xml"
#define EYES_HAAR_FILE  HAAR_BASE_DIR"/haarcascade_eye_tree_eyeglasses.xml"
#define MOUTH_HAAR_FILE HAAR_BASE_DIR"/Mouth.xml"
#define NOSE_HAAR_FILE  HAAR_BASE_DIR"/haarcascade_mcs_nose.xml"
#define CSV_FILE_NAME   IMAGES_BASE_DIR"/config.csv"
#define FACE_SIZE       50

// Debug functions
#ifdef Q_OS_MAC
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { qDebug() << str } while ( false )
#endif

/* Functions definition */

void load_haarcascade(cv::CascadeClassifier&, std::string);
void draw_roi_vector_in_mat(cv::Mat &, std::vector<cv::Rect> &, unsigned);
cv::Mat crop_image(cv::Mat, cv::Rect);
cv::Mat resize_image(cv::Mat, int);
void read_csv(const std::string&, std::vector<cv::Mat>&);


/* Types definition */
typedef std::map<int, cv::Scalar> dict;

/* Classes definition */

// This class is responsible by manage all operations in a face level, i.e,
// it will store the ROI and detect features like eyes, nose, mouth, ...
class Face {
    bool _is_face;
    cv::CascadeClassifier cascade;
    cv::Rect roi;
    cv::Mat img;
    cv::Mat face_img;
    std::vector<cv::Rect> eyes;
    std::vector<cv::Rect> mouth;
    std::vector<cv::Rect> nose;
    public:
        Face(cv::Rect, cv::Mat);
        bool is_face();
        cv::Mat as_mat();
        cv::Rect get_roi();
        std::vector<cv::Rect> get_eyes();
        std::vector<cv::Rect> get_mouth();
        std::vector<cv::Rect> get_nose();
        void detect_eyes();
        void detect_mouth();
        void detect_nose();
        void detect_features();
        void draw_features();
        void recognize(int &, std::string &, double &, cv::Size);
};

// This class is responsible by manage all image operationrs, i.e, it will
// load the image, detect all faces and finally draw what is has detected.
class Image {
    std::vector<std::string> errors;
    char *filename;
    cv::Mat img;
    cv::CascadeClassifier cascade;
    std::vector<Face> faces;
    cv::Size max_face_size;
    public:
        Image() = default;
        Image(char *);    
        cv::Mat as_mat();
        QImage as_qtimage();
        void to_rgb();
        void equalize_hist();
        void detect_faces();
        void draw_faces(unsigned, bool);
        void extract_faces();
        int get_max_face_size();
        std::string get_errors();
        void set_max_face_size(cv::Size);
};

// This class is responsible for return the Scalar object
// for the requested color
class Color {
    unsigned color;
    dict colors;
    public:
        Color(unsigned);
        cv::Scalar get_scalar();
};
