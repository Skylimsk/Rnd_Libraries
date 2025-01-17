#include "imageprocessor.h"
#include <wx/app.h>

class ImageProcessorApp : public wxApp {
public:
    virtual bool OnInit() {
        if (!wxApp::OnInit())
            return false;

        wxInitAllImageHandlers();

        ImageProcessor* frame = new ImageProcessor("Image Processor");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(ImageProcessorApp);