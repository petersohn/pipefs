#include "pipefs_controller.h"
#include "log.h"
#include "data.h"
#include "functions_common.h"

#include "Controller.hpp"
#include "FileData.hpp"

#include <boost/exception/all.hpp>
#include <boost/system/system_error.hpp>
#include <memory>

extern "C" {


#define TRY(expr) try { expr; } \
    catch (std::exception& e) { \
        log_msg("%s\n", boost::diagnostic_information(e).c_str()); \
        throw; \
    }

#define TRY_SYSTEM_ERROR(name, expr) TRY( \
    try { expr; } \
    catch (boost::system::system_error& e) { \
        if (e.code().category() == boost::system::system_category()) {\
                log_error(name, e.code().value()); \
                return -e.code().value(); \
        } else { throw; } \
    })

struct pipefs_controller* pipefs_controller_create(
                const struct pipefs_data* data)
{
    TRY(return reinterpret_cast<pipefs_controller*>(
            new pipefs::Controller{*data}));
}

void pipefs_controller_destroy(struct pipefs_controller* controller)
{
    TRY(delete reinterpret_cast<const pipefs::Controller*>(controller));
}

int pipefs_controller_open(pipefs_controller* controller, const char* filename,
        int fd, struct fuse_file_info* fi, pipefs_filedata** result)
{
    TRY_SYSTEM_ERROR("pipefs_open",
        pipefs::FileData* fileData =
        reinterpret_cast<pipefs::Controller*>(controller)->open(
                filename, fd, *fi);
        *result = reinterpret_cast<pipefs_filedata*>(fileData);
    );
    return 0;
}

int pipefs_controller_read(pipefs_controller* controller,
        struct pipefs_filedata* data, void* buffer, size_t size, off_t offset)
{
    TRY_SYSTEM_ERROR("pipefs_read",
        return reinterpret_cast<pipefs::Controller*>(controller)->read(
                reinterpret_cast<pipefs::FileData*>(data), buffer, size, offset);
    );
}

int pipefs_controller_release(pipefs_controller* controller,
        const char* filename, struct pipefs_filedata* data)
{
    TRY_SYSTEM_ERROR("pipefs_release",
        reinterpret_cast<pipefs::Controller*>(controller)->release(
                filename, reinterpret_cast<pipefs::FileData*>(data));
    );
    return 0;
}

int pipefs_controller_correct_stat_info_file(struct pipefs_controller* controller,
        const char* filename, struct stat* statbuf)
{
    TRY_SYSTEM_ERROR("pipefs_controller_correct_stat_info_file",
        reinterpret_cast<pipefs::Controller*>(controller)->correctStatInfo(
                filename, statbuf);
    );
    return 0;
}

int pipefs_controller_correct_stat_info_fd(struct pipefs_controller* controller,
        struct pipefs_filedata* data, struct stat* statbuf)
{
    TRY_SYSTEM_ERROR("pipefs_controller_correct_stat_info_fd",
        reinterpret_cast<pipefs::Controller*>(controller)->correctStatInfo(
                reinterpret_cast<pipefs::FileData*>(data), statbuf);
    );
    return 0;
}

} // extern "C"

