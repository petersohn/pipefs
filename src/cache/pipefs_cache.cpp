#include "pipefs_cache.h"
#include "log.h"

#include "Controller.hpp"
#include "FileData.hpp"
#include "data.h"

#include <boost/exception/all.hpp>
#include <boost/system/system_error.hpp>
#include <memory>

extern "C" {


#define TRY(expr) try { expr; } \
        catch (std::exception& e) { \
        log_msg("%s\n", boost::diagnostic_information(e).c_str()); \
        throw; \
}

#define TRY_SYSTEM_ERROR(name, expr) TRY(try { expr; } \
        catch (boost::system::system_error& e) { \
                if (e.code().category() == boost::system::system_category()) {\
                        log_error(name, e.code().value()); \
                        return -e.code().value(); \
                } else { throw; }})

void log_error(const char *str, int error)
{
    log_msg("    ERROR %s: %s\n", str, strerror(error));
}

int pipefs_get_original_fd(struct pipefs_filedata* data)
{
        return reinterpret_cast<pipefs::FileData*>(data)->originalFd;
}

struct pipefs_controller* pipefs_controller_create(
                const struct pipefs_data* data)
{
        bool seekable = data->seekable;
        bool useCache = data->cache;
        TRY(return reinterpret_cast<pipefs_controller*>(
                new pipefs::Controller{data->command, seekable, useCache,
                data->cache_limit}));
}

void pipefs_controller_destroy(struct pipefs_controller* controller)
{
        TRY(delete reinterpret_cast<const pipefs::Controller*>(controller));
}

int pipefs_controller_open(pipefs_controller* controller, const char* filename,
        const char* translatedPath, struct fuse_file_info* fi,
        pipefs_filedata** result)
{
        TRY_SYSTEM_ERROR("pipefs_open",
            pipefs::FileData* fileData =
            reinterpret_cast<pipefs::Controller*>(controller)->open(
                            filename, translatedPath, *fi);
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

}

