#ifndef IMAGE_PROXY_H
#define IMAGE_PROXY_H

#include "lms/module.h"
#include "lms/datamanager.h"
#include "lms/imaging/image.h"

class ImageProxy : public lms::Module {
public:
    bool initialize() override;
    bool deinitialize() override;
    bool cycle() override;
private:
    enum DisplayMode {
        // load single image from an absolute path
        SINGLE_FILE,

        // load all image files from a directory
        DIRECTORY,

        // copy the image from an input data channel
        IMAGE_CHANNEL
    };

    enum PlayMode {
        // mode SINGLE_FILE: ignored
        // mode DIRECTORY: automatic playing mode (increment image index)
        // mode IMAGE_CHANNEL: show live image
        PLAY,

        // mode SINGLE_FILE: ignored
        // mode DIRECTORY: manual mode (go to previous or next image with messaging)
        // mode IMAGE_CHANNEL: show persistent image
        STOP
    };

    DisplayMode displayMode;
    PlayMode playMode;

    std::string singleFile;

    std::string directory;
    std::vector<std::string> dirFiles;
    size_t dirFilesIndex;

    const lms::imaging::Image *inputImage;
    lms::imaging::Image *outputImage;
    const lms::type::ModuleConfig *config;

    void imageChannelMode();
    void singleFileMode();
    void directoryMode();

    void fetchMessages();
    static void loadDirectory(const std::string &dir, std::vector<std::string>& files);
};

#endif /* IMAGE_PROXY_H */
