#include "facedetect.h"

/* Expcetions definition */

class IOError: public std::exception {
  virtual const char* what() const throw() {
    return "IO error";
  }
} IOException;
class FaceDetectError: public std::exception {
  virtual const char* what() const throw() {
    return "Face detection error";
  }
} FaceDetectException;

/* Functions to be shared between objects */

// Load the XML file to the cascade object
void load_haarcascade(cv::CascadeClassifier& cascade, std::string haarcascade_file_name) {
    if( !cascade.load(haarcascade_file_name) )
        throw IOException;
}
// Draw a rectangle of the size of a ROI with a given color
void draw_roi_vector_in_mat(cv::Mat &img, std::vector<cv::Rect> &objs, unsigned color) {
    for(std::vector<cv::Rect>::const_iterator r = objs.begin(); r != objs.end(); r++) {
        rectangle(
                    img,
                    cvPoint(r->x, r->y),
                    cvPoint(r->x + r->width, r->y + r->height),
                    Color(color).get_scalar(),
                    4.0
                );
    }
}

// Returns a new image that is a cropped version
// of the original image.
cv::Mat crop_image(cv::Mat img, cv::Rect region)
{
    if (img.size().width <= 0 || img.size().height <= 0
        || region.width <= 0 || region.height <= 0) {
        std::cerr << "ERROR in crop_mage(): invalid dimensions." << std::endl;
        exit(1);
    }

    if (img.depth() != CV_8U) {
        std::cerr << "ERROR in crop_image(): image depth is not 8." << std::endl;
        exit(1);
    }

    // Set the desired region of interest.
    cv::Mat image_cropped = img(region);
    return image_cropped;
}

/* Creates a new image copy that is of a desired size. The aspect ratio will
 * be kept constant if 'keepAspectRatio' is true, by cropping undesired parts
 * so that only pixels of the original image are shown, instead of adding
 *extra blank space.
 */
cv::Mat resize_image(cv::Mat orig_img, int new_width,
        int new_height, bool keep_aspect_ratio)
{
    cv::Mat out_img;
    int orig_width;
    int orig_height;
    if (orig_img.data) {
        orig_width = orig_img.size().width;
        orig_height = orig_img.size().height;
    }
    if (new_width <= 0 || new_height <= 0 || orig_img.data == 0
            || orig_width <= 0 || orig_height <= 0) {
        std::cerr << "ERROR: Bad desired image size of " << new_width
        << "x" << new_height << " in resizeImage()." << std::endl;
        exit(1);
    }

    if (keep_aspect_ratio) {
        // Resize the image without changing its aspect ratio,
        // by cropping off the edges and enlarging the middle section.
        cv::Rect r;
        // input aspect ratio
        float orig_aspect = (orig_width / (float)orig_height);
        // output aspect ratio
        float new_aspect = (new_width / (float)new_height);
        // crop width to be orig_height * new_aspect
        if (orig_aspect > new_aspect) {
            int tw = (orig_height * new_width) / new_height;
            r = cv::Rect((orig_width - tw)/2, 0, tw, orig_height);
        } else {    // crop height to be orig_width / new_aspect
            int th = (orig_width * new_height) / new_width;
            r = cv::Rect(0, (orig_height - th)/2, orig_width, th);
        }
        cv::Mat cropped_img = crop_image(orig_img, r);

        // Call this function again, with the new aspect ratio image.
        // Will do a scaled image resize with the correct aspect ratio.
        out_img = resize_image(cropped_img, new_width, new_height, false);

    } else {

        // Scale the image to the new dimensions,
        // even if the aspect ratio will be changed.
        cv::Size new_size = cv::Size(new_width, new_height);
        if (new_width > orig_img.size().width && new_height > orig_img.size().height) {
            // Make the image larger
            // CV_INTER_LINEAR: good at enlarging.
            // CV_INTER_CUBIC: good at enlarging.
            cv::resize(orig_img, out_img, new_size, CV_INTER_LINEAR);
        } else {
            // Make the image smaller
            // CV_INTER_AREA: good at shrinking (decimation) only.
            cv::resize(orig_img, out_img, new_size, CV_INTER_AREA);
        }
    }
    return out_img;
}

/*
 * Read a CSV file and fill the images and the labels vector
 */
void read_csv(const std::string& filename, std::vector<cv::Mat>& images,
        std::vector<int>& labels, std::map<int, std::string>&vlabels, cv::Size size,
        cv::Size max_face_size, char separator = ';') {

    // Store the greatest size possible
    cv::Size max_size = max_face_size;
    std::ifstream file(filename.c_str(), std::ifstream::in);
    int i = 0;
    if (!file) {
        std::string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    std::string line, path, classlabel, verboselabel;
    // Loop through lines
    while (getline(file, line)) {
        std::stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel, separator);
        getline(liness, verboselabel);
        if(!path.empty() && !classlabel.empty() && !verboselabel.empty()) {
            // Load the given image
            cv::Mat m = cv::imread(path, CV_LOAD_IMAGE_GRAYSCALE);
            if (!m.data) {
                DEBUG_MSG("Could not load the image " << path);
                exit(0);
            }
            DEBUG_MSG(m.size());
            // Update the greatest size possible
            if (m.size().width*m.size().height > max_face_size.width*max_face_size.height) {
                max_size = m.size();
            }
            // Update the vector
            images.push_back(m);
            i = atoi(classlabel.c_str());
            labels.push_back(i);
            vlabels[i] = verboselabel;
        }
    }
    // Loop over images to resize them accordingly the biggest size
    int index;
    for(std::vector<cv::Mat>::iterator r = images.begin(); r != images.end(); r++) {
        index = r - images.begin();
        cv::Mat m = *r;
        cv::Mat m2 = resize_image(m, max_size.width, max_size.height, true);
        images[index] = m2;
        // Save the image (debug only)
        // imwrite("train" + std::to_string(index) + ".tiff", m2);
    }
}

/* Color Implementations */

Color::Color(unsigned code) {
    // Initialize the structure to store the Scalar object corresponding
    // to a specific color
    color = code;
    colors[AQUA]         = cv::Scalar(255  , 255 , 0);
    colors[WHITE]        = cv::Scalar(0    , 0   , 0);
    colors[BLUE]         = cv::Scalar(255  , 0   , 0);
    colors[RED]          = cv::Scalar(0    , 0   , 255);
    colors[YELLOW]       = cv::Scalar(0    , 255 , 255);
    colors[AQUAMARINE]   = cv::Scalar(212  , 255 , 127);
    colors[BLUE_VIOLET]  = cv::Scalar(226  , 43  , 138);
    colors[GREEN]        = cv::Scalar(0    , 255 , 127);
    colors[CORAL]        = cv::Scalar(80   , 127 , 255);
    colors[CBLUE]        = cv::Scalar(237  , 149 , 100);
    colors[PINK]         = cv::Scalar(255  , 0   , 255);
}
cv::Scalar Color::get_scalar() {
    return colors[color];
}
/* Face implementations */

Face::Face(cv::Rect face, cv::Mat bimg) {
    _is_face = false;
    img = bimg;
    roi = face;
    face_img = img(roi);
}
cv::Mat Face::as_mat() {
    return face_img;
}
// Get the region of interest of the image in which we can
// find the detected face
cv::Rect Face::get_roi() {
    return roi;
}
// Get face attributes
std::vector<cv::Rect> Face::get_eyes() {
    return eyes;
}
std::vector<cv::Rect> Face::get_nose() {
    return nose;
}
std::vector<cv::Rect> Face::get_mouth() {
    return mouth;
}
// After detect features we can tell wether this ROI is a face
bool Face::is_face() {
    detect_features();
    return _is_face;
}
/*
 * The feature detection has one purpose: more reliability
 * to the detected face. If at least one feature was found
 * the ROI can be considered a face and no other feature in
 * this ROI needs to be detected.
 *
 * The detection order is important as well. We start the detection
 * with the feature that has shown better results
 *
 */
void Face::detect_features() {
    DEBUG_MSG("Detectando features...");
    detect_eyes();
    // At least 2 eyes
    if (eyes.size() >= 2) {
        DEBUG_MSG("Olhos detectados!");
        _is_face = true;
        return;
    }
    detect_mouth();
    // At least one mouth
    if (mouth.size() != 0) {
        DEBUG_MSG("Boca detectada!");
        _is_face = true;
        return;
    }
    detect_nose();
    // At least one nose
    if (nose.size() != 0) {
        DEBUG_MSG("Nariz detectado!");
        _is_face = true;
        return;
    }
    DEBUG_MSG("Feito!");
}
/*
 * All detect_* functions have the same behaviour:
 * Load the XML file to be used by the classifier
 * and run the algorithm to detect features
 */
void Face::detect_eyes() {
    load_haarcascade(cascade, EYES_HAAR_FILE);
    cascade.detectMultiScale(face_img, eyes, 1.10, 5);
}
void Face::detect_mouth() {
    load_haarcascade(cascade, MOUTH_HAAR_FILE);
    cascade.detectMultiScale(face_img, mouth, 1.55, 5);
}
void Face::detect_nose() {
    load_haarcascade(cascade, NOSE_HAAR_FILE);
    cascade.detectMultiScale(face_img, nose, 1.20, 3);
}
/*
 *  Recognize the face
 *  It will tell the predicted label and the confidence of this prediction
 */
void Face::recognize(int &predicted_label, std::string &vlabel, double &predicted_confidence, cv::Size max_size) {
    // These vectors hold the images and corresponding labels.
    std::vector<cv::Mat> images;
    std::vector<int> labels;
    std::map<int, std::string> vlabels;
    // Read in the data
    try {
        read_csv(CSV_FILE_NAME , images, labels, vlabels, face_img.size(), max_size);
    } catch (cv::Exception& e) {
        std::cerr << "Error opening file \"" << CSV_FILE_NAME << "\". Reason: " << e.msg << std::endl;
        exit(1);
    }
    // Train the predictor
    DEBUG_MSG("Training the recognizer");
    cv::Ptr<cv::FaceRecognizer> model = cv::createEigenFaceRecognizer();
    model->train(images, labels);
    // Now predict the current face
    // Create a Mat to hold the current face scaled to the trained data size
    cv::Mat to_be_rec, to_be_rec_gray;
    cv::resize(face_img, to_be_rec, images[0].size());
    cvtColor(to_be_rec, to_be_rec_gray, CV_BGR2GRAY);
    //imwrite("predict" + std::to_string(rand() % 100) + ".tiff", to_be_rec_gray);
    DEBUG_MSG("Do the prediction");
    model->predict(to_be_rec_gray, predicted_label, predicted_confidence);
    vlabel = vlabels[predicted_label];
}
/* Image implementations */
Image::Image(char * file) {
    max_face_size = cv::Size(0, 0);
    filename = file;
    img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (!img.data) {
        errors.push_back("Error loading image " + std::string(filename));
        throw IOException;
    }
}
std::string Image::get_errors() {
    std::stringstream buffer;
    std::string delimiter =  "\n";
    std::copy(errors.begin(), errors.end(),
              std::ostream_iterator<std::string>(buffer, delimiter.c_str()));
    return buffer.str();
}
int Image::get_max_face_size() {
    return max_face_size.height*max_face_size.width;
}
void Image::set_max_face_size(cv::Size s) {
    max_face_size = s;
}
void Image::to_rgb() {
    cv::cvtColor(img, img, CV_GRAY2RGB);
}
void Image::equalize_hist() {
    cv::equalizeHist(img, img);
}
cv::Mat Image::as_mat() {
    return img;
}
QImage Image::as_qtimage() {
     cv::Mat temp; // make the same cv::Mat
     cvtColor(img, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
     QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     dest.bits(); // enforce deep copy, see documentation
     return dest;
}

void Image::detect_faces() {
    DEBUG_MSG("Detecting faces");
    // Equalize histogram and convert to grayscale for better results
    equalize_hist();
    to_rgb();
    try {
        load_haarcascade(cascade, FACE_HAAR_FILE);
    } catch (std::exception) {
        errors.push_back("Error loading " + std::string(FACE_HAAR_FILE));
        throw IOException;
    }
    std::vector<cv::Rect> img_faces;
    cascade.detectMultiScale(img, img_faces, 1.05, 5); // cascade.detectMultiScale(grayImage, objects, 1.1, 3, CV_HAAR_SCALE_IMAGE | CV_HAAR_DO_CANNY_PRUNING,cvSize(0,0), cvSize(100,100));
    DEBUG_MSG("Found " + std::to_string(img_faces.size()) + " faces suspects");
    for(std::vector<cv::Rect>::iterator r = img_faces.begin(); r != img_faces.end(); r++) {
        cv::Rect face_rect = *r;
        Face f = Face(face_rect, img);
        if (f.is_face()) {
            DEBUG_MSG("Face found!");
            faces.push_back(f);
            cv::Size s = f.as_mat().size();
            if (s.width*s.height > get_max_face_size())
                set_max_face_size(s);
        } else DEBUG_MSG("Not a face");
    }
    DEBUG_MSG("Face detection finished");
}
// Usefull for debug and to save the faces to CSV file
void Image::extract_faces() {
    DEBUG_MSG("Saving face images...");
    detect_faces();
    for(std::vector<Face>::iterator r = faces.begin(); r != faces.end(); r++) {
        Face f = *r;
        std::string result_file_name = std::to_string(r - faces.begin() + 1) + ".tiff";
        imwrite(result_file_name, f.as_mat());
    }
    DEBUG_MSG("Done!");
}
// Draw features found in a face and all faces in an image
void Face::draw_features() {
    draw_roi_vector_in_mat(face_img, mouth, YELLOW);
    draw_roi_vector_in_mat(face_img, nose, PINK);
    draw_roi_vector_in_mat(face_img, eyes, GREEN);
}
void Image::draw_faces(unsigned color, bool show_features) {
    std::vector<cv::Rect> roi_vector;
    for(cv::vector<Face>::iterator r = faces.begin(); r != faces.end(); r++) {
        Face f = *r;
        cv::Rect roi = f.get_roi();
        roi_vector.push_back(roi);
        if (show_features) f.draw_features();
        f.as_mat().copyTo(img(roi));
        int predicted_label = -1;
        double predicted_confidence = 0.0;
        std::string vlabel = "";
        f.recognize(predicted_label, vlabel, predicted_confidence, max_face_size);
        std::string box_text = vlabel + "/" + cv::format("%.2f", predicted_confidence);
        int pos_x = std::max(f.get_roi().tl().x - 10, 0);
        int pos_y = std::max(f.get_roi().tl().y - 10, 0);
        putText(img , box_text, cv::Point(pos_x, pos_y), cv::FONT_HERSHEY_PLAIN, 3.0, Color(GREEN).get_scalar(), 4.0);
    }
    draw_roi_vector_in_mat(img, roi_vector, color);
}


