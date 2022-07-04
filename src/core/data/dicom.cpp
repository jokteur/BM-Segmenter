#include "dicom.h"

namespace Data {
    Tempo::jobId Matrix2DLoader::load(const std::string& path, Matrix2D& matrix2D, NotifyFct callback) {
        callback(false);
        return 0;
    }
    Tempo::jobId Matrix2DLoader::save(const std::string& path, Matrix2D& Matrix2D, NotifyFct callback) {
        callback(false);
        return 0;
    }
    void Matrix2DLoader::unload(Matrix2D& matrix2D, NotifyFct callback) {
        callback(false);
    }
}