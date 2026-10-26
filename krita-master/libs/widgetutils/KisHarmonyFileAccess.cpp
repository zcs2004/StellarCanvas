/*
 * SPDX-FileCopyrightText: 2026 Krita contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisHarmonyFileAccess.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <memory>
#include <QString>

#ifdef Q_OS_HARMONY
#include <QtHarmonyExtras/private/qohosfileshare_p.h>
#endif

namespace
{
#ifdef Q_OS_HARMONY
bool isExternalDocumentPath(const QString &path)
{
    if (path.isEmpty() || !QFileInfo(path).isAbsolute()) {
        return false;
    }

    const QString cleanPath = QDir::cleanPath(path);
    const QList<QStandardPaths::StandardLocation> privateLocations {
        QStandardPaths::AppDataLocation,
        QStandardPaths::CacheLocation,
        QStandardPaths::ConfigLocation,
        QStandardPaths::TempLocation
    };

    for (const auto location : privateLocations) {
        const QString root = QDir::cleanPath(QStandardPaths::writableLocation(location));
        if (!root.isEmpty()
            && (cleanPath == root || cleanPath.startsWith(root + QLatin1Char('/')))) {
            return false;
        }
    }
    return true;
}

QtHarmonyExtras::FileShare::PathPolicy pathPolicy(const QString &path, bool writable)
{
    using namespace QtHarmonyExtras::FileShare;
    OperationModes modes = OperationMode::Read;
    if (writable) {
        modes |= OperationMode::Write;
    }
    return {path, modes};
}

bool succeeded(const std::shared_ptr<QtHarmonyExtras::FileShare::ActionResult> &result)
{
    return result && result->operationStatus() && result->operationStatus()->success();
}
#endif
}

namespace KisHarmonyFileAccess
{
void activate(const QString &path, bool writable)
{
#ifdef Q_OS_HARMONY
    using namespace QtHarmonyExtras::FileShare;
    if (!isExternalDocumentPath(path)) {
        return;
    }

    // A permission persisted by a previous run is dormant until activated.
    // Prefer read/write so an editable document can still be saved, then
    // accept a read-only grant from providers that do not allow modification.
    const auto result = activatePermission({pathPolicy(path, writable)});
    if (writable && !succeeded(result)) {
        activatePermission({pathPolicy(path, false)});
    }
#else
    Q_UNUSED(path);
    Q_UNUSED(writable);
#endif
}

void persist(const QString &path, bool writable)
{
#ifdef Q_OS_HARMONY
    using namespace QtHarmonyExtras::FileShare;
    if (!isExternalDocumentPath(path)) {
        return;
    }

    const auto result = persistPermission({pathPolicy(path, writable)});
    if (writable && !succeeded(result)) {
        persistPermission({pathPolicy(path, false)});
    }
#else
    Q_UNUSED(path);
    Q_UNUSED(writable);
#endif
}
}
