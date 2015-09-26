#ifndef IMAGE_PROXY_H
#define IMAGE_PROXY_H

#include <deque>

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

    bool displayModeFromString(const std::string &s, DisplayMode &mode);

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

    // for displayMode DIRECTORY
    std::string directory;
    std::vector<std::string> dirFiles;
    size_t dirFilesIndex;

    // for displayMode IMAGE_CHANNEL
    size_t maxBufferSize;
    std::deque<lms::imaging::Image> historyBuffer;
    std::deque<lms::imaging::Image> futureBuffer;
    int bufferIndex;

    const lms::imaging::Image *inputImage;
    lms::imaging::Image *outputImage;
    const lms::ModuleConfig *config;

    void imageChannelMode();
    void singleFileMode();
    void directoryMode();

    void fetchMessages();
    static void loadDirectory(const std::string &dir, std::vector<std::string>& files);
    static void drawFailImage(lms::imaging::Image &image);
};

#endif /* IMAGE_PROXY_H */
