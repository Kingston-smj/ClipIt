#include "logging.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

namespace app {
namespace {

constexpr qsizetype kMaxMessageBytes = 2048;

QByteArray log_path;
QByteArray lock_path;
QMutex log_mutex;
QtMessageHandler previous_handler = nullptr;

const char* levelName(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:    return "DEBUG";
    case QtInfoMsg:     return "INFO";
    case QtWarningMsg:  return "WARN";
    case QtCriticalMsg: return "ERROR";
    case QtFatalMsg:    return "FATAL";
    }
    return "UNKNOWN";
}

void appendLine(const QByteArray& line)
{
    if (log_path.isEmpty())
        return;

    QMutexLocker guard(&log_mutex);

    // Trigger processes and the daemon can log at the same time. The lock
    // keeps each line together across processes.
    const int lock_fd = ::open(lock_path.constData(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (lock_fd < 0)
        return;
    ::fchmod(lock_fd, 0600);
    if (::flock(lock_fd, LOCK_EX) != 0)
    {
        ::close(lock_fd);
        return;
    }

    const int fd = ::open(log_path.constData(), O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC, 0600);
    if (fd >= 0)
    {
        ::fchmod(fd, 0600);
        const char* data = line.constData();
        qsizetype remaining = line.size();
        while (remaining > 0)
        {
            const ssize_t written = ::write(fd, data, static_cast<size_t>(remaining));
            if (written <= 0)
                break;
            data += written;
            remaining -= written;
        }
        ::close(fd);
    }

    ::flock(lock_fd, LOCK_UN);
    ::close(lock_fd);
}

void messageHandler(QtMsgType type, const QMessageLogContext& context,
                    const QString& message)
{
    if (type != QtDebugMsg)
    {
        QByteArray body = message.toUtf8();
        body.replace('\n', ' ');
        body.truncate(kMaxMessageBytes);

        QByteArray line = QDateTime::currentDateTimeUtc()
                              .toString(Qt::ISODateWithMs).toUtf8();
        line += " [";
        line += levelName(type);
        line += "] [pid ";
        line += QByteArray::number(QCoreApplication::applicationPid());
        line += "] ";
        if (context.category && *context.category)
        {
            line += context.category;
            line += ": ";
        }
        line += body;
        line += '\n';
        appendLine(line);
    }

    if (previous_handler)
        previous_handler(type, context, message);
}

} // namespace

void install_logging()
{
    QString state_home = qEnvironmentVariable("XDG_STATE_HOME");
    if (state_home.isEmpty() || !QDir::isAbsolutePath(state_home))
        state_home = QDir::homePath() + QStringLiteral("/.local/state");

    const QString directory = state_home + QStringLiteral("/clipit");
    if (!QDir().mkpath(directory))
        return; // Qt's default handler still writes to the terminal.

    log_path = QFile::encodeName(directory + QStringLiteral("/clipit.log"));
    lock_path = QFile::encodeName(directory + QStringLiteral("/clipit.log.lock"));
    previous_handler = qInstallMessageHandler(messageHandler);
}

} // namespace app
